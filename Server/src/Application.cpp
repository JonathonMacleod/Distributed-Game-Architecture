#include "Application.h"

//
// General functionality
//

static void CheckLifetimeOfApplication() {
	SERVER_INFO("Lifetime monitoring thread of Server has started");

	while(true) {
		// Get the application instance
		Server::Application* app = Server::Application::GetInstance();
		if(app == nullptr)
			break;

		// If the application has requested to close then exit the thread
		if(app->WasCloseRequested())
			break;

		// If the application has exceeded it's lifetime then request the application to close
		if(app->HaveExceededLifetime()) {
			SERVER_INFO("Maximum lifetime of the application has been exceeded, requesting the application to close");
			app->RequestClose();
		}
	}

	SERVER_INFO("Lifetime monitoring thread of Server has finished");
}

static void ReceiveNewClientConnection(Middleware::Networking::ServerSocketConnection* newClientConnection) {
	// Get the server application instance
	Server::Application* app = Server::Application::GetInstance();
	if(app == nullptr)
		return;

	// Create a new client and add it to the list of clients connected to the server
	Server::Client* newClient = new Server::Client(newClientConnection);
	app->RegisterNewClient(newClient);

	// Attempt to establish communication with the client
	newClient->EstablishCommunication();
}

static void AwaitNewClientConnections() {
	SERVER_INFO("Client connection accepting thread has started");

	// Get the server application instance
	Server::Application* app = Server::Application::GetInstance();
	if(app == nullptr)
		return;

	int errorCounter = 0;
	while(true) {
		// If the application has requested to close then exit the thread
		if(app->WasCloseRequested())
			break;

		// Attempt to accept a new client connection
		bool resultOfConnectionAttempt = app->GetConnectionSocket()->AwaitAndAccept(ReceiveNewClientConnection);

		// If the application failed to accept a new client connection then there must have been an error, or the connection socket has closed
		if(!resultOfConnectionAttempt) {
			errorCounter++;
			SERVER_ERROR("Failed to accept a new client connection. Have failed %d time(s) so far", errorCounter);

			if(errorCounter >= 3) {
				SERVER_FATAL("Failed to accept new client connections three or more times. Requesting the application to close");
				app->RequestClose();
			}
		}
	}

	SERVER_INFO("Client connection accepting thread has finished");
}



namespace Server {

	//
	// ApplicationConfig struct implementations
	//

	void ApplicationConfig::ReadCommandLineArguments(int argc, char** args) {
		// Read the port number (if one was provided)
		if(argc >= 2)
			serverPort = atoi(args[1]);

		// Read the maximum application lifetime (if one was provided)
		if(argc >= 3) {
			int providedMaximumLifetime = atoi(args[2]);

			// If an non-positive maximum lifetime was provided then set the program to run indefinitely
			if(providedMaximumLifetime <= 0)
				maximumApplicationLifetime = -1;
			else 
				maximumApplicationLifetime = providedMaximumLifetime;
		}
		
		// Read the folder to be used when outputting analytics recorded
		if(argc >= 4)
			outputDirectory = std::string(args[3]);
	}



	//
	// Application class implementation
	//

	Application* Application::m_SingletonApplicationInstance = nullptr;

	Application* Application::GetInstance() {
		//NOTE: Not thread-safe, but no threads are expected to exist before the application itself generates them
		if(m_SingletonApplicationInstance == nullptr)
			m_SingletonApplicationInstance = new Application();

		return m_SingletonApplicationInstance;
	}

	bool Application::Init(int argc, char** args) {
		// Configure the application based on the command line arguments provided
		SERVER_TRACE("About to instantiate the server instance with command-line configurations");
		m_Config.ReadCommandLineArguments(argc, args);

		// Create a hidden GLFW window to create an OpenGL context (allowing GLEW bindings to be initialised)
		SERVER_TRACE("About to create a GLFW window");
		m_Window = glfwCreateWindow(640, 480, "Hidden Server Output", NULL, NULL);
		//glfwHideWindow(m_Window);
		glfwMakeContextCurrent(m_Window);
		if(glewInit() != GLEW_OK) {
			SERVER_FATAL("Failed to initialise GLEW for server");
			glfwDestroyWindow(m_Window);
			return false;
		}

		//TODO: Create the GLFW window callbacks?

		// Create the scene
		//TODO: Cleanup this code. Should we really be defining models and shaders in code? If so, should we really be defining the environment at this point in the code? e.g. not in another, dedicated function?
		SERVER_TRACE("About to create the scene");
		Server::Engine::Graphics::ShaderProgram* individualShaderProgram = new Engine::Graphics::ShaderProgram("assets/shaders/basic/basic.vert", "assets/shaders/basic/basic.frag");
		if(individualShaderProgram->HadErrorBuilding()) {
			SERVER_FATAL("Failed to build individual shader program");
			return false;
		}
		Server::Engine::Graphics::ShaderProgram* instancedShaderProgram = new Engine::Graphics::ShaderProgram("assets/shaders/instanced/instanced.vert", "assets/shaders/instanced/instanced.frag");
		if(instancedShaderProgram->HadErrorBuilding()) {
			SERVER_FATAL("Failed to build instanced shader program");
			return false;
		}
		m_Scene = new Engine::Graphics::Scene(individualShaderProgram, instancedShaderProgram);


		// Cube model values
		// Adapted from values provided freely at https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Tutorial/Creating_3D_objects_using_WebGL (Last accessed 05/12/2022).
		constexpr float cubeModelPositions[] = {
			// Front face
			-0.5, -0.5,  0.5, 
			 0.5, -0.5,  0.5, 
			 0.5,  0.5,  0.5, 
			-0.5,  0.5,  0.5,
			// Back face
			-0.5, -0.5, -0.5, 
			-0.5,  0.5, -0.5, 
			 0.5,  0.5, -0.5, 
			 0.5, -0.5, -0.5,
			// Top face
			-0.5,  0.5, -0.5, 
			-0.5,  0.5,  0.5, 
			 0.5,  0.5,  0.5, 
			 0.5,  0.5, -0.5,
			// Bottom face
			-0.5, -0.5, -0.5, 
			 0.5, -0.5, -0.5, 
			 0.5, -0.5,  0.5, 
			-0.5, -0.5,  0.5,
			// Right face
			 0.5, -0.5, -0.5, 
			 0.5,  0.5, -0.5, 
			 0.5,  0.5,  0.5, 
			 0.5, -0.5,  0.5,
			// Left face
			-0.5, -0.5, -0.5, 
			-0.5, -0.5,  0.5, 
			-0.5,  0.5,  0.5, 
			-0.5,  0.5, -0.5
		};
		constexpr float cubeModelColours[] = {
			// Front face: white
			1.0, 1.0, 1.0,
			1.0, 1.0, 1.0,
			1.0, 1.0, 1.0,
			1.0, 1.0, 1.0,
			// Back face: red
			1.0, 0.0, 0.0,
			1.0, 0.0, 0.0,
			1.0, 0.0, 0.0,
			1.0, 0.0, 0.0,
			// Top face: green
			0.0, 1.0, 0.0, 
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 1.0, 0.0,
			// Bottom face: blue
			0.0, 0.0, 1.0, 
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			0.0, 0.0, 1.0,
			// Right face: yellow
			1.0, 1.0, 0.0, 
			1.0, 1.0, 0.0,
			1.0, 1.0, 0.0,
			1.0, 1.0, 0.0,
			// Left face: purple
			1.0, 0.0, 1.0, 
			1.0, 0.0, 1.0, 
			1.0, 0.0, 1.0, 
			1.0, 0.0, 1.0 
		};
		constexpr unsigned int cubeModelIndices[] = {
			// Front face
			 0,  1,  2,
			 0,  2,  3,
			// Back face
			 4,  5,  6,
			 4,  6,  7,
			// Top face
			 8,  9, 10,
			 8, 10, 11,   
			// Bottom face
			12, 13, 14,
			12, 14, 15,
			// Right face
			16, 17, 18,
			16, 18, 19,
			// Left face
			20, 21, 22,
			20, 22, 23
		};
	
		// Create cube entity and add to scene
		Server::Engine::Graphics::Model* cubeModel = new Server::Engine::Graphics::Model(); //TODO: This memory is allocated and never freed
		cubeModel->Set3DPositionsBuffer(cubeModelPositions, sizeof(cubeModelPositions));
		cubeModel->SetIndexBuffer(cubeModelIndices, sizeof(cubeModelIndices));
		cubeModel->SetColoursBuffer(cubeModelColours, sizeof(cubeModelColours));

		Server::Engine::Graphics::Entity* cubeEntity = new Engine::Graphics::Entity(*cubeModel);
		cubeEntity->SetPosition(0, 0, -4);
		m_Scene->AddEntity(cubeEntity);

		Server::Engine::Graphics::Entity* cubeEntityTwo = new Engine::Graphics::Entity(*cubeModel);
		cubeEntityTwo->SetPosition(0, 0, 4);
		cubeEntityTwo->SetRotation(1.57f, 0, 0);
		m_Scene->AddEntity(cubeEntityTwo);

		Server::Engine::Graphics::Entity* cubeEntityThree = new Engine::Graphics::Entity(*cubeModel);
		cubeEntityThree->SetPosition(-4, 0, 0);
		cubeEntityThree->SetRotation(0, 0, 1.57f);
		m_Scene->AddEntity(cubeEntityThree);

		Server::Engine::Graphics::Entity* cubeEntityFour = new Engine::Graphics::Entity(*cubeModel);
		cubeEntityFour->SetPosition(4, 0, 0);
		cubeEntityFour->SetRotation(1.57f, 0, 1.57f);
		m_Scene->AddEntity(cubeEntityFour);

		SERVER_TRACE("Finished initialising the server");
		return true;
	}

	void Application::Start() {
		SERVER_INFO("About to start the server");

		// Begin monitoring the lifetime of the application (assuming there is a maximum lifetime set)
		m_LifetimeTimer.ResetStartTime();
		if(m_Config.maximumApplicationLifetime > 0)
			m_LifetimeThread = std::thread { &CheckLifetimeOfApplication };

		// Create a new socket to listen for client connections
		SERVER_TRACE("About to create a new socket for the server to listen to connections on");
		m_ConnectionSocket = new Middleware::Networking::ServerSocket(m_Config.serverPort);

		// If the socket was not created successfully then we cannot continue (since no clients will be able to connect)
		if(m_ConnectionSocket->HadErrors()) {
			SERVER_FATAL("Failed to create server socket to listen for client connections");
			m_ConnectionSocket->Close();
			RequestClose();
		} else {
			// Begin listening for any new client connections
			m_ConnectionThread = std::thread { &AwaitNewClientConnections };
		}


		// Run the game loop until close is requested. This function will block until close is requested
		SERVER_TRACE("About to run the game loop (server socket may or may not have succeedeed)");
		GameLoop();

		// Close the application
		SERVER_INFO("Shutting down");
		Shutdown();
	}

	void Application::RequestClose() {
		m_WasCloseRequested = true;
	}

	bool Application::WasCloseRequested() const {
		return m_WasCloseRequested;
	}

	Middleware::Networking::ServerSocket* Application::GetConnectionSocket() {
		return m_ConnectionSocket;
	}

	void Application::RegisterNewClient(Client* client) {
		m_ClientListMutex.lock();
		m_ClientList.push_back(client);
		m_ClientListMutex.unlock();
	}

	void Application::AddNewPlayer(Engine::Players::Player* player) {
		m_Scene->AddPlayer(player);
	}

	void Application::RemovePlayer(Engine::Players::Player* player) {
		m_Scene->RemovePlayer(player);
	}

	bool Application::HaveExceededLifetime() const {
		return ((m_Config.maximumApplicationLifetime > 0) && (m_LifetimeTimer.GetMillisecondsSinceStart() >= m_Config.maximumApplicationLifetime));
	}

	void Application::GameLoop() {
		SERVER_INFO("About to start main game loop");

		// Set any OpenGL flags that are used across the render pipeline
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);




		//
		// START HIDEOUS HACK TO DISPLAY FRAMEBUFFER CONTENTS
		//

		// Create a shader to draw the test framebuffer output
		Engine::Graphics::ShaderProgram* testFramebufferOutputShader = new Engine::Graphics::ShaderProgram("assets/shaders/test_framebuffer_output/vertex.vert", "assets/shaders/test_framebuffer_output/fragment.frag");
		if(testFramebufferOutputShader->HadErrorBuilding()) {
			SERVER_FATAL("Failed to build test framebuffer output shader");
			return;
		}

		// Rectangle to fill screen
		constexpr float rectangleModelPositions[] = {
			-1, -1,
			 1, -1,
			 1,  1,
			-1,  1
		};
		constexpr float rectangleModelTextureCoords[] = {
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};
		constexpr unsigned int rectangleModelIndices[] = {
			0, 1, 3,
			1, 2, 3
		};
		Engine::Graphics::Model screenRectangleModel;
		screenRectangleModel.Set2DPositionsBuffer(rectangleModelPositions, sizeof(rectangleModelPositions));
		screenRectangleModel.SetTextureCoordinatesBuffer(rectangleModelTextureCoords, sizeof(rectangleModelTextureCoords));
		screenRectangleModel.SetIndexBuffer(rectangleModelIndices, sizeof(rectangleModelIndices));

		GLuint frameOfGameplayTexture = 0;
		glGenTextures(1, &frameOfGameplayTexture);
		glBindTexture(GL_TEXTURE_2D, frameOfGameplayTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		//
		// END HIDEOUS HACK TO DISPLAY FRAMEBUFFER CONTENTS
		//




		Middleware::Utils::Timer m_SceneUpdateDeltaTimer;
		while(!WasCloseRequested()) {
			// If the GLFW window is requesting close then request the application to close
			if(glfwWindowShouldClose(m_Window))
				RequestClose();

			// Update all of the entities in the scene
			const double secondsSinceLastUpdate = m_SceneUpdateDeltaTimer.GetSecondsSinceStart();
			m_SceneUpdateDeltaTimer.ResetStartTime();
			m_Scene->Update(secondsSinceLastUpdate);

			// Render the next frame of gameplay for each player
			m_Scene->DrawPlayerPerspectives();




			//
			// START HIDEOUS HACK TO DISPLAY FRAMEBUFFER CONTENTS
			//

			if(m_Scene->GetNumberOfPlayers() > 0) {
				// Draw the contents of the framebuffer to the window
				glViewport(0, 0, 640, 480);
				glClearColor(0.1f, 0.1f, 0.1f, 1);
				glClear(GL_COLOR_BUFFER_BIT);
				glDisable(GL_DEPTH_TEST);


				glActiveTexture(GL_TEXTURE0);
				if(tempValue) {
					glBindTexture(GL_TEXTURE_2D, frameOfGameplayTexture);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Scene->GetPlayer()->GetCamera().GetOutputWidth(), m_Scene->GetPlayer()->GetCamera().GetOutputHeight(), GL_RGB, GL_UNSIGNED_BYTE, m_Scene->GetPlayer()->GetCachedFrameData());
				} else {
					glBindTexture(GL_TEXTURE_2D, m_Scene->GetFramebuffer()->GetColourBufferAttachmentTextureId()); //TODO: Remove the Scene::GetFramebuffer() function when this hack is not needed
				}


				testFramebufferOutputShader->Bind();
				screenRectangleModel.DrawIndividual();
				testFramebufferOutputShader->Unbind();
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			//
			// END HIDEOUS HACK TO DISPLAY FRAMEBUFFER CONTENTS
			//





			// There is nothing to show to the hidden window, but swapping buffers is standard practise for GLFW windows
			glfwSwapBuffers(m_Window);
			// Poll for any GLFW user inputs and IO events
			glfwPollEvents();
		}

		SERVER_INFO("About to stop main game loop");
	}

	void Application::Shutdown() {
		// Ensure a close has been requested to notify any threads still running
		RequestClose();

		// Close the client connection socket (this must happen before waiting for threads to join as the client connection thread may be blocking)
		if(m_ConnectionSocket != nullptr)
			m_ConnectionSocket->Close();

		// Wait for any threads still running to finish
		if(m_ConnectionThread.joinable())
			m_ConnectionThread.join();
		if(m_LifetimeThread.joinable())
			m_LifetimeThread.join();

		// Delete the now closed client connection socket
		if(m_ConnectionSocket != nullptr)
			delete m_ConnectionSocket;

		// Close and destroy the GLFW window
		if(m_Window != nullptr) {
			glfwDestroyWindow(m_Window);
		}

		Middleware::Utils::Analytics::OutputAnalyticsToFolder(m_Config.outputDirectory);
	}

}