#include "BattleCommandUi.h"

#include "BattleUiText.h"

namespace
{
	[[nodiscard]] bool HasSelectedPlayerArchetype(const BattleState& state, const UnitArchetype archetype)
	{
		for (const auto& unit : state.units)
		{
			if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == archetype))
			{
				return true;
			}
		}

		return false;
	}
}

Array<CommandIconEntry> CollectConstructionCommands(const BattleState& state, const BattleConfigData& config)
{
	if (!HasSelectedWorker(state))
	{
		return {};
	}

	if (state.tutorialActive && (state.tutorialPhase != TutorialPhase::BuildStructure))
	{
		return {};
	}

	Array<CommandIconEntry> commands;
	for (const auto& slot : config.playerConstructionSlots)
	{
		if (state.tutorialActive && (slot.archetype != config.tutorial.requiredConstruction))
		{
			continue;
		}

		if (!ContainsArchetype(config.playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = definition ? definition->cost : 0;
      const String descriptionText = BattleUiText::GetLocalizedArchetypeDescription(slot.archetype);
		const String flavorText = BattleUiText::GetLocalizedArchetypeFlavorText(slot.archetype);
		const bool hasEnoughGold = (state.playerGold >= cost);
		const bool isEnabled = (!state.winner) && (cost > 0) && hasEnoughGold;
       String statusText = BattleUiText::GetCommandStatusReady();
		if (state.winner)
		{
           statusText = BattleUiText::GetCommandStatusBattleEnded();
		}
		else if (cost <= 0)
		{
            statusText = BattleUiText::GetCommandStatusUnavailable();
		}
		else if (!hasEnoughGold)
		{
            statusText = BattleUiText::GetCommandStatusNotEnoughGold();
		}

		commands << CommandIconEntry{
			CommandKind::Construction,
			slot.slot,
			UnitArchetype::Worker,
			slot.archetype,
			cost,
			isEnabled,
			statusText,
			U"",
			U"",
			descriptionText,
			flavorText,
			none
		};
	}

	return commands;
}

Array<CommandIconEntry> CollectRepairCommands(const BattleState& state, const BattleConfigData&)
{
	if (!HasSelectedWorker(state))
	{
		return {};
	}

	bool hasConstructedRepairTarget = false;
	bool hasDamagedRepairTarget = false;
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (!(unit
			&& unit->isAlive
			&& (unit->owner == Owner::Player)
			&& ((unit->archetype == UnitArchetype::Turret) || (unit->archetype == UnitArchetype::Base))
			&& building.isConstructed))
		{
			continue;
		}

		hasConstructedRepairTarget = true;
		if (unit->hp < unit->maxHp)
		{
			hasDamagedRepairTarget = true;
			break;
		}
	}

	return {
		CommandIconEntry{
			CommandKind::Repair,
			8,
			UnitArchetype::Worker,
			UnitArchetype::Turret,
			0,
			!state.winner && hasDamagedRepairTarget,
           state.winner ? BattleUiText::GetCommandStatusBattleEnded() : (!hasConstructedRepairTarget ? BattleUiText::GetCommandStatusNoTarget() : (hasDamagedRepairTarget ? BattleUiText::GetCommandStatusReady() : BattleUiText::GetCommandStatusFullHp())),
			BattleUiText::GetRepairCommandLabel(),
			U"R",
           BattleUiText::GetRepairCommandDescription(),
			BattleUiText::GetRepairCommandFlavorText(),
			none
		}
	};
}

Array<CommandIconEntry> CollectDetonateCommands(const BattleState& state, const BattleConfigData&)
{
	if (!HasSelectedPlayerArchetype(state, UnitArchetype::Goliath))
	{
		return {};
	}

	bool hasReadyGoliath = false;
	for (const auto& unit : state.units)
	{
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Goliath)))
		{
			continue;
		}

		if (!unit.isDetonating)
		{
			hasReadyGoliath = true;
			break;
		}
	}

	return {
		CommandIconEntry{
			CommandKind::Detonate,
			7,
			UnitArchetype::Goliath,
			UnitArchetype::Goliath,
			0,
			!state.winner && hasReadyGoliath,
           state.winner ? BattleUiText::GetCommandStatusBattleEnded() : (hasReadyGoliath ? BattleUiText::GetCommandStatusReady() : BattleUiText::GetCommandStatusArmed()),
			BattleUiText::GetDetonateCommandLabel(),
			U"!",
         BattleUiText::GetDetonateCommandDescription(),
			BattleUiText::GetDetonateCommandFlavorText(),
			none
		}
	};
}
