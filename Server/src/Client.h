#ifndef SERVER__CLIENT__H
	#define SERVER__CLIENT__H

	#include <thread>

	#include "Middleware.h"
	#include "engine/players/Player.h"

	namespace Server {
		
		class Client {
			public:
				Client(Middleware::Networking::ServerSocketConnection* connection);
				~Client();

				void EstablishCommunication();
				void CreatePlayer(int outputWidth, int outputHeight);

				Engine::Players::Player* GetPlayer();
				
				bool IsHalted();
				void Halt();

				Middleware::Networking::ClientServerMessage* ReadMessage();
				bool WriteMessage(const Middleware::Networking::ClientServerMessage* message);

			private:
				Engine::Players::Player* m_Player = nullptr;

				std::thread m_ConnectionThread;
				std::thread m_InputHandlerThread;
				std::thread m_FrameOfGameplayHandlerThread;

				std::mutex m_ConnectionSocketReadMutex;
				std::mutex m_ConnectionSocketWriteMutex;
				Middleware::Networking::ServerSocketConnection* m_ConnectionSocket = nullptr;
		};

	}

#endif