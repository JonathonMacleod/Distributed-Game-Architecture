#ifndef MIDDLEWARE__CLIENTSOCKET__H
	#define MIDDLEWARE__CLIENTSOCKET__H

	#include <string>

	#include "Sockets.h"
	#include "Messaging.h"

	namespace Middleware::Networking {

		class ClientSocket {
			public:
				ClientSocket(const std::string& serverIP, int serverPort);
				~ClientSocket();

				ClientServerMessage* ReadServerMessage() const;
				bool SendServerMessage(const ClientServerMessage* message) const;

				void Close();

				bool HadErrors() const;

			private:
				char ReadServerMessageData(char*& messageHeader, int& messageHeaderLength, char*& messageBody, int& messageBodyLength) const;

				bool ReadData(char* data, int length) const;
				bool WriteData(const char* data, int length) const;

				int ReadDataChunk(char* data, int targetLength) const;
				int WriteDataChunk(const char* data, int targetLength) const;

			private:
				bool m_HadErrors = false;
				bool m_WasCloseRequested = false;

				SOCKET m_WindowsSocket = { };
		};

	}

#endif