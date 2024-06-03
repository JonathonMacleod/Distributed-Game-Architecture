#ifndef CLIENT__APPLICATION__H
	#define CLIENT__APPLICATION__H

	#include <mutex>
	#include <string>
	#include <thread>

	#include "Middleware.h"
	#include "graphics/Graphics.h"

	namespace Client {
		
		struct ApplicationConfig {
			bool shouldWindowBeVisible = true;
			bool shouldWindowBeFullscreen = false;
			bool shouldWindowTransmitInputs = true;

			int clientId = 0;
			int outputWidth = 1920;
			int outputHeight = 1080;

			int maximumApplicationLifetime = -1; // Run indefinitely by default unless the maximum number of milliseconds was provided as a command-line argument
			
			std::string serverIP = "127.0.0.1";
			int serverPort = 8080;

			std::string outputDirectory = "";

			void ReadCommandLineArguments(int argc, char** args);
		};

		class Application {
			protected:
				Application() { }

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

				const ApplicationConfig& GetConfig() const;

				int GetWindowWidth();
				int GetWindowHeight();
				void UpdateWindowSize(int width, int height);
				GLFWwindow* GetGLFWWindow();

				bool IsFrameOfGameplayDirty();
				void MarkFrameOfGameplayClean();
				std::mutex& GetFrameOfGameplayMutex();
				int GetFrameOfGameplayLength() const;
				const unsigned char* GetFrameOfGameplay() const;
				void SetFrameOfGameplay(const unsigned char* frameOfGameplayData, int frameOfGameplayLength);

				Middleware::Networking::ClientServerMessage* ReadMessageFromServer();
				bool WriteMessageToServer(const Middleware::Networking::ClientServerMessage* message);

			private:
				static Application* m_SingletonApplicationInstance;

				ApplicationConfig m_Config;
				Middleware::Utils::Timer m_LifetimeTimer;

				bool m_WasCloseRequested = false;

				int m_WindowWidth = 0;
				int m_WindowHeight = 0;
				GLFWwindow* m_Window = nullptr;

				std::thread m_LifetimeThread;
				std::thread m_GameplayFrameThread;

				std::mutex m_FrameOfGameplayMutex;
				bool m_IsFrameOfGameplayDirty = false;
				unsigned char* m_FrameOfGameplay = nullptr;
				int m_FrameOfGameplayLength = 0;

				std::mutex m_ClientSocketReadMutex;
				std::mutex m_ClientSocketWriteMutex;
				Middleware::Networking::ClientSocket* m_ClientSocket = nullptr;

			private:
				void Shutdown();
				bool EstablishServerCommunication();
		};

	}

#endif