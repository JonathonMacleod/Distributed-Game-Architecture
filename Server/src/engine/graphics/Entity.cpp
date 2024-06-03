#ifndef SERVER__ENTITY__H
	#define SERVER__ENTTIY__H

	#include "Entity.h"

	#include <glm/gtc/matrix_transform.hpp>

	namespace Server::Engine::Graphics {
	
		Entity::Entity(const Model& model) : m_Model(model) {
			SetPosition(0, 0, 0);
			SetRotation(0, 0, 0);
		}

		void Entity::SetPosition(float x, float y, float z) {
			m_WorldPosition.x = x;
			m_WorldPosition.y = y;
			m_WorldPosition.z = z;
		}

		void Entity::SetPosition(const glm::vec3& position) {
			m_WorldPosition = position;
		}

		void Entity::Move(float x, float y, float z) {
			m_WorldPosition.x += x;
			m_WorldPosition.y += y;
			m_WorldPosition.z += z;
		}

		void Entity::Move(const glm::vec3& delta) {
			m_WorldPosition += delta;
		}

		void Entity::SetRotation(float xRadians, float yRadians, float zRadians) {
			m_WorldRotation.x += xRadians;
			m_WorldRotation.y += yRadians;
			m_WorldRotation.z += zRadians;
		}

		void Entity::SetRotation(const glm::vec3& rotationRadians) {
			m_WorldRotation = rotationRadians;
		}

		void Entity::Rotate(float xDeltaRadians, float yDeltaRadians, float zDeltaRadians) {
			m_WorldRotation.x += xDeltaRadians;
			m_WorldRotation.y += yDeltaRadians;
			m_WorldRotation.z += zDeltaRadians;
		}

		void Entity::Rotate(const glm::vec3& deltaRadians) {
			m_WorldRotation += deltaRadians;
		}

		glm::mat4 Entity::GetModelMatrix() const {
			glm::mat4 modelMatrix(1);
			modelMatrix = glm::translate(modelMatrix, m_WorldPosition);
			modelMatrix = glm::rotate(modelMatrix, m_WorldRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, m_WorldRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, m_WorldRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			return modelMatrix;
		}

		const glm::vec3& Entity::GetWorldPosition() const {
			return m_WorldPosition;
		}

		const glm::vec3& Entity::GetWorldRotation() const {
			return m_WorldRotation;
		}

	}

#endif