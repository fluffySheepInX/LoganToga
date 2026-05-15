# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "SelectionSystem.h"
# include "BattleInputTypes.h"
# include "../UI/MapEditor.h"

namespace LT3
{
	inline Optional<BuildActionDefId> ResolveVisibleBuildActionId(const BattleWorld& world, const DefinitionStores& defs, UnitId builder, int32 commandIndex)
	{
		if (commandIndex < 0)
		{
			return none;
		}

		int32 visibleIndex = 0;
		for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
		{
			if (!CanUseBuildAction(world, defs, builder, defs.buildActions[i]))
			{
				continue;
			}

			if (visibleIndex == commandIndex)
			{
				return static_cast<BuildActionDefId>(i);
			}
			++visibleIndex;
		}

		return none;
	}

	inline void ResetActionPlacementPreview(BattleWorld& world)
	{
		world.selection.actionPlacementActive = false;
		world.selection.actionBuilder = InvalidUnitId;
		world.selection.actionId = InvalidBuildActionDefId;
		world.selection.actionLineDragging = false;
		world.selection.actionLineTargets.clear();
	}

	inline void BeginActionPlacementPreview(BattleWorld& world, UnitId builder, BuildActionDefId actionId, const Vec2& worldMouse)
	{
		world.selection.actionPlacementActive = true;
		world.selection.actionBuilder = builder;
		world.selection.actionId = actionId;
		world.selection.actionTargetWorld = SnapWorldPositionToBattleCellCenter(world, worldMouse);
		world.selection.actionLineDragging = false;
		world.selection.actionLineStartWorld = world.selection.actionTargetWorld;
		world.selection.actionLineTargets.clear();
	}

	inline bool IsLinePlacementAction(const BuildActionDef& action)
	{
		return action.placementMode == BuildPlacementMode::Line && action.useRightDragPlacement;
	}

	inline bool UpdateLineBuildPlacementInput(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& worldMouse, BattleInputIntent& intent)
	{
		if (!world.selection.actionPlacementActive || world.selection.actionId >= defs.buildActions.size())
		{
			return false;
		}

		const BuildActionDef& action = defs.buildActions[world.selection.actionId];
		if (!IsLinePlacementAction(action))
		{
			return false;
		}

		const Vec2 snappedMouse = SnapWorldPositionToBattleCellCenter(world, worldMouse);
		world.selection.actionTargetWorld = snappedMouse;

		if (KeyEscape.down())
		{
			ResetActionPlacementPreview(world);
			return true;
		}

		if (MouseR.down() && !IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
			world.selection.actionLineDragging = true;
			world.selection.actionLineStartWorld = snappedMouse;
			world.selection.actionLineTargets = BuildLinePlacementTargets(world, action, snappedMouse, snappedMouse);
			return true;
		}

		if (world.selection.actionLineDragging && MouseR.pressed())
		{
			world.selection.actionLineTargets = BuildLinePlacementTargets(world, action, world.selection.actionLineStartWorld, snappedMouse);
			return true;
		}

		if (world.selection.actionLineDragging && MouseR.up())
		{
			world.selection.actionLineTargets = BuildLinePlacementTargets(world, action, world.selection.actionLineStartWorld, snappedMouse);
			if (!world.selection.actionLineTargets.isEmpty())
			{
				intent.commands << BattleInputCommand{
					BattleInputCommandType::StartBuildLineAction,
					world.selection.actionBuilder,
					{},
					Vec2{ 0, 0 },
					Vec2{ 0, 0 },
					world.selection.actionId,
					world.selection.actionLineTargets,
					false,
					false
				};
			}
			ResetActionPlacementPreview(world);
			return true;
		}

		if (MouseL.down() && !IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
			intent.commands << BattleInputCommand{
				BattleInputCommandType::StartBuildAction,
				world.selection.actionBuilder,
				{},
				snappedMouse,
				Vec2{ 0, 0 },
				world.selection.actionId,
				{},
				false,
				true
			};
			ResetActionPlacementPreview(world);
			return true;
		}

		return true;
	}

	inline void QueueVisibleBuildCommand(BattleInputIntent& intent, BattleWorld& world, const DefinitionStores& defs, UnitId builder, int32 commandIndex, const Vec2& worldMouse)
	{
		if (IsBuilderBusyWithBuildQueue(world, builder))
		{
			return;
		}

		const Optional<BuildActionDefId> actionId = ResolveVisibleBuildActionId(world, defs, builder, commandIndex);
		if (!actionId)
		{
			return;
		}

		const BuildActionDef& action = defs.buildActions[*actionId];
		if (DoesBuildActionRequireTargetPosition(action))
		{
			BeginActionPlacementPreview(world, builder, *actionId, worldMouse);
			return;
		}

		intent.commands << BattleInputCommand{ BattleInputCommandType::StartBuildAction, builder, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, *actionId, {}, false, false };
	}
}
