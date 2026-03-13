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
	case 5:
		return Key5.down();
	case 6:
		return Key6.down();
	case 7:
		return Key7.down();
	default:
		return false;
	}
}

void BattleConstructionController::handleInput(BattleSession& session, const Vec2& cursorWorldPos) const
{
	auto& state = session.state();

	if (state.pendingRepairTargeting)
	{
		state.repairPreviewPosition = cursorWorldPos;

		if (!HasSelectedWorker(state))
		{
			state.pendingRepairTargeting = false;
			return;
		}

		if (MouseR.down())
		{
			state.pendingRepairTargeting = false;
			return;
		}

		if (MouseL.down())
		{
			if (const auto buildingUnitId = session.findPlayerBuildingAt(cursorWorldPos))
			{
				if (const auto* buildingUnit = state.findUnit(*buildingUnitId);
					buildingUnit && (buildingUnit->archetype == UnitArchetype::Turret))
				{
					session.enqueue(IssueRepairOrderCommand{ session.getSelectedPlayerUnitIds(), *buildingUnitId });
					state.pendingRepairTargeting = false;
					state.isSelecting = false;
					state.selectionRect = RectF{ 0, 0, 0, 0 };
					return;
				}
			}

			state.statusMessage = U"Click a damaged turret";
			state.statusMessageTimer = 1.5;
		}

		return;
	}

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
			state.pendingRepairTargeting = false;
			state.buildingPreviewPosition = cursorWorldPos;
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
			break;
		}
	}
}
