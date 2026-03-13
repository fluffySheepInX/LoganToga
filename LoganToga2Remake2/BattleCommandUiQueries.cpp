#include "BattleCommandUi.h"

bool HasSelectedWorker(const BattleState& state)
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

Array<UnitArchetype> CollectSelectedBuildingArchetypes(const BattleState& state)
{
	Array<UnitArchetype> archetypes;

	for (const auto& unit : state.units)
	{
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && IsBuildingArchetype(unit.archetype)))
		{
			continue;
		}

		AppendUniqueArchetype(archetypes, unit.archetype);
	}

	return archetypes;
}

Optional<int32> FindSingleSelectedPlayerTurretId(const BattleState& state)
{
	int32 selectedCount = 0;
	Optional<int32> turretId;

	for (const auto& unit : state.units)
	{
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player)))
		{
			continue;
		}

		++selectedCount;
		if (unit.archetype == UnitArchetype::Turret)
		{
			turretId = unit.id;
		}
	}

	if ((selectedCount != 1) || !turretId)
	{
		return none;
	}

	const auto* building = state.findBuildingByUnitId(*turretId);
	if (!(building && building->isConstructed))
	{
		return none;
	}

	return turretId;
}

Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config)
{
	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	Array<CommandIconEntry> commands;

	for (const auto& slot : config.playerProductionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableProductionArchetypes, slot.archetype))
		{
			continue;
		}

		bool isAvailable = false;
		for (const auto producer : selectedBuildings)
		{
			if (producer == slot.producer)
			{
				isAvailable = true;
				break;
			}
		}

		if (!isAvailable)
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = (slot.cost > 0)
			? slot.cost
			: (definition ? definition->cost : 0);
		const String descriptionText = definition ? definition->description : U"";
		const String flavorText = definition ? definition->flavorText : U"";
		bool hasProducer = false;
		for (const auto& building : state.buildings)
		{
			const auto* unit = state.findUnit(building.unitId);
			if (unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == slot.producer) && building.isConstructed)
			{
				hasProducer = true;
				break;
			}
		}

		const bool hasEnoughGold = (state.playerGold >= cost);
		const bool isEnabled = (!state.winner) && hasProducer && (cost > 0) && hasEnoughGold;
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (!hasProducer)
		{
			statusText = U"PRODUCER OFFLINE";
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
			CommandKind::Production,
			slot.slot,
			slot.producer,
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

Array<CommandIconEntry> CollectTurretUpgradeCommands(const BattleState& state, const BattleConfigData& config)
{
	const auto turretId = FindSingleSelectedPlayerTurretId(state);
	if (!turretId)
	{
		return {};
	}

	const auto* building = state.findBuildingByUnitId(*turretId);
	if (!building)
	{
		return {};
	}

	Array<CommandIconEntry> commands;
	for (const auto& definition : config.turretUpgradeDefinitions)
	{
		const bool isUnlocked = ContainsTurretUpgradeType(config.playerAvailableTurretUpgrades, definition.type);
		const bool alreadyUpgraded = building->turretUpgrade.has_value();
		const bool hasEnoughGold = (state.playerGold >= definition.cost);
		const bool isEnabled = (!state.winner) && isUnlocked && !alreadyUpgraded && (definition.cost > 0) && hasEnoughGold;
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (!isUnlocked)
		{
			statusText = U"LOCKED";
		}
		else if (alreadyUpgraded)
		{
			statusText = U"UPGRADED";
		}
		else if (definition.cost <= 0)
		{
			statusText = U"UNAVAILABLE";
		}
		else if (!hasEnoughGold)
		{
			statusText = U"NOT ENOUGH GOLD";
		}

		commands << CommandIconEntry{
			CommandKind::Upgrade,
			definition.slot,
			UnitArchetype::Turret,
			UnitArchetype::Turret,
			definition.cost,
			isEnabled,
			statusText,
			definition.label,
			definition.glyph,
			definition.description,
			definition.flavorText,
			definition.type
		};
	}

	return commands;
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

Array<CommandIconEntry> CollectCommandEntries(const BattleState& state, const BattleConfigData& config)
{
	Array<CommandIconEntry> commands = CollectProductionCommands(state, config);
	for (const auto& command : CollectConstructionCommands(state, config))
	{
		commands << command;
	}
	for (const auto& command : CollectTurretUpgradeCommands(state, config))
	{
		commands << command;
	}

	return commands;
}
