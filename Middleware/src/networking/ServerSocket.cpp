#include "ServerSocket.h"

#include "../utils/Analytics.h"
#include "../utils/Logging.h"

//
// ServerSocket.h implementations
//

namespace Middleware::Networking {

	//
	// ServerSocketConnection class implementations
	//

	ServerSocketConnection::ServerSocketConnection(SOCKET windowsSocket) {
		m_WindowsSocket = windowsSocket;
	}

	ServerSocketConnection::~ServerSocketConnection() {
		Close();
	}

	ClientServerMessage* ServerSocketConnection::ReadClientMessage() const {
		char* messageHeader = nullptr;
		int messageHeaderLength = 0;
		char* messageBody = nullptr;
		int messageBodyLength = 0;

		char identifyingByte = ReadClientMessageData(messageHeader, messageHeaderLength, messageBody, messageBodyLength);

		ClientServerMessage* result = nullptr;

		if(identifyingByte == MIDDLEWARE__MESSAGING__CLIENT_CONNECTION_MESSAGE_IDENTIFIER) {
			ClientConnectionMessage* newMessage = new ClientConnectionMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		} else if(identifyingByte == MIDDLEWARE__MESSAGING__SERVER_CONNECTION_MESSAGE_IDENTIFIER) {
			ServerConnectionMessage* newMessage = new ServerConnectionMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		} else if(identifyingByte == MIDDLEWARE__MESSAGING__KEYBOARD_MESSAGE_IDENTIFIER) {
			ClientKeyboardInputMessage* newMessage = new ClientKeyboardInputMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		} else if(identifyingByte == MIDDLEWARE__MESSAGING__CURSOR_POSITION_MESSAGE_IDENTIFIER) {
			ClientCursorPositionInputMessage* newMessage = new ClientCursorPositionInputMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		} else if(identifyingByte == MIDDLEWARE__MESSAGING__MOUSE_BUTTON_MESSAGE_IDENTIFIER) {
			ClientMouseButtonInputMessage* newMessage = new ClientMouseButtonInputMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		} else if(identifyingByte == MIDDLEWARE__MESSAGING__FRAME_OF_GAMEPLAY_MESSAGE_IDENTIFIER) {
			ServerFrameOfGameplayMessage* newMessage = new ServerFrameOfGameplayMessage();
			if(newMessage->ReadMessage(messageHeader, messageHeaderLength, messageBody, messageBodyLength))
				result = newMessage;
			else
				delete newMessage;

		}

		delete[] messageHeader;
		delete[] messageBody;
		return result;
	}

	bool ServerSocketConnection::SendClientMessage(const ClientServerMessage* message) const {
		int messageLength = 0;
		char* messageData = message->GenerateMessage(messageLength);

		bool isOutgoingDataAFrameOfGameplay = (GetMessageTypeFromMessageHeader(messageData, messageLength) == MIDDLEWARE__MESSAGING__FRAME_OF_GAMEPLAY_MESSAGE_IDENTIFIER);
		bool result = WriteData(messageData, messageLength, isOutgoingDataAFrameOfGameplay);

		delete[] messageData;
		return result;
	}

	void ServerSocketConnection::Close() {
		closesocket(m_WindowsSocket);
	}

	char ServerSocketConnection::ReadClientMessageData(char*& messageHeader, int& messageHeaderLength, char*& messageBody, int& messageBodyLength) const {
		// Initially set the parameter values to error values
		messageHeader = nullptr;
		messageHeaderLength = 0;
		messageBody = nullptr;
		messageBodyLength = 0;

		// Every message starts with a message header containing the length and type of the message
		char* newMessageHeader = new char[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];
		if(!ReadData(newMessageHeader, MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH)) {
			MIDDLEWARE_ERROR("Failed to read message, failed to read message header");
			delete[] newMessageHeader;
			return MIDDLEWARE__MESSAGING__UNKNOWN_MESSAGE_IDENTIFIER;
		}

		// Check the message body length
		int newMessageBodyLength = GetMessageContentLengthFromMessageHeader(newMessageHeader, MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH);

		// Now read the message body
		char* newMessageBody = nullptr;
		if(newMessageBodyLength != 0) {
			newMessageBody = new char[newMessageBodyLength];
			if(!ReadData(newMessageBody, newMessageBodyLength)) {
				MIDDLEWARE_ERROR("Failed to read message, failed to read message body");
				delete[] newMessageHeader;
				delete[] newMessageBody;
				return MIDDLEWARE__MESSAGING__UNKNOWN_MESSAGE_IDENTIFIER; 
			}
		}

		// Now we have a message header and message body
		messageHeader = newMessageHeader;
		messageHeaderLength = MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH;
		messageBody = newMessageBody;
		messageBodyLength = newMessageBodyLength;
		return GetMessageTypeFromMessageHeader(messageHeader, messageHeaderLength);
	}

	bool ServerSocketConnection::ReadData(char* data, int length) const {
		int cumulativeSizeOfDataRead = 0;

		// While there is still more data to read into the data buffer provided continue trying to fetch more bytes
		while(cumulativeSizeOfDataRead < length) {
			const int remainingLengthToRead = (length - cumulativeSizeOfDataRead);

			char* nextBufferPosition = &data[cumulativeSizeOfDataRead];
			int newLengthRead = ReadDataChunk(nextBufferPosition, remainingLengthToRead);

			// Errors and closed connections return a length of 0 or less
			if(newLengthRead <= 0) {
				MIDDLEWARE_ERROR("Failed to read desired amount of data from ServerSocketConnection (connection closed or another error has occurred)");
				break;
			} else {
				cumulativeSizeOfDataRead += newLengthRead;
			}
		}

		// Return whether the entire length of the message was read successfully
		return (cumulativeSizeOfDataRead == length);
	}

	bool ServerSocketConnection::WriteData(const char* data, int length, bool shouldRecordOutgoingTraffic) const {
		int cumulativeSizeOfDataWritten = 0;

		// While there is still more data to write to the socket continue sending more data
		while(cumulativeSizeOfDataWritten < length) {
			const int remainingLengthToWrite = (length - cumulativeSizeOfDataWritten);

			const char* nextBufferPosition = &data[cumulativeSizeOfDataWritten];
			int newLengthWrite = WriteDataChunk(nextBufferPosition, remainingLengthToWrite);

			// Errors and closed connections return a length of 0 or less
			if(newLengthWrite <= 0) {
				MIDDLEWARE_ERROR("Failed to write desired amount of data to ServerSocketConnection (connection closed or another error has occurred)");
				break;
			} else {
				cumulativeSizeOfDataWritten += newLengthWrite;

				// Record outgoing traffic being delivered to clients
				if(shouldRecordOutgoingTraffic) {
					Utils::Analytics::RecordFrameOfGameplayDataSent(newLengthWrite);
				}
			}
		}

		// Return whether the entire length of the message was sent successfully
		return (cumulativeSizeOfDataWritten == length);
	}

	int ServerSocketConnection::ReadDataChunk(char* data, int targetLength) const {
		int sizeOfDataReceived = recv(m_WindowsSocket, data, targetLength, 0);

		// If the amount of data received was 0 then the connection must be closed
		if(sizeOfDataReceived == 0) {
			MIDDLEWARE_ERROR("Failed to read data from ServerSocketConnection (connection closed)");
			return 0;
		}

		// If the amount of data received was -1 then there was some kind of error reading from the socket
		if(sizeOfDataReceived == -1) {
			//TODO: Check errno to find out the specific error
			MIDDLEWARE_ERROR("Failed to read data from ServerSocketConnection (unspecified error)");
			return -1;
		}

		return sizeOfDataReceived;
	}

	int ServerSocketConnection::WriteDataChunk(const char* data, int targetLength) const {
		int dataSent = send(m_WindowsSocket, data, targetLength, 0);
		return dataSent;
	}



	//
	// ServerSocket class implementations
	//

	ServerSocket::ServerSocket(int port) {
		m_PortNumber = port;

		// Create the socket
		m_WindowsSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(m_WindowsSocket == INVALID_SOCKET) {
			MIDDLEWARE_ERROR("Failed to create server socket");
			m_HadErrors = true;
			return;
		}

		// Bind the socket to a port and address
		sockaddr_in windowsSocketAddrInIPv4 = { };
		ZeroMemory(&windowsSocketAddrInIPv4, sizeof(windowsSocketAddrInIPv4));
		windowsSocketAddrInIPv4.sin_family = AF_INET;
		windowsSocketAddrInIPv4.sin_addr.s_addr = INADDR_ANY;
		windowsSocketAddrInIPv4.sin_port = htons((u_short) port);
		if(bind(m_WindowsSocket, (sockaddr*) &windowsSocketAddrInIPv4, sizeof(windowsSocketAddrInIPv4)) != 0) {
			MIDDLEWARE_ERROR("Failed to bind server socket to IPv4 config");
			m_HadErrors = true;
			return;
		}
	

		// Put the socket into listening mode
		if(listen(m_WindowsSocket, SOMAXCONN) != 0) {
			//TODO: Use WSAGetLastError to find out the cause of the error
			MIDDLEWARE_ERROR("Failed to place server socket into listening mode");
			m_HadErrors = true;
			return;
		}
	}

	ServerSocket::~ServerSocket() {
		Close();
	}

	void ServerSocket::Close() {
		m_WasCloseRequested = true;
		closesocket(m_WindowsSocket);
	}

	bool ServerSocket::AwaitAndAccept(ServerSocketNewConnectionCallback callback) {
		struct sockaddr_in clientAddressInfo = { };
		ZeroMemory(&clientAddressInfo, sizeof(clientAddressInfo));

		int sizeOfClientAddressInfo = sizeof(clientAddressInfo);

		MIDDLEWARE_TRACE("About to accept a new connection (if one is waiting)");
		SOCKET clientSocket = accept(m_WindowsSocket, (sockaddr*) &clientAddressInfo, &sizeOfClientAddressInfo);
		if(clientSocket == INVALID_SOCKET) {
			//TODO: Check the error message using WSAGetLastErrorMessage
			MIDDLEWARE_ERROR("Server socket failed to accept IPv4 client connection");
			return false;
		}

		MIDDLEWARE_TRACE("Accepted the new connection, about to create a new server socket connection instance");
		callback(new ServerSocketConnection(clientSocket));
		return true;
	}

	bool ServerSocket::HadErrors() const {
		return m_HadErrors;
	}

}