#ifndef SERVER__SCENE__H
	#define SERVER__SCENE__H

	#include <vector>

	#include "Entity.h"
	#include "../players/Player.h"
	#include "../players/PlayerRenderGroup.h"

	namespace Server::Engine::Graphics {
	
		class Scene {
			public:
				Scene(ShaderProgram* individualShader, ShaderProgram* instanceShader) { 
					m_IndividualShaderProgram = individualShader;
					m_InstanceShaderProgram = instanceShader; 
				}
				~Scene();

				void AddEntity(Entity* entity) { m_AllEntities.push_back(entity); }
				void AddPlayer(Players::Player* player);
				void RemovePlayer(Players::Player* player);

				void Update(double secondsSinceLastUpdate);
				void DrawPlayerPerspectives();

				int GetNumberOfPlayers() const;

				//TODO: Remove this function once the framebuffer no longer needs to be displayed for debugging purposes. This is a dangerous function that does not check whether a render 
				//		group exists
				const FrameBuffer* GetFramebuffer() { return m_AllPlayerRenderGroups.at(0).GetFrameBuffer(); }
				//TODO: Remove this function once the cached player frame of gameplay no longer needs to be displayed for debugging purposes. This is a dangerous function that does not
				//		check whether a player exists
				Players::Player* GetPlayer() { return m_AllPlayers.at(0); }

			private:
				ShaderProgram* m_InstanceShaderProgram = nullptr;
				ShaderProgram* m_IndividualShaderProgram = nullptr;

				std::vector<Entity*> m_AllEntities;

				std::vector<Players::Player*> m_AllPlayers;
				std::vector<Players::PlayerRenderGroup> m_AllPlayerRenderGroups;

			private:
				void FillRenderGroups();
				void ClearRenderGroups();
				void DestroyRenderGroups();

				void DrawIndividualEntities(const ShaderProgram* individualShaderProgram, const Players::PlayerRenderGroup& group, const std::vector<Entity*>& entities);
				void DrawInstancedEntities(const ShaderProgram* instanceShaderProgram, const Players::PlayerRenderGroup& group, const std::vector<Entity*>& entities);
		};

	}

#endif