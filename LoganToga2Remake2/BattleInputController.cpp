#include "BattleInputController.h"

#include "BattleCommandUi.h"

namespace
{
	constexpr double CommandDragThreshold = 12.0;
	constexpr double SelectionDragThreshold = 12.0;
	constexpr double AttackClickSnapRadius = 20.0;

	void clearSelection(BattleState& state)
	{
		for (auto& unit : state.units)
		{
			unit.isSelected = false;
		}
	}

	void toggleSelection(BattleState& state, const int32 unitId, const bool additive)
	{
		const auto* current = state.findUnit(unitId);
		if (!current)
		{
			return;
		}

		const bool nextSelected = !current->isSelected;
		if (!additive)
		{
			clearSelection(state);
		}

		if (auto* unit = state.findUnit(unitId))
		{
			unit->isSelected = nextSelected;
		}
	}

	void resetCommandDrag(BattleState& state)
	{
		state.isCommandDragging = false;
		state.commandDragStart = Vec2::Zero();
		state.commandDragCurrent = Vec2::Zero();
	}

	void resetSelectionDrag(BattleState& state)
	{
		state.isSelecting = false;
		state.selectionStart = Vec2::Zero();
		state.selectionRect = RectF{ 0, 0, 0, 0 };
	}
}

void BattleInputController::handleFormationInput(BattleSession& session) const
{
	if (KeyQ.down())
	{
		session.enqueue(SetPlayerFormationCommand{ FormationType::Line });
	}

	if (KeyW.down())
	{
		session.enqueue(SetPlayerFormationCommand{ FormationType::Column });
	}

	if (KeyE.down())
	{
		session.enqueue(SetPlayerFormationCommand{ FormationType::Square });
	}
}

bool BattleInputController::isCursorOnCommandPanel(const BattleSession& session, const Vec2& cursorScreenPos) const
{
	const auto layout = BuildCommandPanelLayout(session.state(), session.config());
	return layout && layout->panelRect.intersects(cursorScreenPos);
}

bool BattleInputController::handleCommandPanelClick(BattleSession& session, const Vec2& cursorScreenPos) const
{
	if (!MouseL.down())
	{
		return false;
	}

	const auto layout = BuildCommandPanelLayout(session.state(), session.config());
	if (!(layout && layout->panelRect.intersects(cursorScreenPos)))
	{
		return false;
	}

	if (const auto command = HitTestCommandIcon(*layout, cursorScreenPos))
	{
		if (command->isEnabled)
		{
			auto& state = session.state();
			if (command->kind == CommandKind::Construction)
			{
				state.pendingConstructionArchetype = command->archetype;
				state.isSelecting = false;
				state.selectionRect = RectF{ 0, 0, 0, 0 };
			}
			else
			{
				session.trySpawnPlayerUnit(command->archetype);
			}
		}
	}

	return true;
}

void BattleInputController::handleSelectionInput(BattleSession& session, const Vec2& cursorWorldPos, const bool allowClickSelection) const
{
	auto& state = session.state();

	if (MouseR.down() && session.getSelectedPlayerUnitIds().isEmpty())
	{
		state.isSelecting = true;
		state.selectionStart = cursorWorldPos;
		state.selectionRect = RectF{ cursorWorldPos, 0, 0 };
	}

	if (state.isSelecting && MouseR.pressed())
	{
		state.selectionRect = RectF::FromPoints(state.selectionStart, cursorWorldPos);
	}

	if (state.isSelecting && MouseR.up())
	{
		const RectF selectionRect = state.selectionRect;
		const bool isDragSelection = (selectionRect.w >= SelectionDragThreshold)
			|| (selectionRect.h >= SelectionDragThreshold);
		if (isDragSelection)
		{
			session.enqueue(SelectUnitsInRectCommand{ selectionRect, KeyShift.pressed() });
		}

		resetSelectionDrag(state);
	}

	if (!allowClickSelection)
	{
		return;
	}

	const bool additive = KeyShift.pressed();

	if (const auto unitId = session.findPlayerUnitAt(cursorWorldPos))
	{
		toggleSelection(state, *unitId, additive);
	}
	else if (const auto buildingId = session.findPlayerBuildingAt(cursorWorldPos))
	{
		toggleSelection(state, *buildingId, additive);
	}
	else if (!additive)
	{
		clearSelection(state);
	}
}

void BattleInputController::handleCommandInput(BattleSession& session, const Vec2& cursorWorldPos) const
{
	auto& state = session.state();

	if (MouseR.down())
	{
		const auto selectedIds = session.getSelectedPlayerUnitIds();
		if (selectedIds.isEmpty())
		{
			resetCommandDrag(state);
			return;
		}

		state.isCommandDragging = true;
		state.commandDragStart = cursorWorldPos;
		state.commandDragCurrent = state.commandDragStart;
	}

	if (state.isCommandDragging && MouseR.pressed())
	{
		state.commandDragCurrent = cursorWorldPos;
	}

	if (!(state.isCommandDragging && MouseR.up()))
	{
		return;
	}

	state.commandDragCurrent = cursorWorldPos;
	const auto selectedIds = session.getSelectedPlayerUnitIds();
	if (selectedIds.isEmpty())
	{
		resetCommandDrag(state);
		return;
	}

	const Vec2 destination = state.commandDragCurrent;
	const Vec2 dragVector = (state.commandDragCurrent - state.commandDragStart);
	const bool isDragCommand = (dragVector.lengthSq() >= (CommandDragThreshold * CommandDragThreshold));

	if (!isDragCommand)
	{
		if (const auto targetUnitId = session.findEnemyAt(destination))
		{
			session.enqueue(AttackUnitCommand{ selectedIds, *targetUnitId });
		}
		else if (const auto targetUnitId = session.findEnemyNear(destination, AttackClickSnapRadius))
		{
			session.enqueue(AttackUnitCommand{ selectedIds, *targetUnitId });
		}
		else
		{
			session.enqueue(MoveUnitsCommand{ selectedIds, destination, state.playerFormation, Vec2::Zero() });
		}
	}
	else
	{
		session.enqueue(MoveUnitsCommand{ selectedIds, destination, state.playerFormation, dragVector });
	}

	resetCommandDrag(state);
}
