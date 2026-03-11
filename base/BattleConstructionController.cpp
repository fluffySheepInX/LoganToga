#include "BattleConstructionController.h"

bool BattleConstructionController::isConstructionSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 4:
		return Key4.down();
	default:
		return false;
	}
}

void BattleConstructionController::handleInput(BattleSession& session, const Vec2& cursorWorldPos) const
{
	auto& state = session.state();

	if (state.pendingConstructionArchetype)
	{
		state.buildingPreviewPosition = cursorWorldPos;

		if (MouseR.down())
		{
			state.pendingConstructionArchetype.reset();
			return;
		}

		if (MouseL.down())
		{
			session.enqueue(PlaceBuildingCommand{ *state.pendingConstructionArchetype, state.buildingPreviewPosition });
			state.pendingConstructionArchetype.reset();
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
			return;
		}

		return;
	}

	for (const auto& slot : session.config().playerConstructionSlots)
	{
		if (isConstructionSlotTriggered(slot.slot))
		{
			state.pendingConstructionArchetype = slot.archetype;
			state.buildingPreviewPosition = cursorWorldPos;
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
			break;
		}
	}
}
