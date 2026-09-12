# pragma once
# include <Siv3D.hpp>
# include "../Pathfinding/PathfindingCore.h"
# include "BattleQueries.h"
# include "BattleUnitState.h"
# include "../UI/QuarterView.h"

namespace LT3
{
	struct PathMapSnapshot
	{
		int32 width = 0;
		int32 height = 0;
		Array<uint8> blocked;
		uint32 revision = 0;

		[[nodiscard]] bool inBounds(int32 row, int32 col) const
		{
			return row >= 0 && col >= 0 && row < height && col < width;
		}

		[[nodiscard]] TileIndex index(int32 row, int32 col) const
		{
			return static_cast<TileIndex>(row * width + col);
		}

		[[nodiscard]] bool isPassable(int32 row, int32 col) const
		{
			return inBounds(row, col) && blocked[index(row, col)] == 0;
		}
	};

	inline PathMapSnapshot BuildPathMapSnapshot(const BattleWorld& world)
	{
		PathMapSnapshot snapshot;
		snapshot.width = world.map.width;
		snapshot.height = world.map.height;
		snapshot.revision = world.map.revision;
		const size_t total = static_cast<size_t>(snapshot.width * snapshot.height);
		snapshot.blocked.assign(total, 0);

		for (int32 row = 0; row < snapshot.height; ++row)
		{
			for (int32 col = 0; col < snapshot.width; ++col)
			{
				const TileIndex idx = snapshot.index(row, col);
				const bool passable = world.map.isPassable(row, col);
				const bool reserved = world.map.hasBarrierReservation(row, col);
				snapshot.blocked[idx] = (passable && !reserved) ? 0u : 1u;
			}
		}

		return snapshot;
	}

	inline Point PathWorldToBattleCell(const BattleWorld& world, const Vec2& position)
	{
		return QuarterWorldToBattleCell(position, world.mapWidth, world.mapHeight);
	}

	inline Vec2 PathBattleCellToWorldPosition(const Point& cell)
	{
		return QuarterBattleCellCenter(cell.x, cell.y);
	}

	inline bool IsPathCellPassable(const BattleWorld& world, const Point& cell)
	{
		return world.map.inBounds(cell.y, cell.x)
			&& world.map.isPassable(cell.y, cell.x)
			&& !world.map.hasBarrierReservation(cell.y, cell.x);
	}

	inline bool IsPathCellPassable(const PathMapSnapshot& snapshot, const Point& cell)
	{
		return PathfindingCore::IsPassable(
			PathfindingCore::GridView{ snapshot.width, snapshot.height, snapshot.blocked.data() },
			PathfindingCore::Cell{ cell.x, cell.y });
	}

	// Pathfinding Coreのセルを既存のSiv3D座標へ変換する。
	inline Point ToPathfindingSystemPoint(const PathfindingCore::Cell& cell)
	{
		return Point{ cell.x, cell.y };
	}

	inline Optional<Point> FindNearestPassablePathCell(const PathMapSnapshot& snapshot, const Point& center)
	{
		PathfindingCore::Cell result;
		const bool found = PathfindingCore::TryFindNearestPassableCell(
			PathfindingCore::GridView{ snapshot.width, snapshot.height, snapshot.blocked.data() },
			PathfindingCore::Cell{ center.x, center.y },
			result);
		return found ? Optional<Point>{ ToPathfindingSystemPoint(result) } : none;
	}

	inline Array<Point> BuildPathCellsByAStar(const PathMapSnapshot& snapshot, const Point& start, const Point& goal)
	{
		Array<Point> empty;
		const PathfindingCore::GridView grid{ snapshot.width, snapshot.height, snapshot.blocked.data() };
		uint32 coreCellCount = 0;
		if (!PathfindingCore::TryGetCellCount(grid, coreCellCount))
		{
			return empty;
		}

		Array<PathfindingCore::Cell> corePath(coreCellCount);
		uint32 corePathCount = 0;
		const PathfindingCore::PathSearchStatus status = PathfindingCore::FindPathByAStar(
			grid,
			PathfindingCore::Cell{ start.x, start.y },
			PathfindingCore::Cell{ goal.x, goal.y },
			corePath.data(),
			coreCellCount,
			corePathCount);
		if (status != PathfindingCore::PathSearchStatus::Found)
		{
			return empty;
		}

		Array<Point> path;
		path.reserve(corePathCount);
		for (uint32 i = 0; i < corePathCount; ++i)
		{
			path << ToPathfindingSystemPoint(corePath[i]);
		}

		return path;
	}

	inline PathResult BuildPathResult(const BattleWorld& world, const PathMapSnapshot& snapshot, const PathRequest& request)
	{
		PathResult result;
		result.unit = request.unit;
		result.destination = request.destination;
		result.mapRevision = request.mapRevision;

		if (!IsValidUnit(world, request.unit))
		{
			return result;
		}

		Point startCell = request.startCell;
		if (!snapshot.inBounds(startCell.y, startCell.x))
		{
			startCell = PathWorldToBattleCell(world, world.units.position[request.unit]);
		}

		Point requestedGoalCell = request.goalCell;
		if (!snapshot.inBounds(requestedGoalCell.y, requestedGoalCell.x))
		{
			requestedGoalCell = PathWorldToBattleCell(world, request.destination);
		}

		Optional<Point> resolvedGoalCell = requestedGoalCell;
		if (!IsPathCellPassable(snapshot, requestedGoalCell))
		{
			resolvedGoalCell = FindNearestPassablePathCell(snapshot, requestedGoalCell);
		}

		if (!resolvedGoalCell)
		{
			return result;
		}

		const Array<Point> cells = BuildPathCellsByAStar(snapshot, startCell, *resolvedGoalCell);
		if (cells.isEmpty())
		{
			return result;
		}

		result.waypoints.clear();
		result.waypoints.reserve(cells.size());
		for (const Point& cell : cells)
		{
			result.waypoints << PathBattleCellToWorldPosition(cell);
		}

		result.destination = PathBattleCellToWorldPosition(*resolvedGoalCell);
		result.success = true;
		return result;
	}

	inline void EnqueuePathRequest(BattleWorld& world, UnitId unit, const Vec2& destination)
	{
		if (!IsValidUnit(world, unit) || unit >= world.pathing.requestPending.size())
		{
			return;
		}

		if (world.pathing.requestPending[unit])
		{
			return;
		}

		if (unit < world.pathing.repathCooldownSec.size() && world.pathing.repathCooldownSec[unit] > 0.0)
		{
			return;
		}

		world.pathing.requests.remove_if([unit](const PathRequest& request)
		{
			return request.unit == unit;
		});

		world.pathing.requests << PathRequest{
			unit,
			destination,
			PathWorldToBattleCell(world, world.units.position[unit]),
			PathWorldToBattleCell(world, destination),
			world.map.revision
		};
		world.pathing.requestPending[unit] = true;
		world.pathing.destination[unit] = destination;
		world.pathing.repathCooldownSec[unit] = 0.12;
	}

	inline void ApplyPathResultToUnit(BattleWorld& world, const PathResult& result)
	{
		const UnitId unit = result.unit;
		if (!IsValidUnit(world, unit) || unit >= world.pathing.waypoints.size())
		{
			return;
		}

		if (result.mapRevision != world.map.revision)
		{
			world.pathing.requestPending[unit] = false;
			world.pathing.repathCooldownSec[unit] = 0.0;
			return;
		}

		world.pathing.requestPending[unit] = false;
		world.pathing.destination[unit] = result.destination;

		if (!result.success || result.waypoints.isEmpty())
		{
			ClearUnitPath(world, unit);
			world.pathing.repathCooldownSec[unit] = 0.35;
			return;
		}

		world.pathing.waypoints[unit] = result.waypoints;
		world.pathing.waypointIndex[unit] = 0;
		world.pathing.pathMapRevision[unit] = result.mapRevision;
		world.pathing.hasPath[unit] = true;
		SetUnitTargetPosition(world, unit, result.destination);
		world.pathing.repathCooldownSec[unit] = 0.0;
	}

	inline void UpdatePathfinding(BattleWorld& world, const DefinitionStores&, double dt)
	{
		for (const UnitId unit : GetLiveBattleWorldUnits(world))
		{
			if (unit < world.pathing.repathCooldownSec.size())
			{
				world.pathing.repathCooldownSec[unit] = Max(0.0, world.pathing.repathCooldownSec[unit] - dt);
			}
		}

		if (world.pathing.requests.isEmpty())
		{
			return;
		}

		const PathMapSnapshot snapshot = BuildPathMapSnapshot(world);
		const int32 requestBudget = Max(1, world.pathing.maxRequestsPerFrame);
		const int32 requestCount = static_cast<int32>(world.pathing.requests.size());
		const int32 processCount = Min(requestBudget, requestCount);

		world.pathing.results.clear();
		world.pathing.results.reserve(processCount);
		for (int32 i = 0; i < processCount; ++i)
		{
			world.pathing.results << BuildPathResult(world, snapshot, world.pathing.requests[i]);
		}

		if (processCount > 0)
		{
			world.pathing.requests.erase(world.pathing.requests.begin(), world.pathing.requests.begin() + processCount);
		}

		for (const PathResult& result : world.pathing.results)
		{
			ApplyPathResultToUnit(world, result);
		}
		world.pathing.results.clear();
	}
}
