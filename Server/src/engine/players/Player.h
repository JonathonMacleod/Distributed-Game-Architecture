#ifndef SERVER__PLAYER__H
	#define SERVER__PLAYER__H

	#include <thread>
	#include <mutex>

	#include "Middleware.h"
	#include "../graphics/Camera.h"

	namespace Server::Engine::Players {
		
		class Player {
			public:
				Player(int outputWidth, int outputHeight, float fov = 60.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

				void SetPosition(float x, float y, float z);
				void SetPosition(const glm::vec3& position);
				void SetPitchAndYAW(float pitchRadians, float yawRadians);

				void MoveForwards(float delta);
				void MoveBackwards(float delta);
				void MoveLeft(float delta);
				void MoveRight(float delta);
				void MoveYAW(float deltaRadians);
				void MovePitch(float deltaRadians);

				void SetOutputSize(int outputWidth = 100, int outputHeight = 100);
				void SetFOV(float fov = 60.0f);
				void SetNearAndFarDistances(float nearDistance = 0.1f, float farDistance = 100.0f);

				const Graphics::Camera& GetCamera() const;
				const glm::vec3& GetPosition() const;
				glm::mat4 GetViewMatrix() const;
				glm::mat4 GetPerspectiveMatrix() const;

				void SetCachedFrameDataDirty(bool isDirty);
				bool IsCachedFrameDataDirty();
				unsigned char* GetCachedFrameData();
				int GetCachedFrameDataLength();
				std::mutex& GetCachedFrameDataMutex();

				void RecreateCachedFrameBuffer(int outputWidth, int outputHeight); //TODO: ?

				Middleware::Utils::Analytics::PlayerProcessingDelayRecorder& GetProcessingDelayRecorder();

			private:
				Graphics::Camera m_Camera;

				std::mutex m_CachedFrameDataMutex;
				unsigned char* m_CachedFrameData = nullptr;
				int m_CachedFrameDataLength = 0;
				bool m_IsCachedFrameOfGameplayDirty = false;

				Middleware::Utils::Analytics::PlayerProcessingDelayRecorder m_ProcessingDelayRecorder;
		};

	}

#endif