#include "Messaging.h"

#include <vector>

#include "../utils/Analytics.h"
#include "../utils/Compression.h"
#include "../utils/Logging.h"
#include "../utils/Timings.h"

// This integer acts as a counter to uniquely identify the frames of gameplay messages delivered - allowing the time to compress a given frame by the server to be matched to the 
// time taken to decompress that same frame by the client
static int frameOfGameplayMessageCounter = 0;



//
// General functionality
//

static void SetEncodedMessageFloatValue(char* destination, float value) {
	//TODO: Consider endianness of the float values being encoded here
	float* valueDestination = (float*) destination;
	*valueDestination = value;
}

static float GetEncodedMessageFloatValue(const char* source) {
	//TODO: Consider endianness of the float values being encoded here
	const float* valueSource = (float*) source;
	return *valueSource;
}

static void SetEncodedMessageIntegerValue(char* destination, int value) {
	//TODO: Consider endianness of the integer values being encoded here
	int* valueDestination = (int*) destination;
	*valueDestination = value;
}

static int GetEncodedMessageIntegerValue(const char* source) {
	//TODO: Consider endianness of the integer values being encoded here
	const int* valueSource = (int*) source;
	return *valueSource;
}



//
// Messaging.h implementations
//

namespace Middleware::Networking {

	//
	// General functionality implementations
	//

	void SetMessageHeader(char* messageDataBuffer, int messageDataBufferLength, char identifierByte, int messageLengthAfterHeader) {
		// Ensure there is room to add an identifier byte and the length of the message after the header
		if(messageDataBufferLength < MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH)
			return;

		messageDataBuffer[0] = identifierByte;
		SetEncodedMessageIntegerValue(&messageDataBuffer[1], messageLengthAfterHeader);
	}

	char GetMessageTypeFromMessageHeader(const char* source, int length) {
		// Have we got enough of the header to read the identifier byte?
		if(length < 1)
			return MIDDLEWARE__MESSAGING__UNKNOWN_MESSAGE_IDENTIFIER;

		return source[0];
	}

	int GetMessageContentLengthFromMessageHeader(const char* source, int length) {
		// Have we got enough of the header to read the length of the contents?
		if(length < (1 + sizeof(int)))
			return -1;

		int bodyLength = GetEncodedMessageIntegerValue(&source[1]);
		return bodyLength;
	}



	//
	// ClientConnectionMessage class implementations
	//

	void ClientConnectionMessage::SetRequestedClientId(int clientId) {
		m_RequestedClientId = clientId;
	}

	int ClientConnectionMessage::GetRequestedClientId() const {
		return m_RequestedClientId;
	}

	void ClientConnectionMessage::SetRequestedOutputDimensions(int width, int height) {
		m_RequestedOutputWidth = width;
		m_RequestedOutputHeight = height;
	}

	int ClientConnectionMessage::GetRequestedOutputWidth() const {
		return m_RequestedOutputWidth;
	}

	int ClientConnectionMessage::GetRequestedOutputHeight() const {
		return m_RequestedOutputHeight;
	}

	bool ClientConnectionMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__CLIENT_CONNECTION_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected from the header
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// The connection message is a client ID, output width, and output height, so three integers are expected in the body
		if(bodyLength != (3 * sizeof(int)))
			return false;

		m_RequestedClientId = GetEncodedMessageIntegerValue(&messageBody[0]);
		m_RequestedOutputWidth = GetEncodedMessageIntegerValue(&messageBody[sizeof(int)]);
		m_RequestedOutputHeight = GetEncodedMessageIntegerValue(&messageBody[2 * sizeof(int)]);

		return true;
	}

	char* ClientConnectionMessage::GenerateMessage(int& length) const {
		// The message body must contain three integers (client ID, output width, and output height)
		const int expectedLengthOfBody = (3 * sizeof(int));
		const int expectedLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + expectedLengthOfBody);

		// Create a buffer for the message data
		char* messageData = new char[expectedLengthOfMessage];
		SetMessageHeader(messageData, expectedLengthOfMessage, MIDDLEWARE__MESSAGING__CLIENT_CONNECTION_MESSAGE_IDENTIFIER, expectedLengthOfBody);
		char* messageBody = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		SetEncodedMessageIntegerValue(&messageBody[0], m_RequestedClientId);
		SetEncodedMessageIntegerValue(&messageBody[sizeof(int)], m_RequestedOutputWidth);
		SetEncodedMessageIntegerValue(&messageBody[2 * sizeof(int)], m_RequestedOutputHeight);

		// Message has been successfully created, return the allocated memory pointer and length of the message
		length = expectedLengthOfMessage;
		return messageData;
	}



	//
	// ServerConnectionMessage class implementations
	//

	void ServerConnectionMessage::SetClientId(int clientId) {
		m_ClientId = clientId;
	}

	int ServerConnectionMessage::GetClientId() const {
		return m_ClientId;
	}

	void ServerConnectionMessage::SetOutputDimensions(int width, int height) {
		m_OutputWidth = width;
		m_OutputHeight = height;
	}

	int ServerConnectionMessage::GetOutputWidth() const {
		return m_OutputWidth;
	}

	int ServerConnectionMessage::GetOutputHeight() const {
		return m_OutputHeight;
	}

	bool ServerConnectionMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__SERVER_CONNECTION_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected from the header
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// The connection message is a client ID, output width, and output height, so three integers are expected in the body
		if(bodyLength != (3 * sizeof(int)))
			return false;

		m_ClientId = GetEncodedMessageIntegerValue(&messageBody[0]);
		m_OutputWidth = GetEncodedMessageIntegerValue(&messageBody[sizeof(int)]);
		m_OutputHeight = GetEncodedMessageIntegerValue(&messageBody[2 * sizeof(int)]);

		return true;
	}

	char* ServerConnectionMessage::GenerateMessage(int& length) const {
		// The message body must contain three integers (client ID, output width, and output height)
		const int expectedLengthOfBody = (3 * sizeof(int));
		const int expectedLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + expectedLengthOfBody);

		// Create a buffer for the message data
		char* messageData = new char[expectedLengthOfMessage];
		SetMessageHeader(messageData, expectedLengthOfMessage, MIDDLEWARE__MESSAGING__SERVER_CONNECTION_MESSAGE_IDENTIFIER, expectedLengthOfBody);
		char* messageBody = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		SetEncodedMessageIntegerValue(&messageBody[0], m_ClientId);
		SetEncodedMessageIntegerValue(&messageBody[sizeof(int)], m_OutputWidth);
		SetEncodedMessageIntegerValue(&messageBody[2 * sizeof(int)], m_OutputHeight);

		// Message has been successfully created, return the allocated memory pointer and length of the message
		length = expectedLengthOfMessage;
		return messageData;
	}



	//
	// ClientKeboardInputMessage class implementatons
	//

	bool ClientKeyboardInputMessage::GetKeyState(int keycode) const {
		// If we are looking up the state of a valid keycode then check whether it is currently pressed
		if((keycode >= 0) && (keycode <= MIDDLEWARE__MESSAGING__MAX_KEYCODE))
			return m_KeyStates[keycode];

		// If the keycode provided was not valid, there will be no value associated with it, so return false
		return false;
	}

	void ClientKeyboardInputMessage::SetKeyState(int keycode, bool pressed) {
		// If the keycode provided was valid then update it's state, otherwise do nothing
		if((keycode >= 0) && (keycode <= MIDDLEWARE__MESSAGING__MAX_KEYCODE))
			m_KeyStates[keycode] = pressed;
	}

	bool ClientKeyboardInputMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__KEYBOARD_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected from the header
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// Check the body contains enough bytes to represent the state of each key
		if(bodyLength != (MIDDLEWARE__MESSAGING__MAX_KEYCODE + 1))
			return false;

		// The message body is just an array of bytes representing the state of each key
		for(int i = 0; i < bodyLength; i++) {
			const char currentKeyStateByte = messageBody[i];
			const bool currentKeyState = (currentKeyStateByte != 0);
			m_KeyStates[i] = currentKeyState;
		}

		return true;
	}

	char* ClientKeyboardInputMessage::GenerateMessage(int& length) const {
		constexpr int numberOfKeyStates = (MIDDLEWARE__MESSAGING__MAX_KEYCODE + 1);

		// The message must start with a header, which is followed by a byte for each key state stored
		constexpr int expectedLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + numberOfKeyStates);

		// Create a buffer for the message data
		char* messageData = new char[expectedLengthOfMessage];
		SetMessageHeader(messageData, expectedLengthOfMessage, MIDDLEWARE__MESSAGING__KEYBOARD_MESSAGE_IDENTIFIER, numberOfKeyStates);
		char* messageBodyData = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		// Fill the message body with a sequence of bytes representing the state of each key
		for(int i = 0; i < numberOfKeyStates; i++) {
			const char currentKeyStateByte = (m_KeyStates[i] ? 0xff : 0x00);
			messageBodyData[i] = currentKeyStateByte;
		}

		// Message has been successfully created, return the allocated memory pointer and length of the message
		length = expectedLengthOfMessage;
		return messageData;
	}



	//
	// ClientCursorPositionInputMessage class implementations
	//

	float ClientCursorPositionInputMessage::GetCursorX() const {
		return m_CursorX;
	}

	float ClientCursorPositionInputMessage::GetCursorY() const {
		return m_CursorY;
	}

	void ClientCursorPositionInputMessage::SetCursorX(float xPosition) {
		m_CursorX = xPosition;
	}

	void ClientCursorPositionInputMessage::SetCursorY(float yPosition) {
		m_CursorY = yPosition;
	}

	bool ClientCursorPositionInputMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__CURSOR_POSITION_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected from the header
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// Check there are two float values contained within the body
		if(bodyLength != (2 * sizeof(float)))
			return false;

		m_CursorX = GetEncodedMessageFloatValue(&messageBody[0]);
		m_CursorY = GetEncodedMessageFloatValue(&messageBody[sizeof(float)]);

		return true;
	}

	char* ClientCursorPositionInputMessage::GenerateMessage(int& length) const {
		constexpr int bodyLength = (2 * sizeof(float));

		// The message must start with a header, which is followed by the contents of the frame
		const int expectedLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + bodyLength);

		// Create a buffer for the message data
		char* messageData = new char[expectedLengthOfMessage];
		SetMessageHeader(messageData, expectedLengthOfMessage, MIDDLEWARE__MESSAGING__CURSOR_POSITION_MESSAGE_IDENTIFIER, bodyLength);
		char* messageBodyData = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		SetEncodedMessageFloatValue(&messageBodyData[0], m_CursorX);
		SetEncodedMessageFloatValue(&messageBodyData[sizeof(float)], m_CursorY);

		// Message has been successfully created, return the allocated memory pointer and length of the message
		length = expectedLengthOfMessage;
		return messageData;
	}



	//
	// ClientMouseButtonInputMessage class implementations
	//

	bool ClientMouseButtonInputMessage::GetMouseButtonState(int buttonCode) const {
		// If we are looking up the state of a valid button then check whether it is currently pressed
		if ((buttonCode >= 0) && (buttonCode <= MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE))
			return m_ButtonStates[buttonCode];

		// If the button code provided was not valid, there will be no value associated with it, so return false
		return false;
	}

	void ClientMouseButtonInputMessage::SetMouesButtonState(int buttonCode, bool pressed) {
		// If the button provided was valid then update it's state, otherwise do nothing
		if ((buttonCode >= 0) && (buttonCode <= MIDDLEWARE__MESSAGING__MAX_KEYCODE))
			m_ButtonStates[buttonCode] = pressed;
	}

	bool ClientMouseButtonInputMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__MOUSE_BUTTON_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected from the header
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// Check that the body contains one byte for each mouse button state
		if(bodyLength != (MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE + 1))
			return false;

		// The message body is just an array of bytes representing the state of each mouse button
		for(int i = 0; i < bodyLength; i++) {
			const char currentMouseButtonStateByte = messageBody[i];
			const bool currentMouseButtonState = (currentMouseButtonStateByte != 0x00);
			m_ButtonStates[i] = currentMouseButtonState;
		}

		return true;
	}

	char* ClientMouseButtonInputMessage::GenerateMessage(int& length) const {
		constexpr int numberOfButtonStates = (MIDDLEWARE__MESSAGING__MAX_MOUSE_BUTTON_CODE + 1);

		// The message must start with a header, which is followed by a byte for each button state stored
		const int expectedLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + numberOfButtonStates);

		// Create a buffer for the message data
		char* messageData = new char[expectedLengthOfMessage];
		SetMessageHeader(messageData, expectedLengthOfMessage, MIDDLEWARE__MESSAGING__MOUSE_BUTTON_MESSAGE_IDENTIFIER, numberOfButtonStates);
		char* messageBodyData = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		// Fill the message body with a sequence of bytes representing the state of each mouse button
		for(int i = 0; i < numberOfButtonStates; i++) {
			const char currentMouseButtonStateByte = (m_ButtonStates[i] ? 0xff : 0x00);
			messageBodyData[i] = currentMouseButtonStateByte;
		}

		// Message has been successfully created, return the allocated memory pointer and length of the message
		length = expectedLengthOfMessage;
		return messageData;
	}



	//
	// ServerFrameOfGameplayMessage class implementations
	//

	ServerFrameOfGameplayMessage::~ServerFrameOfGameplayMessage() {
		SetFrameData(nullptr, 0);
	}

	void ServerFrameOfGameplayMessage::SetFrameData(const unsigned char* rawFrameData, int length) {
		if(m_RawFrameData != nullptr) {
			delete[] m_RawFrameData;
			m_RawFrameData = nullptr;
			m_RawFrameDataLength = 0;
		}

		if((rawFrameData != nullptr) && (length > 0)) {
			m_RawFrameData = new unsigned char[length];
			m_RawFrameDataLength = length;

			std::memcpy(m_RawFrameData, rawFrameData, length);
		}
	}

	const unsigned char* ServerFrameOfGameplayMessage::GetFrameData() const {
		return m_RawFrameData;
	}

	const int ServerFrameOfGameplayMessage::GetFrameLength() const {
		return m_RawFrameDataLength;
	}

	bool ServerFrameOfGameplayMessage::ReadMessage(const char* messageHeader, int headerLength, const char* messageBody, int bodyLength) {
		// Check the message is the correct type
		if(GetMessageTypeFromMessageHeader(messageHeader, headerLength) != MIDDLEWARE__MESSAGING__FRAME_OF_GAMEPLAY_MESSAGE_IDENTIFIER)
			return false;

		// Check the message body contains the amount of data expected
		int expectedBodyLength = GetMessageContentLengthFromMessageHeader(messageHeader, headerLength);
		if(bodyLength != expectedBodyLength)
			return false;

		// Read the unique identifier of the frame of gameplay message
		int frameIdentifier = GetEncodedMessageIntegerValue(messageBody);

		// Decompress the frame of gameplay from the message (and measure the time taken to do so)
		Utils::Timer decompressionTimer;
		std::vector<char>* decompressedMessageBody = Utils::DecompressMemory(&messageBody[sizeof(int)], bodyLength - 1);
		long long nsToDecompressFrame = decompressionTimer.GetNanosecondsSinceStart();

		// Record the time taken to decompress the frame of gameplay (and the the original and compressed size of the frame)
		Utils::Analytics::RecordClientDecompressionBreakdown(frameIdentifier, decompressedMessageBody->size(), bodyLength - 1, nsToDecompressFrame);

		// Store the decompressed frame of gameplay locally
		SetFrameData((const unsigned char*) decompressedMessageBody->data(), decompressedMessageBody->size());

		delete decompressedMessageBody;

		return true;
	}

	char* ServerFrameOfGameplayMessage::GenerateMessage(int& length) const {
		// The message must start with a header, which is followed by a unique identifier of the message, as well as the compressed contents of the frame
		const int maxLengthOfMessage = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + sizeof(int) + m_RawFrameDataLength);

		// Create a buffer for the message data
		char* messageData = new char[maxLengthOfMessage];
		char* messageBodyData = &messageData[MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH];

		// Uniquely identify this frame (and store the identifier in the message) so that the time taken for the server to compress this frame and the time taken for the client to 
		// decompress this frame can be matched
		int newFrameIdentifier = frameOfGameplayMessageCounter++;
		SetEncodedMessageIntegerValue(&messageBodyData[0], newFrameIdentifier);

		// Compress the frame of gameplay (and measure the time taken to do so)
		Utils::Timer compressionTimer;
		std::vector<char>* compressedMessageBody = Utils::CompressMemory((const char*) m_RawFrameData, m_RawFrameDataLength);
		long long nsToCompressFrame = compressionTimer.GetNanosecondsSinceStart();

		// Record the time taken to compress the frame of gameplay (and the original and compressed size of the frame)
		Utils::Analytics::RecordServerCompressionBreakdown(newFrameIdentifier, m_RawFrameDataLength, compressedMessageBody->size(), nsToCompressFrame);

		// Set the message header to be that of a frame of gameplay message containing a compressed frame of gameplay and the frame identifier
		SetMessageHeader(messageData, maxLengthOfMessage, MIDDLEWARE__MESSAGING__FRAME_OF_GAMEPLAY_MESSAGE_IDENTIFIER, sizeof(int) + compressedMessageBody->size());
		// Set the message body (after the identifier) to the compressed frame of gameplay data
		std::memcpy(&messageBodyData[sizeof(int)], compressedMessageBody->data(), compressedMessageBody->size());

		length = (MIDDLEWARE__MESSAGING__MESSAGE_HEADER_LENGTH + sizeof(int) + compressedMessageBody->size());

		delete compressedMessageBody;

		return messageData;
	}

}