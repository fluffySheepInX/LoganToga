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
		if (const auto modeValue = table[U"enemy_ai_mode"].getOpt<String>())
		{
			progression.overrideEnemyAiMode = true;
			progression.enemyAiMode = ParseEnemyAiMode(*modeValue);
		}
		progression.stagingAssaultMinUnits = table[U"staging_assault_min_units"].getOr<int32>(0);
		progression.stagingAssaultGatherRadius = table[U"staging_assault_gather_radius"].getOr<double>(0.0);
		progression.stagingAssaultMaxWait = table[U"staging_assault_max_wait"].getOr<double>(0.0);
		progression.stagingAssaultCommitTime = table[U"staging_assault_commit_time"].getOr<double>(0.0);
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
