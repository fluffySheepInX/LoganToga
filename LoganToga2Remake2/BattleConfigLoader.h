#pragma once

#include "BattleConfigLookup.h"

[[nodiscard]] inline UnitArchetype ParseUnitArchetype(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"base")
	{
		return UnitArchetype::Base;
	}
	if (normalized == U"barracks")
	{
		return UnitArchetype::Barracks;
	}
	if (normalized == U"turret")
	{
		return UnitArchetype::Turret;
	}
	if (normalized == U"worker")
	{
		return UnitArchetype::Worker;
	}
	if (normalized == U"soldier")
	{
		return UnitArchetype::Soldier;
	}
	if (normalized == U"archer")
	{
		return UnitArchetype::Archer;
	}

	throw Error{ U"Unknown unit archetype: " + value };
}

[[nodiscard]] inline Owner ParseOwner(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"player")
	{
		return Owner::Player;
	}
	if (normalized == U"enemy")
	{
		return Owner::Enemy;
	}
	if (normalized == U"neutral")
	{
		return Owner::Neutral;
	}

	throw Error{ U"Unknown owner: " + value };
}

[[nodiscard]] inline TurretUpgradeType ParseTurretUpgradeType(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"power")
	{
		return TurretUpgradeType::Power;
	}
	if ((normalized == U"rapid") || (normalized == U"speed"))
	{
		return TurretUpgradeType::Rapid;
	}
	if ((normalized == U"dual") || (normalized == U"hybrid") || (normalized == U"both"))
	{
		return TurretUpgradeType::Dual;
	}

	throw Error{ U"Unknown turret upgrade type: " + value };
}

[[nodiscard]] inline BattleConfigData LoadBattleConfig(const String& path)
{
	const TOMLReader toml{ path };
	if (!toml)
	{
		throw Error{ U"Failed to load battle config: " + path };
	}

	BattleConfigData config;
	config.playerGold = toml[U"economy"][U"player_gold"].get<int32>();
	config.enemyGold = toml[U"economy"][U"enemy_gold"].get<int32>();
	config.world.width = toml[U"world"][U"width"].get<double>();
	config.world.height = toml[U"world"][U"height"].get<double>();
	config.income.interval = toml[U"income"][U"interval"].get<double>();
	config.income.playerAmount = toml[U"income"][U"player_amount"].get<int32>();
	config.income.enemyAmount = toml[U"income"][U"enemy_amount"].get<int32>();
	config.hud.title = toml[U"hud"][U"title"].get<String>();
	config.hud.controls = toml[U"hud"][U"controls"].get<String>();
	config.hud.escapeHint = toml[U"hud"][U"escape_hint"].get<String>();
	config.hud.winHint = toml[U"hud"][U"win_hint"].get<String>();

	for (const auto& table : toml[U"units"].tableArrayView())
	{
		UnitDefinition definition;
		definition.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		definition.description = table[U"description"].getOr<String>(U"");
		definition.flavorText = table[U"flavor_text"].getOr<String>(U"");
		definition.radius = table[U"radius"].get<double>();
		definition.moveSpeed = table[U"move_speed"].get<double>();
		definition.attackRange = table[U"attack_range"].get<double>();
		definition.attackCooldown = table[U"attack_cooldown"].get<double>();
		definition.attackPower = table[U"attack_power"].get<int32>();
		definition.hp = table[U"hp"].get<int32>();
		definition.cost = table[U"cost"].get<int32>();
		definition.productionTime = table[U"production_time"].get<double>();
		definition.canMove = table[U"can_move"].get<bool>();
		definition.aggroRange = table[U"aggro_range"].get<double>();
		config.unitDefinitions << definition;
	}

	for (const auto& table : toml[U"initial_units"].tableArrayView())
	{
		InitialUnitPlacement placement;
		placement.owner = ParseOwner(table[U"owner"].get<String>());
		placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		config.initialUnits << placement;
	}

	for (const auto& table : toml[U"player_production"].tableArrayView())
	{
		ProductionSlot slot;
		slot.slot = table[U"slot"].get<int32>();
		slot.producer = ParseUnitArchetype(table[U"producer"].get<String>());
		slot.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		config.playerProductionSlots << slot;
		AppendUniqueArchetype(config.playerAvailableProductionArchetypes, slot.archetype);
	}

	for (const auto& table : toml[U"player_construction"].tableArrayView())
	{
		ConstructionSlot slot;
		slot.slot = table[U"slot"].get<int32>();
		slot.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		config.playerConstructionSlots << slot;
		AppendUniqueArchetype(config.playerAvailableConstructionArchetypes, slot.archetype);
	}

	for (const auto& table : toml[U"turret_upgrades"].tableArrayView())
	{
		TurretUpgradeDefinition definition;
		definition.type = ParseTurretUpgradeType(table[U"type"].get<String>());
		definition.slot = table[U"slot"].getOr<int32>(definition.slot);
		definition.label = table[U"label"].get<String>();
		definition.glyph = table[U"glyph"].get<String>();
		definition.description = table[U"description"].getOr<String>(U"");
		definition.flavorText = table[U"flavor_text"].getOr<String>(U"");
		definition.cost = table[U"cost"].getOr<int32>(definition.cost);
		definition.attackPowerDelta = table[U"attack_power_delta"].getOr<int32>(0);
		definition.attackCooldownDelta = table[U"attack_cooldown_delta"].getOr<double>(0.0);
		definition.unlockedByDefault = table[U"unlocked_by_default"].getOr<bool>(definition.unlockedByDefault);
		config.turretUpgradeDefinitions << definition;
		if (definition.unlockedByDefault)
		{
			AppendUniqueTurretUpgradeType(config.playerAvailableTurretUpgrades, definition.type);
		}
	}

	for (const auto& table : toml[U"obstacles"].tableArrayView())
	{
		ObstacleConfig obstacle;
		obstacle.label = table[U"label"].get<String>();
		obstacle.rect = RectF{ table[U"x"].get<double>(), table[U"y"].get<double>(), table[U"width"].get<double>(), table[U"height"].get<double>() };
		obstacle.blocksMovement = table[U"blocks_movement"].get<bool>();
		config.obstacles << obstacle;
	}

	for (const auto& table : toml[U"resource_points"].tableArrayView())
	{
		ResourcePointConfig resourcePoint;
		resourcePoint.label = table[U"label"].get<String>();
		resourcePoint.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		resourcePoint.radius = table[U"radius"].get<double>();
		resourcePoint.incomeAmount = table[U"income_amount"].get<int32>();
		resourcePoint.captureTime = table[U"capture_time"].get<double>();
		resourcePoint.owner = ParseOwner(table[U"owner"].get<String>());
		config.resourcePoints << resourcePoint;
	}

	config.enemySpawn.interval = toml[U"enemy_spawn"][U"interval"].get<double>();
	config.enemySpawn.basicArchetype = ParseUnitArchetype(toml[U"enemy_spawn"][U"basic_archetype"].get<String>());
	config.enemySpawn.advancedArchetype = ParseUnitArchetype(toml[U"enemy_spawn"][U"advanced_archetype"].get<String>());
	config.enemySpawn.advancedProbability = toml[U"enemy_spawn"][U"advanced_probability"].get<double>();
	config.enemySpawn.position = Vec2{ toml[U"enemy_spawn"][U"x"].get<double>(), toml[U"enemy_spawn"][U"y"].get<double>() };
	config.enemySpawn.randomYOffset = toml[U"enemy_spawn"][U"random_y_offset"].get<double>();
	config.enemyAI.decisionInterval = toml[U"enemy_ai"][U"decision_interval"].getOr<double>(config.enemyAI.decisionInterval);
	config.enemyAI.assaultUnitThreshold = toml[U"enemy_ai"][U"assault_unit_threshold"].getOr<int32>(config.enemyAI.assaultUnitThreshold);
	config.enemyAI.defenseRadius = toml[U"enemy_ai"][U"defense_radius"].getOr<double>(config.enemyAI.defenseRadius);
	config.enemyAI.rallyDistance = toml[U"enemy_ai"][U"rally_distance"].getOr<double>(config.enemyAI.rallyDistance);
	config.enemyAI.baseAssaultLockRadius = toml[U"enemy_ai"][U"base_assault_lock_radius"].getOr<double>(config.enemyAI.baseAssaultLockRadius);
	config.enemyAI.usePathfindingForAttackTarget = toml[U"enemy_ai"][U"use_pathfinding_for_attack_target"].getOr<bool>(config.enemyAI.usePathfindingForAttackTarget);

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

	return config;
}
