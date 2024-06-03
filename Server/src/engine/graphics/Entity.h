#ifndef SERVER__ENTITY__H
	#define SERVER__ENTITY__H

	#include <glm/glm.hpp>

	#include "Model.h"
	#include "Shaders.h"

	#include "../players/Player.h"

	namespace Server::Engine::Graphics {

		class Entity {
			public:
				Entity(const Model& model);

				void SetPosition(float x, float y, float z);
				void SetPosition(const glm::vec3& position);
				void Move(float x, float y, float z);
				void Move(const glm::vec3& delta);

				void SetRotation(float xRadians, float yRadians, float zRadians);
				void SetRotation(const glm::vec3& rotationRadians);
				void Rotate(float xDeltaRadians, float yDeltaRadians, float zDeltaRadians);
				void Rotate(const glm::vec3& deltaRadians);

				const Model& GetModel() const { return m_Model; }
				glm::mat4 GetModelMatrix() const;
				const glm::vec3& GetWorldPosition() const;
				const glm::vec3& GetWorldRotation() const;

				virtual void Update(double secondsSinceLastUpdate) { }

			private:
				const Model& m_Model;

				glm::vec3 m_WorldPosition;
				glm::vec3 m_WorldRotation;
		};

	}

#endif