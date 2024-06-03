#ifndef MIDDLEWARE__MESSAGING__H
	#define MIDDLEWARE__MESSAGING__H

	// The size of the message header is equal to the identifier byte plus an integer containing the length of the message body
	#define MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH				(1 + sizeof(int))

	#define MIDDLEWARE__MESSAGING__UNKNOWN_MESSAGE_IDENTIFIER			(0x00)
	#define MIDDLEWARE__MESSAGING__CLIENT_CONNECTION_MESSAGE_IDENTIFIER	(0x11)
	#define MIDDLEWARE__MESSAGING__SERVER_CONNECTION_MESSAGE_IDENTIFIER	(0x22)
	#define MIDDLEWARE__MESSAGING__KEYBOARD_MESSAGE_IDENTIFIER			(0x33)
	#define MIDDLEWARE__MESSAGING__CURSOR_POSITION_MESSAGE_IDENTIFIER	(0x44)
	#define MIDDLEWARE__MESSAGING__MOUSE_BUTTON_MESSAGE_IDENTIFIER		(0x55)
	#define MIDDLEWARE__MESSAGING__FRAME_OF_GAMEPLAY_MESSAGE_IDENTIFIER	(0x66)

	// Within GLFW3.3 there are valid keycodes from 0-348 (inclusive). As such, this macro holds the maximum valid keycode value of 348 
	#define MIDDLEWARE__MESSAGING__MAX_KEYCODE				(348)
	// Within GLFW3.3 there are valid mouse buttons with codes ranging from 0-7. As such, this macro holds the maximum valid moue button value of 7
	#define MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE	(7)

	namespace Middleware::Networking {

		void SetMessageHeader(char* messageDataBuffer, int messageDataBufferLength, char identifierByte, int messageLengthAfterHeader);
		char GetMessageTypeFromMessageHeader(const char* source, int length);
		int GetMessageContentLengthFromMessageHeader(const char* source, int length);

		class ClientServerMessage {
			public:
				/*
				 * This method will take a given message (both it's header and associated body) and will attempt to update this object with the data provided by the message.
				 * 
				 * @param messageHeader
				 *		A pointer to the array of bytes (chars) containing the message header
				 * @param headerLength
				 *		The number of bytes in the message header. This should be MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH, but should always reflect the size of the message header instead
				 * @param messageBody
				 *		A pointer to the array of bytes (chars) containing the message body
				 * @param bodyLength
				 *		The number of bytes in the message body
				 */
				virtual bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) = 0;

				/*
				 * This method will dynamically allocate an array of bytes (chars) containing a standard message header followed by the contents of the message.
				 * A message header consists of a byte that identifies the type of the message, followed by an integer containing the length of the message body.
				 * The message body will contain data that varies based on the type of message being sent.
				 * 
				 * @param length
				 *		A reference to the integer where the length of the message (header and body combined) will be stored
				 * 
				 * @return The dynamically allocated char array containing the message data, which the caller of this method will be responsible for freeing
				 */
				virtual char* GenerateMessage(int& length) const = 0;
		};

		class ClientConnectionMessage : public ClientServerMessage {
			public:
				void SetRequestedClientId(int clientId);
				int GetRequestedClientId() const;

				void SetRequestedOutputDimensions(int width, int height);
				int GetRequestedOutputWidth() const;
				int GetRequestedOutputHeight() const;

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) override;
				char* GenerateMessage(int& length) const override;

			private:
				int m_RequestedClientId = -1;
				int m_RequestedOutputWidth = -1;
				int m_RequestedOutputHeight = -1;
		};

		class ServerConnectionMessage : public ClientServerMessage {
			public:
				void SetClientId(int clientId);
				int GetClientId() const;

				void SetOutputDimensions(int width, int height);
				int GetOutputWidth() const;
				int GetOutputHeight() const;

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) override;
				char* GenerateMessage(int& length) const override;

			private:
				int m_ClientId = -1;
				int m_OutputWidth = -1;
				int m_OutputHeight = -1;
		};

		class ClientKeyboardInputMessage : public ClientServerMessage {
			public:
				bool GetKeyState(int keycode) const;
				void SetKeyState(int keycode, bool pressed);

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) override;
				char* GenerateMessage(int& length) const override;

			private:
				// This array of booleans allows a key to be looked up using it's keycode as the index
				bool m_KeyStates[MIDDLEWARE__MESSAGING__MAX_KEYCODE + 1] = { };
		};

		class ClientCursorPositionInputMessage : public ClientServerMessage {
			public:
				float GetCursorX() const;
				float GetCursorY() const;

				void SetCursorX(float xPosition);
				void SetCursorY(float yPosition);

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) override;
				char* GenerateMessage(int& length) const override;

			private:
				float m_CursorX = 0;
				float m_CursorY = 0;
		};

		class ClientMouseButtonInputMessage : public ClientServerMessage {
			public:
				bool GetMouseButtonState(int buttonCode) const;
				void SetMouesButtonState(int buttonCode, bool pressed);

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) override;
				char* GenerateMessage(int& length) const override;

			private:
				// This array of booleans allows a button to be looked up using it's code as the index
				bool m_ButtonStates[MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE + 1] = { };
		};

		class ServerFrameOfGameplayMessage : public ClientServerMessage {
			public:
				~ServerFrameOfGameplayMessage();

				void SetFrameData(const unsigned char* rawFrameData, int length);

				const unsigned char* GetFrameData() const;
				const int GetFrameLength() const;

				bool ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength);
				char* GenerateMessage(int& length) const;

			private:
				unsigned char* m_RawFrameData = nullptr;
				int m_RawFrameDataLength = 0;
		};

	}

#endif