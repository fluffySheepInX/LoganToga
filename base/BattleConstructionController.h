#pragma once

#include "BattleSession.h"

class BattleConstructionController
{
public:
	void handleInput(BattleSession& session, const Vec2& cursorWorldPos) const;

private:
	[[nodiscard]] static bool isConstructionSlotTriggered(int32 slot);
};
