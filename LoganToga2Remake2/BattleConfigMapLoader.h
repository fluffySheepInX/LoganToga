#pragma once

#include "BattleConfigParsers.h"

inline void LoadBattleMapConfig(BattleConfigData& config, const TOMLReader& toml)
{
	config.initialUnits.clear();
	config.obstacles.clear();
	config.resourcePoints.clear();
	config.tutorial.enemyWaveUnits.clear();

	const auto initialUnits = toml[U"initial_units"];
	if (initialUnits.isTableArray())
	{
		for (const auto& table : initialUnits.tableArrayView())
		{
			InitialUnitPlacement placement;
			placement.owner = ParseOwner(table[U"owner"].get<String>());
			placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
			placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
			config.initialUnits << placement;
		}
	}

	const auto obstacles = toml[U"obstacles"];
	if (obstacles.isTableArray())
	{
		for (const auto& table : obstacles.tableArrayView())
		{
			ObstacleConfig obstacle;
			obstacle.label = table[U"label"].get<String>();
			obstacle.rect = RectF{ table[U"x"].get<double>(), table[U"y"].get<double>(), table[U"width"].get<double>(), table[U"height"].get<double>() };
			obstacle.blocksMovement = table[U"blocks_movement"].get<bool>();
			config.obstacles << obstacle;
		}
	}

	const auto resourcePoints = toml[U"resource_points"];
	if (resourcePoints.isTableArray())
	{
		for (const auto& table : resourcePoints.tableArrayView())
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
	}

	const auto enemyWaveUnits = toml[U"tutorial"][U"enemy_wave_units"];
	if (enemyWaveUnits.isTableArray())
	{
		for (const auto& table : enemyWaveUnits.tableArrayView())
		{
			InitialUnitPlacement placement;
			placement.owner = table[U"owner"].getOr<String>(U"enemy").lowercased() == U"player"
				? Owner::Player
				: Owner::Enemy;
			placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
			placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
			config.tutorial.enemyWaveUnits << placement;
		}
	}
}
