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
	config.tutorial.enabled = toml[U"tutorial"][U"enabled"].getOr<bool>(config.tutorial.enabled);
	config.tutorial.moveTarget = Vec2{
		toml[U"tutorial"][U"move_target_x"].getOr<double>(config.tutorial.moveTarget.x),
		toml[U"tutorial"][U"move_target_y"].getOr<double>(config.tutorial.moveTarget.y)
	};
	config.tutorial.moveTargetRadius = toml[U"tutorial"][U"move_target_radius"].getOr<double>(config.tutorial.moveTargetRadius);
	config.tutorial.prepareDelay = toml[U"tutorial"][U"prepare_delay"].getOr<double>(config.tutorial.prepareDelay);
	config.tutorial.enemyWaveDelay = toml[U"tutorial"][U"enemy_wave_delay"].getOr<double>(config.tutorial.enemyWaveDelay);
	config.tutorial.requiredProductionCount = toml[U"tutorial"][U"required_production_count"].getOr<int32>(config.tutorial.requiredProductionCount);
	if (const auto archetypeValue = toml[U"tutorial"][U"required_construction"].getOpt<String>())
	{
		config.tutorial.requiredConstruction = ParseUnitArchetype(*archetypeValue);
	}
	if (const auto archetypeValue = toml[U"tutorial"][U"required_production"].getOpt<String>())
	{
		config.tutorial.requiredProduction = ParseUnitArchetype(*archetypeValue);
	}
	config.tutorial.objectiveMove = toml[U"tutorial"][U"objective_move"].getOr<String>(config.tutorial.objectiveMove);
	config.tutorial.objectiveBuild = toml[U"tutorial"][U"objective_build"].getOr<String>(config.tutorial.objectiveBuild);
	config.tutorial.objectivePrepare = toml[U"tutorial"][U"objective_prepare"].getOr<String>(config.tutorial.objectivePrepare);
	config.tutorial.objectiveProduce = toml[U"tutorial"][U"objective_produce"].getOr<String>(config.tutorial.objectiveProduce);
	config.tutorial.objectiveDefend = toml[U"tutorial"][U"objective_defend"].getOr<String>(config.tutorial.objectiveDefend);
	config.tutorial.objectiveComplete = toml[U"tutorial"][U"objective_complete"].getOr<String>(config.tutorial.objectiveComplete);
}

inline void ApplyBattleCoreConfigOverrides(BattleConfigData& config, const TOMLReader& toml)
{
	config.playerGold = toml[U"economy"][U"player_gold"].getOr<int32>(config.playerGold);
	config.enemyGold = toml[U"economy"][U"enemy_gold"].getOr<int32>(config.enemyGold);
	config.world.width = toml[U"world"][U"width"].getOr<double>(config.world.width);
	config.world.height = toml[U"world"][U"height"].getOr<double>(config.world.height);
	config.income.interval = toml[U"income"][U"interval"].getOr<double>(config.income.interval);
	config.income.playerAmount = toml[U"income"][U"player_amount"].getOr<int32>(config.income.playerAmount);
	config.income.enemyAmount = toml[U"income"][U"enemy_amount"].getOr<int32>(config.income.enemyAmount);
	config.debug.enableEnemyAiSwitcher = toml[U"debug"][U"enable_enemy_ai_switcher"].getOr<bool>(config.debug.enableEnemyAiSwitcher);
	config.hud.title = toml[U"hud"][U"title"].getOr<String>(config.hud.title);
	config.hud.controls = toml[U"hud"][U"controls"].getOr<String>(config.hud.controls);
	config.hud.escapeHint = toml[U"hud"][U"escape_hint"].getOr<String>(config.hud.escapeHint);
	config.hud.winHint = toml[U"hud"][U"win_hint"].getOr<String>(config.hud.winHint);
	config.enemySpawn.interval = toml[U"enemy_spawn"][U"interval"].getOr<double>(config.enemySpawn.interval);
	if (const auto archetypeValue = toml[U"enemy_spawn"][U"basic_archetype"].getOpt<String>())
	{
		config.enemySpawn.basicArchetype = ParseUnitArchetype(*archetypeValue);
	}
	if (const auto archetypeValue = toml[U"enemy_spawn"][U"advanced_archetype"].getOpt<String>())
	{
		config.enemySpawn.advancedArchetype = ParseUnitArchetype(*archetypeValue);
	}
	config.enemySpawn.advancedProbability = toml[U"enemy_spawn"][U"advanced_probability"].getOr<double>(config.enemySpawn.advancedProbability);
	config.enemySpawn.position = Vec2{
		toml[U"enemy_spawn"][U"x"].getOr<double>(config.enemySpawn.position.x),
		toml[U"enemy_spawn"][U"y"].getOr<double>(config.enemySpawn.position.y)
	};
	config.enemySpawn.randomYOffset = toml[U"enemy_spawn"][U"random_y_offset"].getOr<double>(config.enemySpawn.randomYOffset);
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
	config.tutorial.enabled = toml[U"tutorial"][U"enabled"].getOr<bool>(config.tutorial.enabled);
	config.tutorial.moveTarget = Vec2{
		toml[U"tutorial"][U"move_target_x"].getOr<double>(config.tutorial.moveTarget.x),
		toml[U"tutorial"][U"move_target_y"].getOr<double>(config.tutorial.moveTarget.y)
	};
	config.tutorial.moveTargetRadius = toml[U"tutorial"][U"move_target_radius"].getOr<double>(config.tutorial.moveTargetRadius);
	config.tutorial.prepareDelay = toml[U"tutorial"][U"prepare_delay"].getOr<double>(config.tutorial.prepareDelay);
	config.tutorial.enemyWaveDelay = toml[U"tutorial"][U"enemy_wave_delay"].getOr<double>(config.tutorial.enemyWaveDelay);
	config.tutorial.requiredProductionCount = toml[U"tutorial"][U"required_production_count"].getOr<int32>(config.tutorial.requiredProductionCount);
	if (const auto archetypeValue = toml[U"tutorial"][U"required_construction"].getOpt<String>())
	{
		config.tutorial.requiredConstruction = ParseUnitArchetype(*archetypeValue);
	}
	if (const auto archetypeValue = toml[U"tutorial"][U"required_production"].getOpt<String>())
	{
		config.tutorial.requiredProduction = ParseUnitArchetype(*archetypeValue);
	}
	config.tutorial.objectiveMove = toml[U"tutorial"][U"objective_move"].getOr<String>(config.tutorial.objectiveMove);
	config.tutorial.objectiveBuild = toml[U"tutorial"][U"objective_build"].getOr<String>(config.tutorial.objectiveBuild);
	config.tutorial.objectivePrepare = toml[U"tutorial"][U"objective_prepare"].getOr<String>(config.tutorial.objectivePrepare);
	config.tutorial.objectiveProduce = toml[U"tutorial"][U"objective_produce"].getOr<String>(config.tutorial.objectiveProduce);
	config.tutorial.objectiveDefend = toml[U"tutorial"][U"objective_defend"].getOr<String>(config.tutorial.objectiveDefend);
	config.tutorial.objectiveComplete = toml[U"tutorial"][U"objective_complete"].getOr<String>(config.tutorial.objectiveComplete);
}
