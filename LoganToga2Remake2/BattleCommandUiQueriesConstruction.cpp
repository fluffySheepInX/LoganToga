#include "BattleCommandUi.h"

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

	Array<CommandIconEntry> commands;
	for (const auto& slot : config.playerConstructionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = definition ? definition->cost : 0;
		const String descriptionText = definition ? definition->description : U"";
		const String flavorText = definition ? definition->flavorText : U"";
		const bool hasEnoughGold = (state.playerGold >= cost);
		const bool isEnabled = (!state.winner) && (cost > 0) && hasEnoughGold;
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (cost <= 0)
		{
			statusText = U"UNAVAILABLE";
		}
		else if (!hasEnoughGold)
		{
			statusText = U"NOT ENOUGH GOLD";
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

	bool hasConstructedTurret = false;
	bool hasDamagedTurret = false;
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (!(unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == UnitArchetype::Turret) && building.isConstructed))
		{
			continue;
		}

		hasConstructedTurret = true;
		if (unit->hp < unit->maxHp)
		{
			hasDamagedTurret = true;
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
			!state.winner && hasDamagedTurret,
			state.winner ? U"BATTLE ENDED" : (!hasConstructedTurret ? U"NO TURRET" : (hasDamagedTurret ? U"READY" : U"FULL HP")),
			U"REPAIR",
			U"R",
			U"After pressing this, click a damaged turret to repair it.",
			U"先に命令してから、直したい砲台を選ぶ。",
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
			state.winner ? U"BATTLE ENDED" : (hasReadyGoliath ? U"READY" : U"ARMED"),
			U"DETONATE",
			U"!",
			U"Self-destruct selected Goliaths after a short fuse.",
			U"近づけて押すだけ。落とされても周囲を巻き込む。",
			none
		}
	};
}
