#pragma once

#include "BattleConfigParsers.h"

inline void LoadBattleProgressionConfig(BattleConfigData& config, const TOMLReader& toml)
{
	config.enemyProgression.clear();

	for (const auto& table : toml[U"enemy_progression"].tableArrayView())
	{
		EnemyProgressionConfig progression;
		progression.battle = table[U"battle"].get<int32>();
		progression.goldBonus = table[U"gold_bonus"].getOr<int32>(0);
		progression.incomeBonus = table[U"income_bonus"].getOr<int32>(0);
		progression.spawnInterval = table[U"spawn_interval"].getOr<double>(0.0);
		progression.assaultUnitThreshold = table[U"assault_unit_threshold"].getOr<int32>(0);
		progression.extraBasicUnits = table[U"extra_basic_units"].getOr<int32>(0);
		progression.extraAdvancedUnits = table[U"extra_advanced_units"].getOr<int32>(0);
		progression.replaceEnemyInitialUnits = table[U"replace_enemy_initial_units"].getOr<bool>(false);
		for (const auto& unitTable : table[U"enemy_initial_units"].tableArrayView())
		{
			InitialUnitPlacement placement;
			placement.owner = Owner::Enemy;
			placement.archetype = ParseUnitArchetype(unitTable[U"archetype"].get<String>());
			placement.position = Vec2{ unitTable[U"x"].get<double>(), unitTable[U"y"].get<double>() };
			progression.enemyInitialUnits << placement;
		}
		config.enemyProgression << progression;
	}
}
