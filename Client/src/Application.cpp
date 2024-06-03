#include "Application.h"

#include "graphics/Shaders.h"
#include <cstdlib>

//
// Internal functionality
//

static void CheckLifetimeOfApplication() {
	CLIENT_INFO("Lifetime monitoring thread of Client has started");

	while(true) {
		// Get the application instance
		Client::Application* app = Client::Application::GetInstance();
		if(app == nullptr)
			break;

		// If the application has requested to close then exit the thread
		if(app->WasCloseRequested())
			break;

		// If the application has exceeded it's lifetime then request the application to close
		if(app->HaveExceededLifetime()) {
			CLIENT_INFO("Maximum lifetime of the application has been exceeded, requesting the application to close");
			app->RequestClose();
		}
	}

	CLIENT_INFO("Lifetime monitoring thread of Client has finished");
}

static void FetchFramesOfGameplay() {
	int errorCounter = 0;

	CLIENT_INFO("Frame of gameplay fetching thread of Client has started");

	while(true) {
		// Get the application instance
		Client::Application* app = Client::Application::GetInstance();
		if(app == nullptr)
			break;

		// If the application has requested to close then exit the thread
		if(app->WasCloseRequested())
			break;

		// Attempt to fetch a frame of gameplay
		Middleware::Networking::ClientServerMessage* message = app->ReadMessageFromServer();

		// If the application failed to fetch a message from the socket, it must have closed, or had an error
		if(message == nullptr) {
			errorCounter++;
			CLIENT_ERROR("Failed to fetch message (expected frame of gameplay). Have failed %d time(s) so far", errorCounter);
			
			// This covers a scenario where the client is unaware that the socket is closed (or experiencing errors), but the app hasn't been instructed to close yet (e.g. the server has disconnected)
			if(errorCounter >= 3) {
				CLIENT_FATAL("Failed to fetch frame of gameplay message three or more times. Requesting the application to close");
				app->RequestClose();
			}
		} else {
			// If a message was successfully retrieved, reset the error counter
			errorCounter = 0;

			Middleware::Networking::ServerFrameOfGameplayMessage* frameOfGameplayMessage = dynamic_cast<Middleware::Networking::ServerFrameOfGameplayMessage*>(message);
			if(frameOfGameplayMessage == nullptr) {
				CLIENT_ERROR("Encountered unknown message type while trying to read frame of gameplay. Discarding message...");
				delete message;
			} else {
				// Extract the frame of gameplay and update the locally cached version
				app->GetFrameOfGameplayMutex().lock();
				app->SetFrameOfGameplay(frameOfGameplayMessage->GetFrameData(), frameOfGameplayMessage->GetFrameLength());

				Middleware::Utils::Analytics::GetClientPlayoutDelayRecorder().RecordFrameCopied();
				Middleware::Utils::Analytics::RecordNewFrameOfGameplayReceived();

				app->GetFrameOfGameplayMutex().unlock();
				delete frameOfGameplayMessage;
			}
		}
	}

	CLIENT_INFO("Frame of gameplay fetching thread of Client has stopped");
}

static void GLFWKeyboardInputMonitor(GLFWwindow* window) {
	Client::Application* app = Client::Application::GetInstance();
	if(app == nullptr)
		return;
	
	// Find any keys being pressed to send to the server
	Middleware::Networking::ClientKeyboardInputMessage* keyboardInputMessage = new Middleware::Networking::ClientKeyboardInputMessage();
	for(int i = 0; i <= MIDDLEWARE__MESSAGING__MAX_KEYCODE; i++) {
		bool currentKeyState = (glfwGetKey(window, i) == GLFW_PRESS);
		keyboardInputMessage->SetKeyState(i, currentKeyState);
	}

	// Attempt to send the keyboard inputs to the server
	app->WriteMessageToServer(keyboardInputMessage);
	delete keyboardInputMessage;
}

static void GLFWMouseButtonInputMonitor(GLFWwindow* window) {
	Client::Application* app = Client::Application::GetInstance();
	if(app == nullptr)
		return;

	// Find any mouse buttons being pressed to send to the server
	Middleware::Networking::ClientMouseButtonInputMessage* mouseButtonInputMessage = new Middleware::Networking::ClientMouseButtonInputMessage();
	for(int i = 0; i <= MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE; i++) {
		bool currentButtonState = (glfwGetMouseButton(window, i) == GLFW_PRESS);
		mouseButtonInputMessage->SetMouesButtonState(i, currentButtonState);
	}

	// Attempt to send the mouse button inputs to the server
	app->WriteMessageToServer(mouseButtonInputMessage);
	delete mouseButtonInputMessage;
}

static void GLFWMouseMovementCallback(GLFWwindow* window, double xPos, double yPos) { 
	Client::Application* app = Client::Application::GetInstance();
	if(app == nullptr)
		return;

	static float lastRelativeX = 0;
	static float lastRelativeY = 0;

	// Get a position relative to the output size (e.g. 0 is at 0, 1 is the size of the output dimension)
	float relativeX = ((float) xPos / (float) app->GetWindowWidth());
	float relativeY = ((float) yPos / (float) app->GetWindowHeight());

	float diffX = (relativeX - lastRelativeX);
	float diffY = (relativeY - lastRelativeY);
	lastRelativeX = relativeX;
	lastRelativeY = relativeY;

	// Attempt to send the cursor position to the server
	Middleware::Networking::ClientCursorPositionInputMessage* cursorPositionMessage = new Middleware::Networking::ClientCursorPositionInputMessage();
	cursorPositionMessage->SetCursorX(diffX);
	cursorPositionMessage->SetCursorY(diffY);
	app->WriteMessageToServer(cursorPositionMessage);
	delete cursorPositionMessage;
}

static void GLFWWindowResizeCallback(GLFWwindow* window, int width, int height) {
	Client::Application* app = Client::Application::GetInstance();
	if(app != nullptr)
		app->UpdateWindowSize(width, height);
}

static void GameLogic() {
	Client::Application* app = Client::Application::GetInstance();
	if(app == nullptr)
		return;


	// Create a VAO containing a fullscreen square with texture coordinates at each vertex for the frame of gameplay to be mapped to
	unsigned int fullScreenSquareVertexArrayObject = 0;
	glGenVertexArrays(1, &fullScreenSquareVertexArrayObject);
	glBindVertexArray(fullScreenSquareVertexArrayObject);

	// Apply the vertices of the square to the VAO
	float vertices[] = {
		 1.0f,  1.0f,  0.0f,
		 1.0f, -1.0f,  0.0f,
		-1.0f, -1.0f,  0.0f,
		-1.0f,  1.0f,  0.0f
	};

	unsigned int fullScreenSquarePositionsBuffer = 0;
	glGenBuffers(1, &fullScreenSquarePositionsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenSquarePositionsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Apply the texture coordinates to the VAO
	float textureCoordinates[] = {
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};
	unsigned int fullScreenSquareTextureCoordsBuffer = 0;
	glGenBuffers(1, &fullScreenSquareTextureCoordsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fullScreenSquareTextureCoordsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, textureCoordinates, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create an IBO for the fullscreen square VAO
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	unsigned int fullScreenSquareIndicesBuffer = 0;
	glGenBuffers(1, &fullScreenSquareIndicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullScreenSquareIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Unbind the VAO
	glBindVertexArray(0);


	// Create a texture to contain the frame of gameplay
	GLuint frameOfGameplayTexture = 0;
	glGenTextures(1, &frameOfGameplayTexture);
	glBindTexture(GL_TEXTURE_2D, frameOfGameplayTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, app->GetConfig().outputWidth, app->GetConfig().outputHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Create a shader to draw the frame of gameplay
	Client::Graphics::ShaderProgram shaderProgram("assets/shaders/basic/basic.vert", "assets/shaders/basic/basic.frag");
	if(shaderProgram.HadErrorBuilding()) {
		CLIENT_FATAL("Failed to create client shader program");
		app->RequestClose();
	}

	while(!app->WasCloseRequested()) {
		// If close was requested then close the application
		if(glfwWindowShouldClose(app->GetGLFWWindow()))
			app->RequestClose();

		// If there is a new frame of gameplay to draw then update the texture contents
		if(app->IsFrameOfGameplayDirty()) {
			Middleware::Utils::Analytics::GetClientPlayoutDelayRecorder().RecordRenderStarted();

			// Get the currently cached frame of gameplay and copy it to a texture to draw to the frame
			app->GetFrameOfGameplayMutex().lock();

			glBindTexture(GL_TEXTURE_2D, frameOfGameplayTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, app->GetConfig().outputWidth, app->GetConfig().outputHeight, GL_RGB, GL_UNSIGNED_BYTE, app->GetFrameOfGameplay());
			glBindTexture(GL_TEXTURE_2D, 0);

			app->MarkFrameOfGameplayClean();
			app->GetFrameOfGameplayMutex().unlock();
		}


		// Clear the screen ready to draw
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		// Draw the fullscreen square VAO using the index buffer object, the texture containing the fullscreen texture, and the shader
		shaderProgram.Bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frameOfGameplayTexture);
		glBindVertexArray(fullScreenSquareVertexArrayObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullScreenSquareIndicesBuffer);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		shaderProgram.Unbind();


		// Draw the screen
		glfwSwapBuffers(app->GetGLFWWindow());

		Middleware::Utils::Analytics::GetClientPlayoutDelayRecorder().RecordRenderCompleted();

		// Poll for user inputs and IO events
		glfwPollEvents();
		// Check for any keyboard inputs
		GLFWKeyboardInputMonitor(app->GetGLFWWindow());
		// Check for any mouse button inputs
		GLFWMouseButtonInputMonitor(app->GetGLFWWindow());
	}


	// Delete the fullscreen square VAO, buffers, and indices buffer
	glDeleteBuffers(1, &fullScreenSquarePositionsBuffer);
	glDeleteBuffers(1, &fullScreenSquareTextureCoordsBuffer);
	glDeleteVertexArrays(1, &fullScreenSquareVertexArrayObject);

	// Delete the frame of gameplay texture
	glDeleteTextures(1, &frameOfGameplayTexture);
}



namespace Client {

	//
	// ApplicationConfig struct implementations
	//

	void ApplicationConfig::ReadCommandLineArguments(int argc, char** args) {
		// Read the server IP to connect to (if one was provided)
		if(argc >= 2)
			serverIP = std::string(args[1]);

		// Read the server port to connect to (if one was provided)
		if(argc >= 3)
			serverPort = atoi(args[2]);

		// Read the maximum lifetime of the application (if one was provided)
		if(argc >= 4) {
			int providedMaximumLifetime = atoi(args[3]);

			// If an non-positive maximum lifetime was provided then set the program to run indefinitely
			if(providedMaximumLifetime <= 0)
				maximumApplicationLifetime = -1;
			else 
				maximumApplicationLifetime = providedMaximumLifetime;
		}

		// Read the client ID (if one was specified)
		if(argc >= 5)
			clientId = atoi(args[4]);

		// Read the directory path to output any results measured (assuming a path was provided)
		if(argc >= 6)
			outputDirectory = std::string(args[5]);
	}



	//
	// Application class implementations
	//

	Application* Application::m_SingletonApplicationInstance = nullptr;

	Application* Application::GetInstance() {
		//NOTE: Not thread-safe, but no threads are expected to exist before the application itself generates them
		if(m_SingletonApplicationInstance == nullptr)
			m_SingletonApplicationInstance = new Application();

		return m_SingletonApplicationInstance;
	}

	bool Application::Init(int argc, char** args) {
		m_Config.ReadCommandLineArguments(argc, args);

		// Create the GLFW window
		m_Window = glfwCreateWindow(640, 480, "Client View", NULL, NULL);
		if(!m_Window) {
			CLIENT_FATAL("Failed to create a client GLFW window");
			return false;
		}
			
		// Make the GLFW window the current OpenGL context
		glfwMakeContextCurrent(m_Window);

		// Update the OpenGL viewport to the size of the window
		UpdateWindowSize(640, 480);

		// Initialise GLEW bindings now that the OpenGL context has been created (the GLFW window)
		if(glewInit() != GLEW_OK) {
			CLIENT_FATAL("Failed to initialise GLEW for client");
			glfwDestroyWindow(m_Window);
			delete m_Window;
			return false;
		}

		// Create the GLFW window callbacks
		glfwSetCursorPosCallback(m_Window, GLFWMouseMovementCallback);
		glfwSetWindowSizeCallback(m_Window, GLFWWindowResizeCallback);

		// Hide the cursor when the window is focussed by the user
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		return true;
	}

	void Application::Start() {
		// Begin monitoring the lifetime of the thread (assuming there is a maximum lifetime set)
		m_LifetimeTimer.ResetStartTime();
		if(m_Config.maximumApplicationLifetime > 0)
			m_LifetimeThread = std::thread{ &CheckLifetimeOfApplication };

		// Create a client socket and attempt to connect to the server (for as long as the client lifetime hasn't been exceeded)
		while((m_ClientSocket == nullptr) && !WasCloseRequested()) {
			m_ClientSocket = new Middleware::Networking::ClientSocket(m_Config.serverIP, m_Config.serverPort);

			// If there was an error creating the client socket (e.g. it failed to connect to the server) then deallocate it and prepare to attempt a new connection
			if(m_ClientSocket->HadErrors()) {
				delete m_ClientSocket;
				m_ClientSocket = nullptr;
			}
		}

		// If the client socket was successfully connected
		if(m_ClientSocket != nullptr) {
			// Attempt to establish a connection with the server
			if(!EstablishServerCommunication())
				RequestClose();

			// Begin attempting to fetch new frames of gameplay continually (until the application closes)
			m_GameplayFrameThread = std::thread{ &FetchFramesOfGameplay };

			// Run the game logic until close is requested. This function will block until close is requested
			GameLogic();
		}

		// Close the application
		Shutdown();
	}

	void Application::RequestClose() {
		m_WasCloseRequested = true;
	}

	bool Application::WasCloseRequested() const {
		return m_WasCloseRequested;
	}

	bool Application::HaveExceededLifetime() const {
		return ((m_Config.maximumApplicationLifetime > 0) && (m_LifetimeTimer.GetMillisecondsSinceStart() >= m_Config.maximumApplicationLifetime));
	}

	const ApplicationConfig& Application::GetConfig() const {
		return m_Config;
	}

	int Application::GetWindowWidth() {
		return m_WindowWidth;
	}

	int Application::GetWindowHeight() {
		return m_WindowHeight;
	}

	void Application::UpdateWindowSize(int width, int height) {
		m_WindowWidth = width;
		m_WindowHeight = height;
		glViewport(0, 0, width, height);
	}

	GLFWwindow* Application::GetGLFWWindow() {
		return m_Window;
	}

	bool Application::IsFrameOfGameplayDirty() {
		return m_IsFrameOfGameplayDirty;
	}

	void Application::MarkFrameOfGameplayClean() {
		m_IsFrameOfGameplayDirty = false;
	}

	std::mutex& Application::GetFrameOfGameplayMutex() {
		return m_FrameOfGameplayMutex;
	}

	const unsigned char* Application::GetFrameOfGameplay() const {
		return m_FrameOfGameplay;
	}

	int Application::GetFrameOfGameplayLength() const {
		return m_FrameOfGameplayLength;
	}

	void Application::SetFrameOfGameplay(const unsigned char* frameOfGameplayData, int frameOfGameplayLength) {
		if(m_FrameOfGameplayLength != frameOfGameplayLength) {
			if(m_FrameOfGameplay != nullptr)
				delete[] m_FrameOfGameplay;
			m_FrameOfGameplay = new unsigned char[frameOfGameplayLength];
		}

		std::memcpy(m_FrameOfGameplay, frameOfGameplayData, frameOfGameplayLength);
		m_FrameOfGameplayLength = frameOfGameplayLength;

		m_IsFrameOfGameplayDirty = true;
	}

	Middleware::Networking::ClientServerMessage* Application::ReadMessageFromServer() {
		Middleware::Networking::ClientServerMessage* message = nullptr;

		m_ClientSocketReadMutex.lock();
		message = m_ClientSocket->ReadServerMessage();
		m_ClientSocketReadMutex.unlock();

		return message;
	}

	bool Application::WriteMessageToServer(const Middleware::Networking::ClientServerMessage* message) {
		m_ClientSocketWriteMutex.lock();
		bool result = m_ClientSocket->SendServerMessage(message);
		m_ClientSocketWriteMutex.unlock();

		return result;
	}

	void Application::Shutdown() {
		// Ensure a close has been requested to notify any threads still running
		RequestClose();

		// Close the sockets (this must happen before waiting for threads to join since they may be blocking on a socket operation)
		if(m_ClientSocket != nullptr)
			m_ClientSocket->Close();

		// Wait for any threads still running to finish
		if(m_LifetimeThread.joinable()) 
			m_LifetimeThread.join();
		if(m_GameplayFrameThread.joinable()) 
			m_GameplayFrameThread.join();

		// Destroy the GLFW window
		glfwDestroyWindow(m_Window);

		// Free the cached frame of gameplay
		if(m_FrameOfGameplay != nullptr)
			delete[] m_FrameOfGameplay;

		// Free the client socket
		if(m_ClientSocket != nullptr)
			delete m_ClientSocket;

		Middleware::Utils::Analytics::OutputAnalyticsToFolder(m_Config.outputDirectory, m_Config.clientId);
	}

	bool Application::EstablishServerCommunication() {
		// Send a message to the server requesting a particular client ID, output width and output height
		Middleware::Networking::ClientConnectionMessage clientConnectionMessage;
		clientConnectionMessage.SetRequestedClientId(m_Config.clientId);
		clientConnectionMessage.SetRequestedOutputDimensions(m_Config.outputWidth, m_Config.outputHeight);

		if(!WriteMessageToServer(&clientConnectionMessage)) {
			CLIENT_ERROR("Failed to send client connection message to server");
			return false;
		}

		int errorCounter = 0;

		// Attempt to recieve a response from the server to verify the connection attempt
		bool hasReceivedServerConnectionMessage = false;
		do {
			// If the application has requested to close then stop trying to connect to the server
			if(WasCloseRequested())
				break;

			// Attempt to fetch the server connection message
			Middleware::Networking::ClientServerMessage* serverConnectionMessageResponse = nullptr;
			serverConnectionMessageResponse = ReadMessageFromServer();

			// If there is no response the connection must have been closed, or there must have been an error
			if(serverConnectionMessageResponse == nullptr) {
				errorCounter++;
				CLIENT_ERROR("Failed to fetch message (expected server connection mesage). Have failed %d time(s) so far", errorCounter);

				if(errorCounter >= 3) {
					CLIENT_FATAL("Failed to fetch server connection message three or more times. Requesting the application to close");
					RequestClose();
				}

				continue;
			}

			// Ensure the message received was actually a ServerConnectionMessage
			Middleware::Networking::ServerConnectionMessage* serverConnectionMessage = dynamic_cast<Middleware::Networking::ServerConnectionMessage*>(serverConnectionMessageResponse);
			if(serverConnectionMessage == nullptr) {
				CLIENT_ERROR("Encountered unknown message type while trying to read frame of gameplay. Discarding message...");
				delete serverConnectionMessageResponse;
			} else {
				hasReceivedServerConnectionMessage = true;

				//TODO: Should verify and accept client identifiers, for now they are assigned by command line argument
//				m_Config.clientId = serverConnectionMessage->GetClientId();
				m_Config.outputWidth = serverConnectionMessage->GetOutputWidth();
				m_Config.outputHeight = serverConnectionMessage->GetOutputHeight();
				delete serverConnectionMessage;
			}
		} while(!hasReceivedServerConnectionMessage && !HaveExceededLifetime());

		return hasReceivedServerConnectionMessage;
	}

}