#ifndef SERVER__PLAYERRENDERGROUP__H
	#define SERVER__PLAYERRENDERGROUP__H

	#include <vector>

	#include "Player.h"
	#include "PlayerRenderGroupRow.h"
	#include "../graphics/FrameBuffer.h"

	namespace Server::Engine::Players {

		class PlayerRenderGroup {
			public:
				PlayerRenderGroup() {
					// Currently the frame buffer size is selected to accomodate 4x4 1920x1080 views
					m_FrameBuffer = new Graphics::FrameBuffer((int) std::pow(2, 13), (int) std::pow(2, 13));
				}

				bool AddPlayer(Player* player);

				void ClearPlayers();
				void DestroyGroup() {
					ClearPlayers();
					delete m_FrameBuffer;
				}

				void RecordStartOfRendering() {
					for(const PlayerRenderGroupRow& currentGroupRow : m_MemberRows) {
						for(const PlayerRenderGroupMember& currentMember : currentGroupRow.GetRowMembers()) {
							currentMember.GetPlayer()->GetProcessingDelayRecorder().RecordGroupRenderingStarted();
						}
					}
				}

				void RecordEndOfRendering() {
					for(const PlayerRenderGroupRow& currentGroupRow : m_MemberRows) {
						for(const PlayerRenderGroupMember& currentMember : currentGroupRow.GetRowMembers()) {
							currentMember.GetPlayer()->GetProcessingDelayRecorder().RecordGroupRenderingComplete();
						}
					}
				}

				void UpdatePlayerFrameCache() {
					// For each player we must copy their frame of gameplay from the group's frame buffer into their local frame of gameplay cache
					for(const PlayerRenderGroupRow& currentGroupRow : m_MemberRows) {
						for(const PlayerRenderGroupMember& currentMember : currentGroupRow.GetRowMembers()) {
							const int memberPosXInFrameBuffer = currentMember.GetOutputX();
							const int memberPosYInFrameBuffer = currentMember.GetOutputY();
							const int memberSizeWidth = currentMember.GetOutputWidth();
							const int memberSizeHeight = currentMember.GetOutputHeight();

							currentMember.GetPlayer()->GetCachedFrameDataMutex().lock();

							//TODO: Copying from the GPU is very costly, and we are currently copying once per player. A better approach here would be to copy once per row, and 
							//		then divide the CPU-side memory into the player's local cache. This would reduce the number of GPU copy operations, and allow the CPU to make 
							//		use of caching for faster processing.
							glReadPixels(memberPosXInFrameBuffer, memberPosYInFrameBuffer, memberSizeWidth, memberSizeHeight, GL_RGB, GL_UNSIGNED_BYTE, currentMember.GetPlayer()->GetCachedFrameData());

							currentMember.GetPlayer()->GetProcessingDelayRecorder().RecordPlayerFrameCopied();

							currentMember.GetPlayer()->SetCachedFrameDataDirty(true);
							currentMember.GetPlayer()->GetCachedFrameDataMutex().unlock();
						}
					}
				}

				size_t GetNumberOfMembers() const { return m_NumberOfMembers; }

				const std::vector<glm::mat4>& GetGroupMemberViewMatrices() const { return m_GroupMemberViewMatrices; }
				const std::vector<glm::vec4>& GetGroupMemberDrawingBoundaries() const { return m_GroupMemberDrawingBoundaries; }
				const std::vector<glm::mat4>& GetGroupMemberPerspectiveMatrices() const { return m_GroupMemberPerspectiveMatrices; }

				const Graphics::FrameBuffer* GetFrameBuffer() const { return m_FrameBuffer; }

			private:
				Graphics::FrameBuffer* m_FrameBuffer;

				size_t m_NumberOfMembers = 0;
				std::vector<PlayerRenderGroupRow> m_MemberRows;

				std::vector<glm::vec4> m_GroupMemberDrawingBoundaries;
				std::vector<glm::mat4> m_GroupMemberViewMatrices;
				std::vector<glm::mat4> m_GroupMemberPerspectiveMatrices;

			private:
				void AddPlayerToRow(PlayerRenderGroupRow& row, Player* player, int x, int y, int width, int height);
		};

	}

#endif