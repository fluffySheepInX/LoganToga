#include "BattleConstructionController.h"

namespace
{
	[[nodiscard]] bool HasSelectedWorker(const BattleState& state)
	{
		for (const auto& unit : state.units)
		{
			if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
			{
				return true;
			}
		}

		return false;
	}
}

bool BattleConstructionController::isConstructionSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 4:
		return Key4.down();
	case 5:
		return Key5.down();
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
			if (const auto workerUnitId = session.findSelectedPlayerWorkerId())
			{
				session.enqueue(IssueConstructionOrderCommand{ *workerUnitId, *state.pendingConstructionArchetype, state.buildingPreviewPosition });
			}
			state.pendingConstructionArchetype.reset();
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
			return;
		}

		return;
	}

	for (const auto& slot : session.config().playerConstructionSlots)
	{
		if (!ContainsArchetype(session.config().playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		if (isConstructionSlotTriggered(slot.slot) && HasSelectedWorker(state))
		{
			state.pendingConstructionArchetype = slot.archetype;
			state.buildingPreviewPosition = cursorWorldPos;
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
			break;
		}
	}
}
