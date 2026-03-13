#pragma once

#include "BattleSession.h"

class BattleInputController
{
public:
	[[nodiscard]] bool isCursorOnCommandPanel(const BattleSession& session, const Vec2& cursorScreenPos) const;
	[[nodiscard]] bool handleCommandPanelClick(BattleSession& session, const Vec2& cursorScreenPos) const;
	void handleSelectionInput(BattleSession& session, const Vec2& cursorWorldPos, bool allowClickSelection) const;
	void handleCommandInput(BattleSession& session, const Vec2& cursorWorldPos) const;
};
