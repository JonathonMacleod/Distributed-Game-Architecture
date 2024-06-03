#ifndef SERVER__APPLICATION__H
	#define SERVER__APPLICATION__H

	#include <mutex>
	#include <thread>
	#include <vector>

	#include "Client.h"
	#include "engine/Engine.h"

	namespace Server {
		
		struct ApplicationConfig {
			int serverPort = 8080;
			int maximumApplicationLifetime = -1; // Run indefinitely by default (unless the maximum life was specified in milliseconds as a command line argument)

			std::string outputDirectory = "";

			void ReadCommandLineArguments(int argc, char** args);
		};

		class Application {
			protected:
				Application() { }

				static Application* m_ApplicationInstance;

			public:
				// Singletons should not be cloneable or assignable
				Application(Application& other) = delete;
				void operator=(const Application&) = delete;

				static Application* GetInstance();

				bool Init(int argc, char** args);
				void Start();

				void RequestClose();
				bool WasCloseRequested() const;
				bool HaveExceededLifetime() const;

				Middleware::Networking::ServerSocket* GetConnectionSocket();
				void RegisterNewClient(Client* client);
				void AddNewPlayer(Engine::Players::Player* player);
				void RemovePlayer(Engine::Players::Player* player);

				bool tempValue = false;

			private:
				static Application* m_SingletonApplicationInstance;

				ApplicationConfig m_Config;
				Middleware::Utils::Timer m_LifetimeTimer;

				bool m_WasCloseRequested = false;

				std::thread m_LifetimeThread;
				std::thread m_ConnectionThread;
 
				Middleware::Networking::ServerSocket* m_ConnectionSocket = nullptr;

				GLFWwindow* m_Window = nullptr;
				Engine::Graphics::Scene* m_Scene = nullptr;

				std::mutex m_ClientListMutex;
				std::vector<Client*> m_ClientList;

			private:
				void GameLoop();
				void Shutdown();
		};

	}

#endif