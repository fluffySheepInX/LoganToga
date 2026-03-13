#pragma once

#include "BattleConfigParsers.h"

inline void LoadBattleCoreConfig(BattleConfigData& config, const TOMLReader& toml)
{
	config.playerGold = toml[U"economy"][U"player_gold"].get<int32>();
	config.enemyGold = toml[U"economy"][U"enemy_gold"].get<int32>();
	config.world.width = toml[U"world"][U"width"].get<double>();
	config.world.height = toml[U"world"][U"height"].get<double>();
	config.income.interval = toml[U"income"][U"interval"].get<double>();
	config.income.playerAmount = toml[U"income"][U"player_amount"].get<int32>();
	config.income.enemyAmount = toml[U"income"][U"enemy_amount"].get<int32>();
	config.debug.enableEnemyAiSwitcher = toml[U"debug"][U"enable_enemy_ai_switcher"].getOr<bool>(config.debug.enableEnemyAiSwitcher);
	config.hud.title = toml[U"hud"][U"title"].get<String>();
	config.hud.controls = toml[U"hud"][U"controls"].get<String>();
	config.hud.escapeHint = toml[U"hud"][U"escape_hint"].get<String>();
	config.hud.winHint = toml[U"hud"][U"win_hint"].get<String>();
	config.enemySpawn.interval = toml[U"enemy_spawn"][U"interval"].get<double>();
	config.enemySpawn.basicArchetype = ParseUnitArchetype(toml[U"enemy_spawn"][U"basic_archetype"].get<String>());
	config.enemySpawn.advancedArchetype = ParseUnitArchetype(toml[U"enemy_spawn"][U"advanced_archetype"].get<String>());
	config.enemySpawn.advancedProbability = toml[U"enemy_spawn"][U"advanced_probability"].get<double>();
	config.enemySpawn.position = Vec2{ toml[U"enemy_spawn"][U"x"].get<double>(), toml[U"enemy_spawn"][U"y"].get<double>() };
	config.enemySpawn.randomYOffset = toml[U"enemy_spawn"][U"random_y_offset"].get<double>();
	if (const auto modeValue = toml[U"enemy_ai"][U"mode"].getOpt<String>())
	{
		config.enemyAI.mode = ParseEnemyAiMode(*modeValue);
	}
	config.enemyAI.decisionInterval = toml[U"enemy_ai"][U"decision_interval"].getOr<double>(config.enemyAI.decisionInterval);
	config.enemyAI.assaultUnitThreshold = toml[U"enemy_ai"][U"assault_unit_threshold"].getOr<int32>(config.enemyAI.assaultUnitThreshold);
	config.enemyAI.defenseRadius = toml[U"enemy_ai"][U"defense_radius"].getOr<double>(config.enemyAI.defenseRadius);
	config.enemyAI.rallyDistance = toml[U"enemy_ai"][U"rally_distance"].getOr<double>(config.enemyAI.rallyDistance);
	config.enemyAI.baseAssaultLockRadius = toml[U"enemy_ai"][U"base_assault_lock_radius"].getOr<double>(config.enemyAI.baseAssaultLockRadius);
	config.enemyAI.stagingAssaultMinUnits = toml[U"enemy_ai"][U"staging_assault_min_units"].getOr<int32>(config.enemyAI.stagingAssaultMinUnits);
	config.enemyAI.stagingAssaultGatherRadius = toml[U"enemy_ai"][U"staging_assault_gather_radius"].getOr<double>(config.enemyAI.stagingAssaultGatherRadius);
	config.enemyAI.stagingAssaultMaxWait = toml[U"enemy_ai"][U"staging_assault_max_wait"].getOr<double>(config.enemyAI.stagingAssaultMaxWait);
	config.enemyAI.stagingAssaultCommitTime = toml[U"enemy_ai"][U"staging_assault_commit_time"].getOr<double>(config.enemyAI.stagingAssaultCommitTime);
	config.enemyAI.usePathfindingForAttackTarget = toml[U"enemy_ai"][U"use_pathfinding_for_attack_target"].getOr<bool>(config.enemyAI.usePathfindingForAttackTarget);
}
