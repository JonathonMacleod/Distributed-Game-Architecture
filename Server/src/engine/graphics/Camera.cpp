#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Server::Engine::Graphics {

	Camera::Camera(int outputWidth, int outputHeight, float fovDegrees, float zNear, float zFar) {
		SetConfiguration(outputWidth, outputHeight, fovDegrees, zNear, zFar);
	}

	void Camera::SetPosition(float x, float y, float z) { 
		m_CameraPosition.x = x;
		m_CameraPosition.y = y;
		m_CameraPosition.z = z;
	}

	void Camera::SetPosition(const glm::vec3& position) {
		m_CameraPosition = position;
	}

	void Camera::Move(float x, float y, float z) {
		m_CameraPosition.x += x;
		m_CameraPosition.y += y;
		m_CameraPosition.z += z;
	}

	void Camera::Move(const glm::vec3& delta) {
		m_CameraPosition += delta;
	}

	void Camera::MoveForwards(float delta) {
		m_CameraPosition += (delta * m_CameraFront);
	}

	void Camera::MoveBackwards(float delta) {
		m_CameraPosition -= (delta * m_CameraFront);
	}

	void Camera::MoveLeft(float delta) {
		m_CameraPosition -= (delta * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)));
	}

	void Camera::MoveRight(float delta) {
		m_CameraPosition += (delta * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)));
	}

	void Camera::SetRotation(float pitchRadians, float yawRadians) {
		m_CameraPitch = pitchRadians;
		m_CameraYaw = yawRadians;

		if(m_CameraPitch > glm::radians<float>(89)) {
			m_CameraPitch = glm::radians<float>(89);
		}
		if(m_CameraPitch < glm::radians<float>(-89)) {
			m_CameraPitch = glm::radians<float>(-89);
		}

		glm::vec3 front = {
			cos(m_CameraYaw) * cos(m_CameraPitch),
			sin(m_CameraPitch),
			sin(m_CameraYaw) * cos(m_CameraPitch)
		};
		m_CameraFront = glm::normalize(front);
	}

	void Camera::Rotate(float pitchDeltaRadians, float yawDeltaRadians) {
		SetRotation(m_CameraPitch + pitchDeltaRadians, m_CameraYaw + yawDeltaRadians);
	}

	void Camera::SetConfiguration(int outputWidth, int outputHeight, float fovDegrees, float zNear, float zFar) {
		m_OutputWidth = outputWidth;
		m_OutputHeight = outputHeight;
		m_CameraFOV = fovDegrees;
		m_CameraNearPlane = zNear;
		m_CameraFarPlane = zFar;

		const float fovRadians = glm::radians<float>(m_CameraFOV);
		const float aspectRatio = ((float) m_OutputWidth) / ((float) m_OutputHeight);

		m_PerspectiveMatrix = glm::perspective(fovRadians, aspectRatio, m_CameraNearPlane, m_CameraFarPlane);
	}

	void Camera::SetOutputSize(int outputWidth, int outputHeight) {
		SetConfiguration(outputWidth, outputHeight, m_CameraFOV, m_CameraNearPlane, m_CameraFarPlane);
	}

	void Camera::SetFOV(float fov) {
		SetConfiguration(m_OutputWidth, m_OutputHeight, fov, m_CameraNearPlane, m_CameraFarPlane);
	}

	void Camera::SetNearAndFarDistances(float nearDistance, float farDistance) {
		SetConfiguration(m_OutputWidth, m_OutputHeight, m_CameraFOV, nearDistance, farDistance);
	}

	glm::mat4 Camera::GetViewMatrix() const {
		return glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraFront, m_CameraUp);
	}

}