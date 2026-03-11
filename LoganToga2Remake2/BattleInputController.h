#pragma once

#include "BattleSession.h"

class BattleInputController
{
public:
	void handleFormationInput(BattleSession& session) const;
	void handleSelectionInput(BattleSession& session, const Vec2& cursorWorldPos, bool allowClickSelection) const;
	void handleCommandInput(BattleSession& session, const Vec2& cursorWorldPos) const;
};
