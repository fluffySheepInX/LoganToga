#pragma once

#include "BattleConfigParsers.h"
#include "BattleConfigPathResolver.h"

[[nodiscard]] inline EnemyProgressionConfig* FindMutableEnemyProgressionConfig(BattleConfigData& config, const int32 battle)
{
	for (auto& progression : config.enemyProgression)
	{
		if (progression.battle == battle)
		{
			return &progression;
		}
	}

	return nullptr;
}

inline void LoadBattleProgressionConfig(BattleConfigData& config, const TOMLReader& toml, const String& sourcePath)
{
	config.enemyProgression.clear();

	for (const auto& table : toml[U"enemy_progression"].tableArrayView())
	{
		EnemyProgressionConfig progression;
		progression.battle = table[U"battle"].get<int32>();
		if (const auto mapSource = table[U"map_source"].getOpt<String>())
		{
			progression.mapSourcePath = ResolveBattleConfigSourcePath(sourcePath, *mapSource);
		}
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

inline void ApplyBattleProgressionConfigOverrides(BattleConfigData& config, const TOMLReader& toml, const String& sourcePath)
{
	if (!toml[U"enemy_progression"].isTableArray())
	{
		return;
	}

	for (const auto& table : toml[U"enemy_progression"].tableArrayView())
	{
		const int32 battle = table[U"battle"].get<int32>();
		auto* progression = FindMutableEnemyProgressionConfig(config, battle);
		if (!progression)
		{
			config.enemyProgression << EnemyProgressionConfig{};
			progression = &config.enemyProgression.back();
			progression->battle = battle;
		}

		if (const auto mapSource = table[U"map_source"].getOpt<String>())
		{
			progression->mapSourcePath = ResolveBattleConfigSourcePath(sourcePath, *mapSource);
		}
		progression->goldBonus = table[U"gold_bonus"].getOr<int32>(progression->goldBonus);
		progression->incomeBonus = table[U"income_bonus"].getOr<int32>(progression->incomeBonus);
		progression->spawnInterval = table[U"spawn_interval"].getOr<double>(progression->spawnInterval);
		progression->assaultUnitThreshold = table[U"assault_unit_threshold"].getOr<int32>(progression->assaultUnitThreshold);
		if (const auto modeValue = table[U"enemy_ai_mode"].getOpt<String>())
		{
			progression->overrideEnemyAiMode = true;
			progression->enemyAiMode = ParseEnemyAiMode(*modeValue);
		}
		progression->stagingAssaultMinUnits = table[U"staging_assault_min_units"].getOr<int32>(progression->stagingAssaultMinUnits);
		progression->stagingAssaultGatherRadius = table[U"staging_assault_gather_radius"].getOr<double>(progression->stagingAssaultGatherRadius);
		progression->stagingAssaultMaxWait = table[U"staging_assault_max_wait"].getOr<double>(progression->stagingAssaultMaxWait);
		progression->stagingAssaultCommitTime = table[U"staging_assault_commit_time"].getOr<double>(progression->stagingAssaultCommitTime);
		progression->extraBasicUnits = table[U"extra_basic_units"].getOr<int32>(progression->extraBasicUnits);
		progression->extraAdvancedUnits = table[U"extra_advanced_units"].getOr<int32>(progression->extraAdvancedUnits);
		progression->replaceEnemyInitialUnits = table[U"replace_enemy_initial_units"].getOr<bool>(progression->replaceEnemyInitialUnits);
		if (table[U"enemy_initial_units"].isTableArray())
		{
			progression->enemyInitialUnits.clear();
			for (const auto& unitTable : table[U"enemy_initial_units"].tableArrayView())
			{
				InitialUnitPlacement placement;
				placement.owner = Owner::Enemy;
				placement.archetype = ParseUnitArchetype(unitTable[U"archetype"].get<String>());
				placement.position = Vec2{ unitTable[U"x"].get<double>(), unitTable[U"y"].get<double>() };
				progression->enemyInitialUnits << placement;
			}
		}
	}
}
