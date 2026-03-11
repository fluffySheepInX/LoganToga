#pragma once

#include "BattleModel.h"

struct UnitDefinition
{
	UnitArchetype archetype = UnitArchetype::Soldier;
	double radius = 12.0;
	double moveSpeed = 80.0;
	double attackRange = 24.0;
	double attackCooldown = 0.7;
	int32 attackPower = 8;
	int32 hp = 40;
	int32 cost = 60;
	double productionTime = 1.5;
	bool canMove = true;
	double aggroRange = 170.0;
};

struct InitialUnitPlacement
{
	Owner owner = Owner::Player;
	UnitArchetype archetype = UnitArchetype::Soldier;
	Vec2 position = Vec2::Zero();
};

struct ProductionSlot
{
	int32 slot = 1;
	UnitArchetype producer = UnitArchetype::Base;
	UnitArchetype archetype = UnitArchetype::Soldier;
};

struct ConstructionSlot
{
	int32 slot = 4;
	UnitArchetype archetype = UnitArchetype::Barracks;
};

struct EnemySpawnConfig
{
	double interval = 4.0;
	UnitArchetype basicArchetype = UnitArchetype::Soldier;
	UnitArchetype advancedArchetype = UnitArchetype::Archer;
	double advancedProbability = 0.4;
	Vec2 position = Vec2{ 1130, 170 };
	double randomYOffset = 50.0;
};

struct EnemyAiConfig
{
	double decisionInterval = 0.4;
	int32 assaultUnitThreshold = 4;
	double defenseRadius = 240.0;
	double rallyDistance = 120.0;
	double baseAssaultLockRadius = 180.0;
};

struct IncomeConfig
{
	double interval = 1.0;
	int32 playerAmount = 20;
	int32 enemyAmount = 20;
};

struct WorldConfig
{
	double width = 1280.0;
	double height = 720.0;
};

struct HudConfig
{
	String title = U"LoganToga2 Remake Prototype";
	String controls = U"L drag: select / R click: move or attack / Q-W: formation / 1-3: queue / 4: build / X: cancel";
	String escapeHint = U"Esc: back to title";
	String winHint = U"Enter: title / R: retry";
};

struct ObstacleConfig
{
	String label = U"Obstacle";
	RectF rect{ 0, 0, 96, 96 };
	bool blocksMovement = true;
};

struct ResourcePointConfig
{
	String label = U"Resource";
	Vec2 position = Vec2::Zero();
	double radius = 42.0;
	int32 incomeAmount = 10;
	double captureTime = 2.0;
	Owner owner = Owner::Neutral;
};

struct BattleConfigData
{
	int32 playerGold = 200;
	int32 enemyGold = 200;
	WorldConfig world;
	IncomeConfig income;
	HudConfig hud;
	Array<UnitDefinition> unitDefinitions;
	Array<InitialUnitPlacement> initialUnits;
	Array<ProductionSlot> playerProductionSlots;
	Array<ConstructionSlot> playerConstructionSlots;
	Array<ObstacleConfig> obstacles;
	Array<ResourcePointConfig> resourcePoints;
	EnemySpawnConfig enemySpawn;
	EnemyAiConfig enemyAI;
};

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
	}

	for (const auto& table : toml[U"player_construction"].tableArrayView())
	{
		ConstructionSlot slot;
		slot.slot = table[U"slot"].get<int32>();
		slot.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		config.playerConstructionSlots << slot;
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

	return config;
}
