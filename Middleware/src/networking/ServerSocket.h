#ifndef MIDDLEWARE__SERVERSOCKET__H
	#define MIDDLEWARE__SERVERSOCKET__H

	#include "Sockets.h"
	#include "Messaging.h"

	namespace Middleware::Networking {

		class ServerSocketConnection;
		typedef void(*ServerSocketNewConnectionCallback)(ServerSocketConnection*);

		class ServerSocketConnection {
			public:
				ServerSocketConnection(SOCKET windowsSocket);
				~ServerSocketConnection();

				ClientServerMessage* ReadClientMessage() const;
				bool SendClientMessage(const ClientServerMessage* message) const;

				void Close();

			private:
				SOCKET m_WindowsSocket = { };

			private:
				char ReadClientMessageData(char*& messageHeader, int& messageHeaderLength, char*& messageBody, int& messageBodyLength) const;

				bool ReadData(char* data, int length) const;
				bool WriteData(const char* data, int length, bool shouldRecordOutgoingTraffic) const;

				int ReadDataChunk(char* data, int targetLength) const;
				int WriteDataChunk(const char* data, int targetLength) const;
		};

		class ServerSocket {
			public:
				ServerSocket(int port);
				~ServerSocket();

				bool AwaitAndAccept(ServerSocketNewConnectionCallback callback);

				void Close();

				bool HadErrors() const;

			private:
				bool m_HadErrors = false;
				bool m_WasCloseRequested = false;

				int m_PortNumber;
				SOCKET m_WindowsSocket = { };
		};
		
	}

#endif