#pragma once

#include "BattleConfigTypes.h"

[[nodiscard]] inline bool ContainsArchetype(const Array<UnitArchetype>& archetypes, const UnitArchetype archetype)
{
	for (const auto current : archetypes)
	{
		if (current == archetype)
		{
			return true;
		}
	}

	return false;
}

inline void AppendUniqueArchetype(Array<UnitArchetype>& archetypes, const UnitArchetype archetype)
{
	if (!ContainsArchetype(archetypes, archetype))
	{
		archetypes << archetype;
	}
}

[[nodiscard]] inline bool ContainsTurretUpgradeType(const Array<TurretUpgradeType>& upgrades, const TurretUpgradeType type)
{
	for (const auto current : upgrades)
	{
		if (current == type)
		{
			return true;
		}
	}

	return false;
}

inline void AppendUniqueTurretUpgradeType(Array<TurretUpgradeType>& upgrades, const TurretUpgradeType type)
{
	if (!ContainsTurretUpgradeType(upgrades, type))
	{
		upgrades << type;
	}
}

[[nodiscard]] inline const UnitDefinition* FindUnitDefinition(const BattleConfigData& config, const UnitArchetype archetype)
{
	for (const auto& definition : config.unitDefinitions)
	{
		if (definition.archetype == archetype)
		{
			return &definition;
		}
	}

	return nullptr;
}

[[nodiscard]] inline const TurretUpgradeDefinition* FindTurretUpgradeDefinition(const BattleConfigData& config, const TurretUpgradeType type)
{
	for (const auto& definition : config.turretUpgradeDefinitions)
	{
		if (definition.type == type)
		{
			return &definition;
		}
	}

	return nullptr;
}

[[nodiscard]] inline const EnemyProgressionConfig* FindEnemyProgressionConfig(const BattleConfigData& config, const int32 battle)
{
	for (const auto& progression : config.enemyProgression)
	{
		if (progression.battle == battle)
		{
			return &progression;
		}
	}

	return nullptr;
}

[[nodiscard]] inline const PlayerUnitModifier* FindPlayerUnitModifier(const BattleConfigData& config, const UnitArchetype archetype)
{
	for (const auto& modifier : config.playerUnitModifiers)
	{
		if (modifier.archetype == archetype)
		{
			return &modifier;
		}
	}

	return nullptr;
}

[[nodiscard]] inline PlayerUnitModifier* FindPlayerUnitModifier(BattleConfigData& config, const UnitArchetype archetype)
{
	for (auto& modifier : config.playerUnitModifiers)
	{
		if (modifier.archetype == archetype)
		{
			return &modifier;
		}
	}

	return nullptr;
}
