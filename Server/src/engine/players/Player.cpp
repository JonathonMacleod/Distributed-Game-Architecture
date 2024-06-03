#include "Player.h"

namespace Server::Engine::Players {

	//
	// Player class functionality
	//

	//TOOD: Pass the unique player ID to the processing delay recorder to distinguish each player
	Player::Player(int outputWidth, int outputHeight, float fov, float nearPlane, float farPlane) : m_ProcessingDelayRecorder(0) {
		m_Camera.SetConfiguration(outputWidth, outputHeight, fov, nearPlane, farPlane);
		RecreateCachedFrameBuffer(outputWidth, outputHeight);
	}

	void Player::SetPosition(float x, float y, float z) { 
		m_Camera.SetPosition(x, y, z); 
	}

	void Player::SetPosition(const glm::vec3& position) { 
		m_Camera.SetPosition(position);
	}

	void Player::SetPitchAndYAW(float pitchRadians, float yawRadians) { 
		m_Camera.SetRotation(pitchRadians, yawRadians); 
	}

	void Player::MoveForwards(float delta) { 
		m_Camera.MoveForwards(delta); 
	}

	void Player::MoveBackwards(float delta) {
		m_Camera.MoveBackwards(delta); 
	}

	void Player::MoveLeft(float delta) {
		m_Camera.MoveLeft(delta); 
	}

	void Player::MoveRight(float delta) {
		m_Camera.MoveRight(delta); 
	}

	void Player::MoveYAW(float deltaRadians) {
		m_Camera.Rotate(0, deltaRadians); 
	}

	void Player::MovePitch(float deltaRadians) {
		m_Camera.Rotate(deltaRadians, 0); 
	}

	void Player::SetOutputSize(int outputWidth, int outputHeight) {
		m_Camera.SetOutputSize(outputWidth, outputHeight);
		RecreateCachedFrameBuffer(outputWidth, outputHeight);
	}

	void Player::SetFOV(float fov) { 
		m_Camera.SetFOV(fov); 
	}

	void Player::SetNearAndFarDistances(float nearDistance, float farDistance) { 
		m_Camera.SetNearAndFarDistances(nearDistance, farDistance); 
	}

	const Graphics::Camera& Player::GetCamera() const {
		return m_Camera; 
	}

	const glm::vec3& Player::GetPosition() const { 
		return m_Camera.GetPosition(); 
	}

	glm::mat4 Player::GetViewMatrix() const { 
		return m_Camera.GetViewMatrix(); 
	}

	glm::mat4 Player::GetPerspectiveMatrix() const { 
		return m_Camera.GetPerspectiveMatrix(); 
	}

	void Player::SetCachedFrameDataDirty(bool isDirty) {
		m_IsCachedFrameOfGameplayDirty = isDirty;
	}

	bool Player::IsCachedFrameDataDirty() {
		return m_IsCachedFrameOfGameplayDirty;
	}

	unsigned char* Player::GetCachedFrameData() { 
		return m_CachedFrameData; 
	}

	int Player::GetCachedFrameDataLength() {
		return m_CachedFrameDataLength;
	}

	std::mutex& Player::GetCachedFrameDataMutex() {
		return m_CachedFrameDataMutex;
	}

	void Player::RecreateCachedFrameBuffer(int outputWidth, int outputHeight) {
		m_CachedFrameDataMutex.lock();

		// Delete the frame currently cached
		if(m_CachedFrameData != nullptr)
			delete[] m_CachedFrameData;

		// Create a new frame of gameplay cache at the specified size
		m_CachedFrameDataLength = (outputWidth * outputHeight * 3);
		m_CachedFrameData = new unsigned char[m_CachedFrameDataLength];

		// Set the frame of gameplay to clean. Whatever changes the player would be looking for have just been cleared
		m_IsCachedFrameOfGameplayDirty = false;

		m_CachedFrameDataMutex.unlock();
	}

	Middleware::Utils::Analytics::PlayerProcessingDelayRecorder& Player::GetProcessingDelayRecorder() {
		return m_ProcessingDelayRecorder;
	}

}