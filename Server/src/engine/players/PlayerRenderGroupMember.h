#ifndef SERVER__PLAYERRENDERGROUPMEMBER__H
	#define SERVER__PLAYERRENDERGROUPMEMBER__H

	#include "Player.h"
	#include "../graphics/Graphics.h"

	#include "Middleware.h"
	#define COPY_METHOD_PER_PLAYER	1
	#define COPY_METHOD_PER_ROW		2
	#define COPY_METHOD_PER_GROUP	4
	#define COPY_METHOD COPY_METHOD_PER_ROW

	namespace Server::Engine::Players {
		
		class PlayerRenderGroupMember {
			public:
				PlayerRenderGroupMember(Player* player, int x, int y, int width, int height) {
					m_Player = player;
					m_OutputX = x;
					m_OutputY = y;
					m_OutputWidth = width;
					m_OutputHeight = height;
				}

				Player* GetPlayer() const { return m_Player; }
				int GetOutputX() const { return m_OutputX; }
				int GetOutputY() const { return m_OutputY; }
				int GetOutputWidth() const { return m_OutputWidth; }
				int GetOutputHeight() const { return m_OutputHeight; }

			private:
				Player* m_Player = nullptr;
				int m_OutputX = 0;
				int m_OutputY = 0;
				int m_OutputWidth = 0;
				int m_OutputHeight = 0;
		};

	}

#endif