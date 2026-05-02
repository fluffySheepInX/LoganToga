# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "CameraInputSystem.h"
# include "SelectionSystem.h"
# include "../UI/MapEditor.h"

namespace LT3
{
	enum class BattleInputCommandType
	{
		SelectUnit,
		MoveUnit,
		StartBuildAction,
	};

	struct BattleInputCommand
	{
		BattleInputCommandType type = BattleInputCommandType::SelectUnit;
		UnitId unit = InvalidUnitId;
      Array<UnitId> units;
		Vec2 position{ 0, 0 };
     Vec2 direction{ 0, 0 };
		BuildActionDefId buildAction = InvalidBuildActionDefId;
      bool useFormation = false;
	};

	struct BattleInputIntent
	{
		Array<BattleInputCommand> commands;
	};

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

	inline void QueueVisibleBuildCommand(BattleInputIntent& intent, const BattleWorld& world, const DefinitionStores& defs, UnitId builder, int32 commandIndex)
	{
		const Optional<BuildActionDefId> actionId = ResolveVisibleBuildActionId(world, defs, builder, commandIndex);
		if (!actionId)
		{
			return;
		}

        intent.commands << BattleInputCommand{ BattleInputCommandType::StartBuildAction, builder, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, *actionId, false };
	}

	inline Array<UnitId> PickUnitsInScreenRect(const BattleWorld& world, const DefinitionStores& defs, const RectF& screenRect, Faction faction)
	{
		Array<UnitId> units;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != faction)
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			const Vec2 pos = ToQuarterViewportScreen(world.units.position[unit]);
            if (screenRect.intersects(Circle{ pos, Max(6.0, UnitSelectionRadius(def) * GetQuarterViewCameraScale()) }))
			{
				units << unit;
			}
		}

		return units;
	}

	inline BattleInputIntent ReadBattleInput(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& worldMouse)
	{
		BattleInputIntent intent;
		const UnitId selectedBuilder = GetSelectedUnit(world);

		int32 visibleBuildCommandIndex = 0;
		for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
		{
			if (!CanUseBuildAction(world, defs, selectedBuilder, defs.buildActions[i]))
			{
				continue;
			}

			if (BattleCommandIconRect(mapEditor, visibleBuildCommandIndex).leftClicked())
			{
             intent.commands << BattleInputCommand{ BattleInputCommandType::StartBuildAction, selectedBuilder, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, static_cast<BuildActionDefId>(i), false };
				return intent;
			}
			++visibleBuildCommandIndex;
		}

		UnitId previewSelected = selectedBuilder;
		if (MouseL.down() && !IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
			if (const Optional<UnitId> unit = PickUnitAt(world, defs, worldMouse, Faction::Player))
			{
				previewSelected = *unit;
			}
			else
			{
				previewSelected = InvalidUnitId;
			}

            intent.commands << BattleInputCommand{ BattleInputCommandType::SelectUnit, previewSelected, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, InvalidBuildActionDefId, false };
		}

		if (Key1.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 0);
		if (Key2.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 1);
		if (Key3.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 2);

		return intent;
	}

	inline void ApplyBattleInputIntent(BattleWorld& world, const DefinitionStores& defs, const BattleInputIntent& intent)
	{
		for (const BattleInputCommand& command : intent.commands)
		{
			switch (command.type)
			{
			case BattleInputCommandType::SelectUnit:
				if (command.units.isEmpty())
				{
					SelectUnit(world, command.unit);
				}
				else
				{
					SelectUnits(world, command.units);
				}
				break;
			case BattleInputCommandType::MoveUnit:
				if (command.units.isEmpty())
				{
					IssueMove(world, command.unit, command.position);
				}
				else
				{
                 if (command.useFormation)
					{
						IssueFormationMove(world, defs, command.units, command.position, command.direction);
					}
					else
					{
                       for (const UnitId unit : command.units)
						{
							IssueMove(world, unit, command.position);
						}
					}
				}
				break;
			case BattleInputCommandType::StartBuildAction:
				TryStartBuild(world, defs, command.unit, command.buildAction);
				break;
			}
		}
	}

	inline void UpdateAreaSelectionDrag(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, BattleInputIntent& intent)
	{
		if (IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
			world.selection.areaDragging = false;
			return;
		}

		const Vec2 screenMouse = Cursor::PosF();
		if (MouseL.down())
		{
			world.selection.areaDragging = true;
			world.selection.areaDragStartScreen = screenMouse;
			world.selection.areaDragCurrentScreen = screenMouse;
		}
		else if (world.selection.areaDragging && MouseL.pressed())
		{
			world.selection.areaDragCurrentScreen = screenMouse;
		}
		else if (world.selection.areaDragging && MouseL.up())
		{
			world.selection.areaDragCurrentScreen = screenMouse;
			const RectF rect = MakeDragSelectionRect(world.selection.areaDragStartScreen, world.selection.areaDragCurrentScreen);
			if (rect.size.lengthSq() >= 36.0)
			{
               intent.commands << BattleInputCommand{ BattleInputCommandType::SelectUnit, InvalidUnitId, PickUnitsInScreenRect(world, defs, rect, Faction::Player), Vec2{ 0, 0 }, Vec2{ 0, 0 }, InvalidBuildActionDefId, false };
			}
			world.selection.areaDragging = false;
		}
	}

	inline void ResetFormationPlacementPreview(BattleWorld& world)
	{
		world.selection.formationPlacementActive = false;
		world.selection.formationUnits.clear();
	}

	inline void UpdateFormationPlacementPreview(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& worldMouse, BattleInputIntent& intent)
	{
		const bool canStart = IsCursorOnMapArea(mapEditor, world, defs)
			&& !IsCursorOnBuildCommandUi(world, defs, mapEditor)
			&& !GetSelectedUnits(world).isEmpty();

		if (MouseR.down())
		{
			if (canStart)
			{
				world.selection.formationPlacementActive = true;
				world.selection.formationUnits = GetSelectedUnits(world);
				world.selection.formationDestinationWorld = worldMouse;
				world.selection.formationCurrentWorld = worldMouse;
			}
			else
			{
				ResetFormationPlacementPreview(world);
			}
			return;
		}

		if (!world.selection.formationPlacementActive)
		{
			return;
		}

		if (MouseR.pressed())
		{
			world.selection.formationCurrentWorld = worldMouse;
			return;
		}

		if (MouseR.up())
		{
			world.selection.formationCurrentWorld = worldMouse;
			const Array<UnitId> previewUnits = world.selection.formationUnits;
			if (!previewUnits.isEmpty())
			{
				if (previewUnits.size() == 1)
				{
					intent.commands << BattleInputCommand{ BattleInputCommandType::MoveUnit, previewUnits.front(), {}, world.selection.formationDestinationWorld, Vec2{ 0, 0 }, InvalidBuildActionDefId, false };
				}
				else
				{
					intent.commands << BattleInputCommand{
						BattleInputCommandType::MoveUnit,
						previewUnits.front(),
						previewUnits,
						world.selection.formationDestinationWorld,
						ResolveFormationFacingDirection(world, previewUnits, world.selection.formationDestinationWorld, world.selection.formationCurrentWorld),
						InvalidBuildActionDefId,
						true
					};
				}
			}
			ResetFormationPlacementPreview(world);
			return;
		}

		ResetFormationPlacementPreview(world);
	}

	inline void HandleBattleInput(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& worldMouse)
	{
		BattleInputIntent intent = ReadBattleInput(world, defs, mapEditor, worldMouse);
		UpdateAreaSelectionDrag(world, defs, mapEditor, intent);
        UpdateFormationPlacementPreview(world, defs, mapEditor, worldMouse, intent);
		ApplyBattleInputIntent(world, defs, intent);
	}
}
