#pragma once
#include "stdafx.h"
#include "ClassAStar.h"
#include "ClassHorizontalUnit.h"
#include "ClassUnitMovePlan.h"
#include "MapTile.h"

// Forward declaration to avoid circular dependency
// struct Unit;
// struct MapDetail;

class AStar
{
private:
	static constexpr int32 SEARCH_TIMEOUT_S = 10;
	static constexpr double RETREAT_DISTANCE = 50.0;

public:
	/// @brief
	AsyncTask<void> taskAStarEnemy;
	/// @brief
	AsyncTask<void> taskAStarMyUnits;
	/// @brief
	std::atomic<bool> abortAStarEnemy{ false };
	/// @brief
	std::atomic<bool> abortAStarMyUnits{ false };
	/// @brief
	std::atomic<bool> pauseAStarTaskEnemy{ false };
	/// @brief
	std::atomic<bool> pauseAStarTaskMyUnits{ false };

	std::atomic<bool> changeUnitMember{ false };
	std::mutex aiRootMutex;

	Optional<ClassAStar*> SearchMinScore(const Array<ClassAStar*>& ls);

	int32 BattleMoveAStar(std::mutex& unitListMutex,
		Array<ClassHorizontalUnit>& target,
		Array<ClassHorizontalUnit>& enemy,
		Array<Array<MapDetail>> mapData,
		HashTable<int64, ClassUnitMovePlan>& aiRoot,
		const std::atomic<bool>& abort,
		const std::atomic<bool>& pause,
		std::atomic<bool>& changeUnitMember,
		HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
		MapTile& mapTile
	);

	int32 BattleMoveAStarMyUnitsKai(std::mutex& unitListMutex,
		Array<ClassHorizontalUnit>& target,
		Array<ClassHorizontalUnit>& enemy,
		Array<Array<MapDetail>> mapData,
		HashTable<int64, ClassUnitMovePlan>& aiRoot,
		const std::atomic<bool>& abort,
		const std::atomic<bool>& pause,
		HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
		MapTile& mapTile
	);

private:
	Array<Point> findPath(const Point& start,
		const Point& end,
		Array<Array<MapDetail>>& mapData,
		Array<ClassHorizontalUnit>& enemy,
		Array<ClassHorizontalUnit>& target,
		MapTile& mapTile,
		HashTable<Point, Array<Unit*>>& hsBuildingUnitForAstar,
		const std::atomic<bool>& abort,
		bool isMyUnit);
};
