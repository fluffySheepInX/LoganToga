#pragma once

#include "BattleConfigParsers.h"

inline void LoadBattleMapConfig(BattleConfigData& config, const TOMLReader& toml)
{
	config.initialUnits.clear();
	config.obstacles.clear();
	config.resourcePoints.clear();

	for (const auto& table : toml[U"initial_units"].tableArrayView())
	{
		InitialUnitPlacement placement;
		placement.owner = ParseOwner(table[U"owner"].get<String>());
		placement.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		placement.position = Vec2{ table[U"x"].get<double>(), table[U"y"].get<double>() };
		config.initialUnits << placement;
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
}
