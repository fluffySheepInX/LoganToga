# pragma once
# include <Siv3D.hpp>
# include <queue>
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
		return snapshot.isPassable(cell.y, cell.x);
	}

	inline Optional<Point> FindNearestPassablePathCell(const PathMapSnapshot& snapshot, const Point& center)
	{
		if (IsPathCellPassable(snapshot, center))
		{
			return center;
		}

		if (!snapshot.inBounds(center.y, center.x))
		{
			return none;
		}

		const size_t total = static_cast<size_t>(snapshot.width * snapshot.height);
		Array<bool> visited(total, false);
		Array<Point> queue;
		queue << center;
		visited[snapshot.index(center.y, center.x)] = true;

		const Point offsets[4] = {
			Point{ 1, 0 },
			Point{ -1, 0 },
			Point{ 0, 1 },
			Point{ 0, -1 }
		};

		size_t head = 0;
		while (head < queue.size())
		{
			const Point cell = queue[head++];
			for (const Point& offset : offsets)
			{
				const Point next = cell + offset;
				if (!snapshot.inBounds(next.y, next.x))
				{
					continue;
				}

				const TileIndex index = snapshot.index(next.y, next.x);
				if (visited[index])
				{
					continue;
				}

				visited[index] = true;
				if (IsPathCellPassable(snapshot, next))
				{
					return next;
				}

				queue << next;
			}
		}

		return none;
	}

	inline Array<Point> BuildPathCellsByAStar(const PathMapSnapshot& snapshot, const Point& start, const Point& goal)
	{
		Array<Point> empty;
		if (!snapshot.inBounds(start.y, start.x) || !snapshot.inBounds(goal.y, goal.x))
		{
			return empty;
		}

		if (start == goal)
		{
			return Array<Point>{ start };
		}

		struct OpenEntry
		{
			int32 fScore = 0;
			TileIndex index = 0;

			bool operator<(const OpenEntry& rhs) const
			{
				return fScore > rhs.fScore;
			}
		};

		const size_t total = static_cast<size_t>(snapshot.width * snapshot.height);
		Array<int32> gScore(total, INT32_MAX);
		Array<TileIndex> cameFrom(total, static_cast<TileIndex>(UINT32_MAX));
		Array<bool> closed(total, false);

		const TileIndex startIndex = snapshot.index(start.y, start.x);
		const TileIndex goalIndex = snapshot.index(goal.y, goal.x);

		const auto heuristic = [](const Point& a, const Point& b)
		{
			return (Abs(a.x - b.x) + Abs(a.y - b.y)) * 10;
		};

		const auto indexToPoint = [&snapshot](TileIndex index)
		{
			const int32 row = static_cast<int32>(index) / snapshot.width;
			const int32 col = static_cast<int32>(index) % snapshot.width;
			return Point{ col, row };
		};

		std::priority_queue<OpenEntry> open;
		gScore[startIndex] = 0;
		open.push(OpenEntry{ heuristic(start, goal), startIndex });

		const Point offsets[4] = {
			Point{ 1, 0 },
			Point{ -1, 0 },
			Point{ 0, 1 },
			Point{ 0, -1 }
		};

		bool found = false;
		while (!open.empty())
		{
			const OpenEntry currentEntry = open.top();
			open.pop();

			const TileIndex current = currentEntry.index;
			if (closed[current])
			{
				continue;
			}

			if (current == goalIndex)
			{
				found = true;
				break;
			}

			closed[current] = true;
			const Point currentPoint = indexToPoint(current);
			for (const Point& offset : offsets)
			{
				const Point nextPoint = currentPoint + offset;
				if (!snapshot.inBounds(nextPoint.y, nextPoint.x))
				{
					continue;
				}

				if (!IsPathCellPassable(snapshot, nextPoint) && nextPoint != goal)
				{
					continue;
				}

				const TileIndex next = snapshot.index(nextPoint.y, nextPoint.x);
				if (closed[next])
				{
					continue;
				}

				const int32 nextG = gScore[current] + 10;
				if (nextG >= gScore[next])
				{
					continue;
				}

				cameFrom[next] = current;
				gScore[next] = nextG;
				const int32 nextF = nextG + heuristic(nextPoint, goal);
				open.push(OpenEntry{ nextF, next });
			}
		}

		if (!found)
		{
			return empty;
		}

		Array<Point> reversed;
		TileIndex current = goalIndex;
		while (true)
		{
			reversed << indexToPoint(current);
			if (current == startIndex)
			{
				break;
			}

			const TileIndex parent = cameFrom[current];
			if (parent == static_cast<TileIndex>(UINT32_MAX))
			{
				return empty;
			}
			current = parent;
		}

		reversed.reverse();
		return reversed;
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
		for (UnitId unit = 0; unit < world.units.size() && unit < world.pathing.repathCooldownSec.size(); ++unit)
		{
			world.pathing.repathCooldownSec[unit] = Max(0.0, world.pathing.repathCooldownSec[unit] - dt);
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
