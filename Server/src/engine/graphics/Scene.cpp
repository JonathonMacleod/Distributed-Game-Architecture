#include "Scene.h"

#include <glm/glm.hpp>

#include "Middleware.h"

namespace Server::Engine::Graphics {

	Scene::~Scene() {
		for(Entity* entity : m_AllEntities) {
			delete entity;
		}
		for(Players::Player* player : m_AllPlayers) {
			delete player;
		}

		delete m_InstanceShaderProgram;
		delete m_IndividualShaderProgram;
	}

	void Scene::AddPlayer(Players::Player* player) {
		//TODO: Await the end of any draw attempts in-progress and notify that player modification is in-progress

		m_AllPlayers.push_back(player);

		//TODO: Notify that player modification has finished
	}

	void Scene::RemovePlayer(Players::Player* player) {
		//TODO: Await the end of any draw attempts in-progress and notify that player modification is in-progres

		bool haveRemovedPlayer = false;
		size_t currentPlayerIndex = 0;
		while(currentPlayerIndex < m_AllPlayers.size()) {
			haveRemovedPlayer = false;

			Players::Player* currentPlayer = m_AllPlayers.at(currentPlayerIndex);

			if(currentPlayer == player) {
				// Remove the group
				std::vector<Players::Player*>::iterator iteratorPosition = m_AllPlayers.begin();
				std::advance(iteratorPosition, currentPlayerIndex);
				m_AllPlayers.erase(iteratorPosition);
				
				haveRemovedPlayer = true;
			}

			if(!haveRemovedPlayer) {
				currentPlayerIndex++;
			}
		}

		//TODO: Notify that player modification has finished
	}
	
	void Scene::Update(double secondsSinceLastUpdate) {
		for(Entity* currentEntity : m_AllEntities) {
			currentEntity->Update(secondsSinceLastUpdate);
		}
	}

	void Scene::DrawPlayerPerspectives() {
		if(GetNumberOfPlayers() <= 0)
			return;

		//TODO: Await any entity modifications and notify other functions that a draw is now in-progress

		// Organise each player into groups, which contain all the players that will be drawn to a single framebuffer
		FillRenderGroups();

		glClearColor(0.4f, 0.4f, 0.4f, 1);
		glEnable(GL_DEPTH_TEST);

		// Draw the scene for each group individually
		for(Players::PlayerRenderGroup& currentRenderGroup : m_AllPlayerRenderGroups) {
			// Check that the current render group isn't empty
			if(currentRenderGroup.GetNumberOfMembers() == 0)
				continue;

			currentRenderGroup.RecordStartOfRendering();

			currentRenderGroup.GetFrameBuffer()->Bind();
			currentRenderGroup.GetFrameBuffer()->Clear();

			// While everything is instanceable at the moment, we may later need to split the group draw based on entities being alpha blended etc
			DrawInstancedEntities(m_InstanceShaderProgram, currentRenderGroup, m_AllEntities);
			//DrawIndividualEntities(m_IndividualShaderProgram, currentRenderGroup, m_AllEntities);

			currentRenderGroup.RecordEndOfRendering();

			// While the framebuffer is bound, copy the drawn frame to each player
			currentRenderGroup.UpdatePlayerFrameCache();

			currentRenderGroup.GetFrameBuffer()->Unbind();
		}

		// Clear the existing groups (incase a player leaves or requests a different output size etc between now and next time). This leaves the framebuffers for future use
		ClearRenderGroups();

		//TODO: Notify that the draw has finished
	}

	int Scene::GetNumberOfPlayers() const {
		return ((int) m_AllPlayers.size());
	}

	void Scene::FillRenderGroups() {
		// Allocate all players in the scene to a render group
		for(Players::Player* currentPlayer : m_AllPlayers) {
			// Attempt to find an existing group that can accomodate the player's perspective
			bool foundGroup = false;
			for(Players::PlayerRenderGroup& currentGroup : m_AllPlayerRenderGroups) {
				if(currentGroup.AddPlayer(currentPlayer)) {
					foundGroup = true;
					break;
				}
			}

			// We have failed to find an existing group, create a new one and try adding the player
			if(!foundGroup) {
				Players::PlayerRenderGroup newGroup;
				if(newGroup.GetFrameBuffer()->HadErrorBuilding()) {
					SERVER_ERROR("Failed to create a new render group (failed to build a framebuffer)");
				} else {
					if(!newGroup.AddPlayer(currentPlayer)) {
						SERVER_ERROR("Failed to allocate player to a render group");
					} else {
						m_AllPlayerRenderGroups.push_back(newGroup);
					}
				}
			}
		}
	}

	void Scene::ClearRenderGroups() {
		for(Players::PlayerRenderGroup& currentGroup : m_AllPlayerRenderGroups) {
			currentGroup.ClearPlayers();
		}
	}

	void Scene::DestroyRenderGroups() {
		for(Players::PlayerRenderGroup& currentGroup : m_AllPlayerRenderGroups) {
			currentGroup.DestroyGroup();
		}
		m_AllPlayerRenderGroups.clear();
	}

	void Scene::DrawIndividualEntities(const ShaderProgram* individualShaderProgram, const Players::PlayerRenderGroup& group, const std::vector<Entity*>& entities) {
		constexpr GLint uniformLocationViewMatrix = 0;
		constexpr GLint uniformLocationModelMatrix = 1;
		constexpr GLint uniformLocationProjectionMatrix = 2;

		individualShaderProgram->Bind();

		// Set shader uniform data about each player in the group (e.g. where they should be drawn to the target, and what perspective matrix they are using)
		const std::vector<glm::vec4>& groupMemberDrawingBoundaries = group.GetGroupMemberDrawingBoundaries();
		const std::vector<glm::mat4>& groupMemberViewMatrices = group.GetGroupMemberViewMatrices();
		const std::vector<glm::mat4>& groupMemberPerspectiveMatrices = group.GetGroupMemberPerspectiveMatrices();

		// Draw all specified entities for each player in the group
		for(size_t i = 0; i < group.GetNumberOfMembers(); i++) {
			const glm::mat4& currentMemberViewMatrix = groupMemberViewMatrices.at(i);
			const glm::mat4& currentMemberPerspectiveMatrix = groupMemberPerspectiveMatrices.at(i);
			individualShaderProgram->SetUniformMatrix4(uniformLocationProjectionMatrix, currentMemberPerspectiveMatrix);
			individualShaderProgram->SetUniformMatrix4(uniformLocationViewMatrix, currentMemberViewMatrix);

			for(Entity* currentEntity : entities) {
				individualShaderProgram->SetUniformMatrix4(uniformLocationModelMatrix, currentEntity->GetModelMatrix());
				currentEntity->GetModel().DrawIndividual();
			}
		}

		individualShaderProgram->Unbind();
	}

	void Scene::DrawInstancedEntities(const ShaderProgram* instanceShaderProgram, const Players::PlayerRenderGroup& group, const std::vector<Entity*>& entities) {
		constexpr GLint uniformLocationViewMatrices = 0;
		constexpr GLint uniformLocationModelMatrix = 200;
		constexpr GLint uniformLocationProjectionMatrices = 400;
		constexpr GLint uniformLocationDrawingBoundaries = 600;

		instanceShaderProgram->Bind();

		// Set shader uniform data about each player in the group (e.g. where they should be drawn to the target, and what perspective matrix they are using)
		const std::vector<glm::vec4>& groupMemberDrawingBoundaries = group.GetGroupMemberDrawingBoundaries();
		const std::vector<glm::mat4>& groupMemberViewMatrices = group.GetGroupMemberViewMatrices();
		const std::vector<glm::mat4>& groupMemberPerspectiveMatrices = group.GetGroupMemberPerspectiveMatrices();
		instanceShaderProgram->SetUniformMatrix4Array(uniformLocationProjectionMatrices, groupMemberPerspectiveMatrices);
		instanceShaderProgram->SetUniformMatrix4Array(uniformLocationViewMatrices, groupMemberViewMatrices);
		instanceShaderProgram->SetUniformVec4Array(uniformLocationDrawingBoundaries, groupMemberDrawingBoundaries);

		// Draw all specified entities for each player in the group
		for(Entity* currentEntity : entities) {
			instanceShaderProgram->SetUniformMatrix4(uniformLocationModelMatrix, currentEntity->GetModelMatrix());
			currentEntity->GetModel().DrawInstanced(GLsizei(group.GetNumberOfMembers()));
		}

		instanceShaderProgram->Unbind();
	}

}
