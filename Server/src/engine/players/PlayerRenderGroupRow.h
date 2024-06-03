#ifndef SERVER__PLAYERRENDERGROUPROW__H
	#define SERVER__PLAYERRENDERGROUPROW__H

	#include <vector>

	#include "PlayerRenderGroupMember.h"

	namespace Server::Engine::Players {
	
		class PlayerRenderGroupRow {
			public:
				PlayerRenderGroupRow(int x, int y) {
					m_CurrentRowStartX = x;
					m_CurrentRowStartY = y;
					m_CurrentRowEndX = x;
					m_CurrentRowEndY = y;
				}

				void AddPlayer(Player* player, int x, int y, int width, int height, bool allowVerticalGrowth = true) {
					PlayerRenderGroupMember newGroupMember(player, x, y, width, height);
					m_RowMembers.push_back(newGroupMember);

					// Now update the boundaries of the row
					const int endX = (x + width);
					const int endY = (y + height);
					if(endX > m_CurrentRowEndX)
						m_CurrentRowEndX = endX;
					if(endY > m_CurrentRowEndY)
						m_CurrentRowEndY = endY;
				}

				int GetCurrentStartX() const { return m_CurrentRowStartX; }
				int GetCurrentStartY() const { return m_CurrentRowStartY; }
				int GetCurrentEndX() const { return m_CurrentRowEndX; }
				int GetCurrentEndY() const { return m_CurrentRowEndY; }
				int GetCurrentWidth() const { return (m_CurrentRowEndX - m_CurrentRowStartX); }
				int GetCurrentHeight() const { return (m_CurrentRowEndY - m_CurrentRowStartY); }

				size_t GetNumberOfMembers() const { return m_RowMembers.size(); }
				const std::vector<PlayerRenderGroupMember>& GetRowMembers() const { return m_RowMembers; }

			private:
				int m_CurrentRowStartX = 0;
				int m_CurrentRowStartY = 0;
				int m_CurrentRowEndX = 0;
				int m_CurrentRowEndY = 0;

				std::vector<PlayerRenderGroupMember> m_RowMembers;
		};

	}

#endif