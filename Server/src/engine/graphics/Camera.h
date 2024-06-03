#ifndef SERVER__CAMERA__H
	#define SERVER__CAMERA__H
	
	#include <glm/glm.hpp>

	namespace Server::Engine::Graphics {

		class Camera {
			public:
				Camera(int outputWidth = 100, int outputHeight = 100, float fovDegrees = 60.0f, float zNear = 0.01f, float zFar = 100.0f);

				void SetPosition(float x, float y, float z);
				void SetPosition(const glm::vec3& position);
				void Move(float x, float y, float z);
				void Move(const glm::vec3& delta);

				void MoveForwards(float delta);
				void MoveBackwards(float delta);
				void MoveLeft(float delta);
				void MoveRight(float delta);

				void SetRotation(float pitchRadians, float yawRadians);
				void Rotate(float pitchDeltaRadians, float yawDeltaRadians);

				void SetConfiguration(int outputWidth = 100, int outputHeight = 100, float fovDegrees = 60.0f, float zNear = 0.01f, float zFar = 100.0f);
				void SetOutputSize(int outputWidth = 100, int outputHeight = 100);
				void SetFOV(float fov = 60.0f);
				void SetNearAndFarDistances(float nearDistance = 0.1f, float farDistance = 100.0f);

				const glm::vec3& GetPosition() const { return m_CameraPosition; }
				float GetYaw() const { return m_CameraYaw; }
				float GetPitch() const { return m_CameraPitch; }
				float GetFOV() const { return m_CameraFOV; }
				float GetNearPlane() const { return m_CameraNearPlane; }
				float GetFarPlane() const { return m_CameraFarPlane; }
				int GetOutputWidth() const { return m_OutputWidth; }
				int GetOutputHeight() const { return m_OutputHeight; }

				glm::mat4 GetViewMatrix() const;
				const glm::mat4& GetPerspectiveMatrix() const { return m_PerspectiveMatrix; }

			private:
				glm::mat4 m_PerspectiveMatrix;

				int m_OutputWidth = 100;
				int m_OutputHeight = 100;
				float m_CameraFOV = 60.0f;
				float m_CameraNearPlane = 0.1f;
				float m_CameraFarPlane = 100.0f;

				float m_CameraPitch = 0.0f;
				float m_CameraYaw = -90.0f;
				glm::vec3 m_CameraUp = glm::vec3(0, 1, 0);
				glm::vec3 m_CameraPosition = glm::vec3(0, 0, 0);
				glm::vec3 m_CameraFront = glm::vec3(0, 0, -1);
		};

	}

#endif