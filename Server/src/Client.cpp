#include "Client.h"

#include "Application.h"

//
// General Functionality
//

static void EstablishClientConnection(Server::Client* client) {
	bool wasConnectionMessageReceived = false;
	int requestedClientId = 0;
	int requestedOutputWidth = 0;
	int requestedOutputHeight = 0;

	// Attempt to receive a client connection message
	int errorCounter = 0;
	while(true) {
		Middleware::Networking::ClientServerMessage* clientMessage = client->ReadMessage();

		// If there is no message from the client, the connection must have been closed, or there must have been an error reading the message
		if(clientMessage == nullptr) {
			errorCounter++;
			CLIENT_ERROR("Failed to fetch message (expected client connection message). Have failed %d time(s)", errorCounter);

			if(errorCounter >= 3) {
				CLIENT_FATAL("Failed to receive client connection message three or more times. Aborting client connection");
				break;
			}

			continue;
		}

		// Ensure the message received was actually a ClientConnectionMessage
		Middleware::Networking::ClientConnectionMessage* connectionMessage = dynamic_cast<Middleware::Networking::ClientConnectionMessage*>(clientMessage);
		if(connectionMessage == nullptr) {
			CLIENT_ERROR("Encountered unknown message while trying to read client connection message. Discarding message...");
			delete clientMessage;
		} else {
			wasConnectionMessageReceived = true;

			requestedClientId = connectionMessage->GetRequestedClientId();
			requestedOutputWidth = connectionMessage->GetRequestedOutputWidth();
			requestedOutputHeight = connectionMessage->GetRequestedOutputHeight();

			delete connectionMessage;
			break;
		}
	}

	// Send a reply to the client letting them know the connection has been accepted
	Middleware::Networking::ServerConnectionMessage* serverConnectionMessage = new Middleware::Networking::ServerConnectionMessage();
	//TODO: Verify the client ID is not in use
	serverConnectionMessage->SetClientId(requestedClientId);
	//TODO: Verify the output dimensions are valid (e.g. larger than 0, not exceptionally large)
	serverConnectionMessage->SetOutputDimensions(requestedOutputWidth, requestedOutputHeight);
	bool wasServerConnectionSent = client->WriteMessage(serverConnectionMessage);

	if(!wasServerConnectionSent) {
		CLIENT_FATAL("Failed to send server connection message back to client");
	} else {
		// If the server connection message was sent correctly then create a player in the scene and 
		client->CreatePlayer(requestedOutputWidth, requestedOutputHeight);
	}
}

static void AwaitClientInputs(Server::Client* client) { 
	Server::Application* app = Server::Application::GetInstance();
	if(app == nullptr)
		return;

	while(true) {
		if(app->WasCloseRequested())
			break;

		Middleware::Networking::ClientServerMessage* messageReceived = client->ReadMessage();
		if(messageReceived == nullptr) {
			// TODO:
		} else {
			Middleware::Networking::ClientCursorPositionInputMessage* cursorPositionInputMessage = dynamic_cast<Middleware::Networking::ClientCursorPositionInputMessage*>(messageReceived);
			Middleware::Networking::ClientKeyboardInputMessage* keyboardInputMessage = dynamic_cast<Middleware::Networking::ClientKeyboardInputMessage*>(messageReceived);
			Middleware::Networking::ClientMouseButtonInputMessage* mouseButtonInputMessage = dynamic_cast<Middleware::Networking::ClientMouseButtonInputMessage*>(messageReceived);

			if(cursorPositionInputMessage != nullptr) { 
				float relativeDeltaX = cursorPositionInputMessage->GetCursorX();
				float relativeDeltaY = cursorPositionInputMessage->GetCursorY();

				float yawDelta = (relativeDeltaX * 0.3f);
				float pitchDelta = (relativeDeltaY * -0.3f);

				client->GetPlayer()->MoveYAW(yawDelta);
				client->GetPlayer()->MovePitch(pitchDelta);

				delete cursorPositionInputMessage;

				// Start measuring how long it takes for a frame to be drawn now an input has been received.
				client->GetPlayer()->GetProcessingDelayRecorder().RecordInputReceived();
			} else if(keyboardInputMessage != nullptr) {
				for(int i = 0; i < (MIDDLEWARE__MESSAGING__MAX_KEYCODE + 1); i++) {
					if(keyboardInputMessage->GetKeyState(i)) 
						MIDDLEWARE_TEST("Keyboard input detected. The following key was pressed: %s", glfwGetKeyName(i, 0));
				}

				if(keyboardInputMessage->GetKeyState(GLFW_KEY_LEFT_CONTROL)) {
					client->GetPlayer()->SetPosition(0, 0, 0);
					client->GetPlayer()->SetPitchAndYAW(0, 0);
				}

				if(keyboardInputMessage->GetKeyState(GLFW_KEY_K))
					app->tempValue = false;
				if(keyboardInputMessage->GetKeyState(GLFW_KEY_L))
					app->tempValue = true;

				if(keyboardInputMessage->GetKeyState(GLFW_KEY_W))
					client->GetPlayer()->MoveForwards(0.02f);
				if(keyboardInputMessage->GetKeyState(GLFW_KEY_S))
					client->GetPlayer()->MoveForwards(-0.02f);
				if(keyboardInputMessage->GetKeyState(GLFW_KEY_A))
					client->GetPlayer()->MoveRight(-0.02f);
				if(keyboardInputMessage->GetKeyState(GLFW_KEY_D))
					client->GetPlayer()->MoveRight(0.02f);

				delete keyboardInputMessage;

				// Start measuring how long it takes for a frame to be drawn now an input has been received.
				client->GetPlayer()->GetProcessingDelayRecorder().RecordInputReceived();
			} else if(mouseButtonInputMessage != nullptr) {
				delete mouseButtonInputMessage;

				// Start measuring how long it takes for a frame to be drawn now an input has been received.
				client->GetPlayer()->GetProcessingDelayRecorder().RecordInputReceived();
			} else {
				// Unrecognised message type
				delete messageReceived;
			}
		}
	}
}

static void SendFramesOfGameplayToClient(Server::Client* client) { 
	Server::Application* app = Server::Application::GetInstance();
	if(app == nullptr)
		return;

	SERVER_INFO("Thread to send frames of gameplay to client has started");

	int errorCounter = 0;
	while(true) {
		if(app->WasCloseRequested())
			break;

		if(client->IsHalted())
			break;

		if(client->GetPlayer()->IsCachedFrameDataDirty()) {
			bool wasSendSuccessful = false;

			client->GetPlayer()->GetCachedFrameDataMutex().lock();

			// Record how long it took from a frame being generated to the server starting to transfer it to the client.
			client->GetPlayer()->GetProcessingDelayRecorder().RecordTransferStarted();

			Middleware::Networking::ServerFrameOfGameplayMessage frameOfGameplayMessage;

			// Start measuring how long it takes to compress the frame.
			client->GetPlayer()->GetProcessingDelayRecorder().RecordCompressionStart();

			frameOfGameplayMessage.SetFrameData(client->GetPlayer()->GetCachedFrameData(), client->GetPlayer()->GetCachedFrameDataLength());

			// Finish measuring how long it takes to compress the frame being sent (and start measuring how long it takes to send the frame).
			client->GetPlayer()->GetProcessingDelayRecorder().RecordCompressionComplete();

			wasSendSuccessful = client->WriteMessage(&frameOfGameplayMessage);

			// Finish measuring how long it takes to send the frame.
			client->GetPlayer()->GetProcessingDelayRecorder().RecordTransferComplete();

			// If the message was sent successfully then mark the cached frame as clean so we don't resend the same frame contents
			if(wasSendSuccessful) client->GetPlayer()->SetCachedFrameDataDirty(false);
			client->GetPlayer()->GetCachedFrameDataMutex().unlock();

			// If we failed to send the message containing the frame of gameplay then 
			if(!wasSendSuccessful) {
				errorCounter++;
				SERVER_ERROR("Failed to send client a frame of gameplay. Have failed %d time(s) so far.", errorCounter);

				if(errorCounter >= 3) {
					SERVER_ERROR("Failed to send client a frame of gameplay three or more times. Halting client connection...");
					break;
				}
			}
		}
	}

	SERVER_INFO("Thread to send frames of gameplay to client is stopping");
}



namespace Server {

	//
	// Client class functionality
	//

	Client::Client(Middleware::Networking::ServerSocketConnection* connection) {
		m_ConnectionSocket = connection;
	}

	Client::~Client() {
		Halt();

		if(m_Player != nullptr) {
			//TODO: Remove player from scene
			delete m_Player;
		}

		if(m_ConnectionSocket != nullptr)
			delete m_ConnectionSocket;
	}

	void Client::EstablishCommunication() {
		// Start the thread to establish communication with the client
		m_ConnectionThread = std::thread { &EstablishClientConnection, this };
	}

	void Client::CreatePlayer(int outputWidth, int outputHeight) {
		// Create a player for the client
		m_Player = new Engine::Players::Player(outputWidth, outputHeight);

		// Add player to the scene
		Application* app = Application::GetInstance();
		app->AddNewPlayer(m_Player);

		// Start threads for listening to client inputs and sending frames of gameplay back to the client
		m_InputHandlerThread = std::thread{ &AwaitClientInputs, this };
		m_FrameOfGameplayHandlerThread = std::thread{ &SendFramesOfGameplayToClient, this };
	}

	Engine::Players::Player* Client::GetPlayer() {
		return m_Player;
	}

	bool Client::IsHalted() {
		return (!m_ConnectionThread.joinable() && !m_InputHandlerThread.joinable() && !m_FrameOfGameplayHandlerThread.joinable());
	}

	void Client::Halt() {
		m_ConnectionSocket->Close();

		// Wait for the connection establishing thread to join
		if(m_ConnectionThread.joinable())
			m_ConnectionThread.join();

		// Wait for the input receiver thread to join
		if(m_InputHandlerThread.joinable())
			m_InputHandlerThread.join();

		// Wait for the frame of gameplay propogation thread to join
		if(m_FrameOfGameplayHandlerThread.joinable())
			m_FrameOfGameplayHandlerThread.join();
	}

	Middleware::Networking::ClientServerMessage* Client::ReadMessage() {
		m_ConnectionSocketReadMutex.lock();
		Middleware::Networking::ClientServerMessage* message = m_ConnectionSocket->ReadClientMessage();
		m_ConnectionSocketReadMutex.unlock();
		return message;
	}

	bool Client::WriteMessage(const Middleware::Networking::ClientServerMessage* message) {
		bool result = false;

		m_ConnectionSocketWriteMutex.lock();
		result = m_ConnectionSocket->SendClientMessage(message);
		m_ConnectionSocketWriteMutex.unlock();

		return result;
	}
	
}