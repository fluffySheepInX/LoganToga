# pragma once
# include "BattleOrders.h"
# include "CameraInputSystem.h"
# include "BattleBuildInputSystem.h"
# include "BattleSelectionInputSystem.h"

namespace LT3
{
	inline bool CanIssueMoveOrder(const BattleWorld& world, const DefinitionStores& defs, UnitId unit)
	{
		if (!IsValidUnit(world, unit) || unit >= world.units.defId.size() || world.units.defId[unit] >= defs.units.size())
		{
			return false;
		}

		const UnitDef& def = defs.units[world.units.defId[unit]];
		return def.speed > 0.0 && def.role != UnitRole::Base && def.role != UnitRole::Barrier;
	}

   inline BattleInputIntent ReadBattleInput(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& screenMouse, const Vec2& worldMouse)
	{
		BattleInputIntent intent;
		const UnitId selectedBuilder = GetSelectedUnit(world);
		const Optional<size_t> hoveredResourceNode = FindHoveredResourceNode(world, screenMouse);
		world.selection.hoveredResourceNode = hoveredResourceNode ? static_cast<int32>(*hoveredResourceNode) : -1;

		const Array<BattleSkillFilterKind> skillFilters = CollectBattleSkillFilterKinds();
		for (int32 filterIndex = 0; filterIndex < static_cast<int32>(skillFilters.size()); ++filterIndex)
		{
			if (BattleSkillFilterButtonRect(BattleSkillPanelRect(), filterIndex).leftClicked())
			{
				world.selection.skillFilter = skillFilters[filterIndex];
				world.selection.selectedSkill = InvalidSkillDefId;
				return intent;
			}
		}

		const Array<SkillDefId> visibleSkills = CollectVisibleBattleSkills(world, defs);
		for (int32 skillIndex = 0; skillIndex < static_cast<int32>(visibleSkills.size()); ++skillIndex)
		{
			if (!BattleSkillIconRect(BattleSkillPanelRect(), skillIndex).leftClicked())
			{
				continue;
			}

			const SkillDefId skillId = visibleSkills[skillIndex];
			world.selection.selectedSkill = (world.selection.selectedSkill == skillId) ? InvalidSkillDefId : skillId;
			return intent;
		}

		if (world.selection.actionPlacementActive)
		{
            if (UpdateLineBuildPlacementInput(world, defs, mapEditor, worldMouse, intent))
			{
				return intent;
			}

		 world.selection.actionTargetWorld = SnapWorldToBattleCellCenter(world, worldMouse);
			if (MouseR.down() || KeyEscape.down())
			{
				ResetActionPlacementPreview(world);
				return intent;
			}

		if (MouseL.down() && !IsCursorOnBuildCommandUi(world, defs, mapEditor) && !IsCursorOnBattleSkillUi(world, defs))
			{
				intent.commands << BattleInputCommand{
					BattleInputCommandType::StartBuildAction,
					world.selection.actionBuilder,
					{},
					world.selection.actionTargetWorld,
					Vec2{ 0, 0 },
					world.selection.actionId,
                  {},
					false,
					true
				};
				ResetActionPlacementPreview(world);
			}
			return intent;
		}

		int32 visibleBuildCommandIndex = 0;
     int32 visibleBuildCommandCount = 0;
		for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
		{
			if (CanUseBuildAction(world, defs, selectedBuilder, defs.buildActions[i]))
			{
				++visibleBuildCommandCount;
			}
		}

		const int32 visibleBuildCommandRows = (visibleBuildCommandCount + 2) / 3;
		for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
		{
			if (!CanUseBuildAction(world, defs, selectedBuilder, defs.buildActions[i]))
			{
				continue;
			}

           if (BattleCommandIconRect(mapEditor, visibleBuildCommandIndex, visibleBuildCommandRows).leftClicked())
			{
               const BuildActionDef& action = defs.buildActions[i];
				if (DoesBuildActionRequireTargetPosition(action))
				{
					BeginActionPlacementPreview(world, selectedBuilder, static_cast<BuildActionDefId>(i), worldMouse);
					return intent;
				}

             intent.commands << BattleInputCommand{ BattleInputCommandType::StartBuildAction, selectedBuilder, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, static_cast<BuildActionDefId>(i), {}, false, false };
				return intent;
			}
			++visibleBuildCommandIndex;
		}

		UnitId previewSelected = selectedBuilder;
		if (MouseL.down() && !IsCursorOnBuildCommandUi(world, defs, mapEditor))
		{
		 if (hoveredResourceNode && !GetSelectedUnits(world).isEmpty())
			{
				for (const UnitId unit : GetSelectedUnits(world))
				{
					if (!IsBuildQueueLocked(world, unit) && CanIssueMoveOrder(world, defs, unit))
					{
						IssueMoveToResourceNode(world, unit, *hoveredResourceNode);
					}
				}
				return intent;
			}

			Optional<UnitId> unit = PickUnitAt(world, defs, worldMouse, Faction::Player);
			if (!unit)
			{
				unit = PickUnitAt(world, defs, worldMouse, Faction::Enemy);
			}
			if (unit)
			{
				previewSelected = *unit;
			}
			else
			{
				previewSelected = InvalidUnitId;
			}

               intent.commands << BattleInputCommand{ BattleInputCommandType::SelectUnit, previewSelected, {}, Vec2{ 0, 0 }, Vec2{ 0, 0 }, InvalidBuildActionDefId, {}, false, false };
		}

     if (Key1.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 0, worldMouse);
		if (Key2.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 1, worldMouse);
		if (Key3.down()) QueueVisibleBuildCommand(intent, world, defs, selectedBuilder, 2, worldMouse);

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
				   if (!IsBuildQueueLocked(world, command.unit) && CanIssueMoveOrder(world, defs, command.unit))
					{
						IssueMove(world, command.unit, command.position);
					}
				}
				else
				{
				 if (command.useFormation)
					{
						Array<UnitId> movableUnits;
						for (const UnitId unit : command.units)
						{
							if (!IsBuildQueueLocked(world, unit) && CanIssueMoveOrder(world, defs, unit))
							{
								movableUnits << unit;
							}
						}
						IssueFormationMove(world, defs, movableUnits, command.position, command.direction);
					}
					else
					{
					   for (const UnitId unit : command.units)
						{
						   if (!IsBuildQueueLocked(world, unit) && CanIssueMoveOrder(world, defs, unit))
							{
								IssueMove(world, unit, command.position);
							}
						}
					}
				}
				break;
					case BattleInputCommandType::StartBuildAction:
						TryStartBuild(world, defs, command.unit, command.buildAction, command.hasTargetPosition ? Optional<Vec2>{ command.position } : none);
							break;
						case BattleInputCommandType::StartBuildLineAction:
							TryStartBuildLine(world, defs, command.unit, command.buildAction, command.positions);
							break;
						}
					}
				}

				inline void HandleBattleInput(BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Vec2& screenMouse, const Vec2& worldMouse)
				{
					BattleInputIntent intent = ReadBattleInput(world, defs, mapEditor, screenMouse, worldMouse);
					UpdateAreaSelectionDrag(world, defs, mapEditor, intent);
					UpdateFormationPlacementPreview(world, defs, mapEditor, worldMouse, intent);
					ApplyBattleInputIntent(world, defs, intent);
				}
			}
