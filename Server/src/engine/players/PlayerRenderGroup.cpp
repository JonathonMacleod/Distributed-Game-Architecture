#include "PlayerRenderGroup.h"

#include "../graphics/Graphics.h"

#include "Middleware.h"

namespace Server::Engine::Players {

	bool PlayerRenderGroup::AddPlayer(Player* player) {
		const int playerOutputWidth = player->GetCamera().GetOutputWidth();
		const int playerOutputHeight = player->GetCamera().GetOutputHeight();

		int lastRowEndY = 0;

		// Check all existing rows of players to determine if it's possible to accomodate the new player in one of those
		for(size_t currentRowIndex = 0; currentRowIndex < m_MemberRows.size(); currentRowIndex++) {
			PlayerRenderGroupRow& currentRow = m_MemberRows.at(currentRowIndex);
			const bool currentRowIsLast = (currentRowIndex == (m_MemberRows.size() - 1));

			// If we are adding a player to this row, we have the distance between the end of the frame and the last player's output in the row
			const int widthRemainingForOutput = (m_FrameBuffer->GetWidth() - currentRow.GetCurrentEndX());
			// If this isn't the last row limit the height of this row to the existing row boundary (to avoid overlap). Otherwise, fill from the row start to the end of the frame
			const int heightRemainingForOutput = (currentRowIsLast ? (m_FrameBuffer->GetHeight() - currentRow.GetCurrentStartY()) : currentRow.GetCurrentHeight());

			if((playerOutputWidth <= widthRemainingForOutput) && (playerOutputHeight <= heightRemainingForOutput)) {
				AddPlayerToRow(currentRow, player, currentRow.GetCurrentEndX(), currentRow.GetCurrentStartY(), playerOutputWidth, playerOutputHeight);
				return true;
			}

			lastRowEndY = currentRow.GetCurrentEndY();
		}

		// It wasn't possible to add the player to any existing rows, so add a new row to accomodate the player (if possible)
		const int heightAvailableForNewRow = (m_FrameBuffer->GetHeight() - lastRowEndY);
		if((playerOutputWidth <= m_FrameBuffer->GetWidth()) && (playerOutputHeight <= heightAvailableForNewRow)) {
			// Create the row and add it to the list of rows
			PlayerRenderGroupRow newRow(0, lastRowEndY);
			m_MemberRows.push_back(newRow);

			// Add the player
			PlayerRenderGroupRow& newRowInList = m_MemberRows.at(m_MemberRows.size() - 1);
			AddPlayerToRow(newRowInList, player, 0, lastRowEndY, playerOutputWidth, playerOutputHeight);
			return true;
		}

		// We have failed to add the player to this group
		return false;
	}

	void PlayerRenderGroup::ClearPlayers() {
		m_MemberRows.clear();
		m_NumberOfMembers = 0;

		m_GroupMemberDrawingBoundaries.clear();
		m_GroupMemberViewMatrices.clear();
		m_GroupMemberPerspectiveMatrices.clear();
	}

	void PlayerRenderGroup::AddPlayerToRow(PlayerRenderGroupRow& row, Player* player, int x, int y, int width, int height) {
		// Add the player to the row
		row.AddPlayer(player, x, y, width, height);

		// Calculate OpenGL screen-space positions for the player's output and store the boundaries
		const float screenSpaceXScale = (2.0f / m_FrameBuffer->GetWidth());
		const float screenSpaceYScale = (2.0f / m_FrameBuffer->GetHeight());
		const float screenSpaceX = -1.0f + (x * screenSpaceXScale);
		const float screenSpaceY = -1.0f + (y * screenSpaceYScale);
		const float screenSpaceWidth = (width * screenSpaceXScale);
		const float screenSpaceHeight = (height * screenSpaceYScale);
		glm::vec4 screenSpaceBoundaries(screenSpaceX, screenSpaceY, screenSpaceWidth, screenSpaceHeight);
		m_GroupMemberDrawingBoundaries.push_back(screenSpaceBoundaries);

		// Calculate the player's current view matrix (e.g. their position/rotation) and store this
		m_GroupMemberViewMatrices.push_back(player->GetViewMatrix());

		// Store the player's perspective matrix
		m_GroupMemberPerspectiveMatrices.push_back(player->GetPerspectiveMatrix());

		// Update the number of members stored within this render group
		m_NumberOfMembers++;
	}

}