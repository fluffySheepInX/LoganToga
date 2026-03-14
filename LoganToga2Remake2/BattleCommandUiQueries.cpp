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
	for (const auto& command : CollectRepairCommands(state, config))
	{
		commands << command;
	}
	for (const auto& command : CollectDetonateCommands(state, config))
	{
		commands << command;
	}

	return commands;
}
