#pragma once

#include "BattleConfigParsers.h"

inline void ReplaceBattleInitialUnits(Array<InitialUnitPlacement>& placements, const auto& initialUnits)
{
	placements.clear();
	if (!initialUnits.isTableArray())
	{
		return;
	}

	for (const auto& table : initialUnits.tableArrayView())
	{
		InitialUnitPlacement placement;
		placement.owner = ParseOwner(table[U"owner"].get<String>());
		placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		placements << placement;
	}
}

inline void ReplaceBattleObstacles(Array<ObstacleConfig>& obstacles, const auto& obstacleTables)
{
	obstacles.clear();
	if (!obstacleTables.isTableArray())
	{
		return;
	}

	for (const auto& table : obstacleTables.tableArrayView())
	{
		ObstacleConfig obstacle;
		obstacle.label = table[U"label"].get<String>();
		obstacle.rect = RectF{ table[U"x"].get<double>(), table[U"y"].get<double>(), table[U"width"].get<double>(), table[U"height"].get<double>() };
		obstacle.blocksMovement = table[U"blocks_movement"].get<bool>();
		obstacles << obstacle;
	}
}

inline void ReplaceBattleResourcePoints(Array<ResourcePointConfig>& resourcePoints, const auto& resourcePointTables)
{
	resourcePoints.clear();
	if (!resourcePointTables.isTableArray())
	{
		return;
	}

	for (const auto& table : resourcePointTables.tableArrayView())
	{
		ResourcePointConfig resourcePoint;
		resourcePoint.label = table[U"label"].get<String>();
		resourcePoint.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		resourcePoint.radius = table[U"radius"].get<double>();
		resourcePoint.incomeAmount = table[U"income_amount"].get<int32>();
		resourcePoint.captureTime = table[U"capture_time"].get<double>();
		resourcePoint.owner = ParseOwner(table[U"owner"].get<String>());
		resourcePoints << resourcePoint;
	}
}

inline void ReplaceTutorialEnemyWaveUnits(Array<InitialUnitPlacement>& placements, const auto& enemyWaveUnits)
{
	placements.clear();
	if (!enemyWaveUnits.isTableArray())
	{
		return;
	}

	for (const auto& table : enemyWaveUnits.tableArrayView())
	{
		InitialUnitPlacement placement;
		placement.owner = table[U"owner"].getOr<String>(U"enemy").lowercased() == U"player"
			? Owner::Player
			: Owner::Enemy;
		placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		placements << placement;
	}
}

inline void LoadBattleMapConfig(BattleConfigData& config, const TOMLReader& toml)
{
	ReplaceBattleInitialUnits(config.initialUnits, toml[U"initial_units"]);
	ReplaceBattleObstacles(config.obstacles, toml[U"obstacles"]);
	ReplaceBattleResourcePoints(config.resourcePoints, toml[U"resource_points"]);
	ReplaceTutorialEnemyWaveUnits(config.tutorial.enemyWaveUnits, toml[U"tutorial"][U"enemy_wave_units"]);
}

inline void ApplyBattleMapConfigOverrides(BattleConfigData& config, const TOMLReader& toml)
{
	if (toml[U"initial_units"].isTableArray())
	{
		ReplaceBattleInitialUnits(config.initialUnits, toml[U"initial_units"]);
	}

	if (toml[U"obstacles"].isTableArray())
	{
		ReplaceBattleObstacles(config.obstacles, toml[U"obstacles"]);
	}

	if (toml[U"resource_points"].isTableArray())
	{
		ReplaceBattleResourcePoints(config.resourcePoints, toml[U"resource_points"]);
	}

	if (toml[U"tutorial"][U"enemy_wave_units"].isTableArray())
	{
		ReplaceTutorialEnemyWaveUnits(config.tutorial.enemyWaveUnits, toml[U"tutorial"][U"enemy_wave_units"]);
	}
}
