#pragma once

#include <algorithm>
#include <cmath>

#include "BattleSessionCollision.h"

namespace BattleSessionInternal
{
	struct NavigationGridCell
	{
		int32 x = 0;
		int32 y = 0;
	};

	struct NavigationGrid
	{
		RectF bounds{ 0, 0, 0, 0 };
		double cellSize = 24.0;
		int32 columns = 1;
		int32 rows = 1;
		Array<char> blocked;
	};

	inline void ClearNavigationPath(UnitState& unit)
	{
		unit.pathPoints.clear();
		unit.pathIndex = 0;
		unit.pathDestination = unit.position;
		unit.pathDirty = false;
	}

	inline void InvalidateNavigationPath(UnitState& unit)
	{
		unit.pathPoints.clear();
		unit.pathIndex = 0;
		unit.pathDestination = unit.position;
		unit.pathDirty = true;
	}

	[[nodiscard]] inline int32 GetNavigationGridIndex(const NavigationGrid& grid, const int32 x, const int32 y)
	{
		return (y * grid.columns) + x;
	}

	[[nodiscard]] inline NavigationGridCell GetNavigationGridCell(const NavigationGrid& grid, const int32 index)
	{
		return NavigationGridCell{ (index % grid.columns), (index / grid.columns) };
	}

	[[nodiscard]] inline NavigationGridCell MakeNavigationGridCell(const NavigationGrid& grid, const Vec2& position)
	{
		return {
			Clamp(static_cast<int32>((position.x - grid.bounds.leftX()) / grid.cellSize), 0, grid.columns - 1),
			Clamp(static_cast<int32>((position.y - grid.bounds.topY()) / grid.cellSize), 0, grid.rows - 1)
		};
	}

	[[nodiscard]] inline Vec2 GetNavigationCellCenter(const NavigationGrid& grid, const NavigationGridCell& cell)
	{
		return {
			Min(grid.bounds.leftX() + ((cell.x + 0.5) * grid.cellSize), grid.bounds.rightX() - (grid.cellSize * 0.5)),
			Min(grid.bounds.topY() + ((cell.y + 0.5) * grid.cellSize), grid.bounds.bottomY() - (grid.cellSize * 0.5))
		};
	}

	[[nodiscard]] inline NavigationGrid MakeNavigationGrid(const RectF& bounds, const double cellSize, const int32 columns, const int32 rows, const Array<char>& blocked)
	{
		NavigationGrid grid;
		grid.bounds = bounds;
		grid.cellSize = cellSize;
		grid.columns = columns;
		grid.rows = rows;
		grid.blocked = blocked;
		return grid;
	}

	[[nodiscard]] inline bool IsNavigationCellBlocked(const NavigationGrid& grid, const int32 index)
	{
		return ((index < 0) || (index >= grid.blocked.size()) || (grid.blocked[index] != 0));
	}

	inline void SetNavigationCellBlocked(NavigationGrid& grid, const int32 index, const bool blocked)
	{
		if ((0 <= index) && (index < grid.blocked.size()))
		{
			grid.blocked[index] = blocked ? 1 : 0;
		}
	}

	[[nodiscard]] inline int32 FindClosestOpenNavigationCell(const NavigationGrid& grid, const int32 desiredIndex)
	{
		if (!IsNavigationCellBlocked(grid, desiredIndex))
		{
			return desiredIndex;
		}

		const auto desiredCell = GetNavigationGridCell(grid, desiredIndex);
		int32 bestIndex = -1;
		double bestScore = 1e18;

		for (int32 index = 0; index < grid.blocked.size(); ++index)
		{
			if (IsNavigationCellBlocked(grid, index))
			{
				continue;
			}

			const auto candidate = GetNavigationGridCell(grid, index);
			const double dx = static_cast<double>(candidate.x - desiredCell.x);
			const double dy = static_cast<double>(candidate.y - desiredCell.y);
			const double score = ((dx * dx) + (dy * dy));
			if (score < bestScore)
			{
				bestScore = score;
				bestIndex = index;
			}
		}

		return bestIndex;
	}

	[[nodiscard]] inline double EstimateNavigationCost(const NavigationGrid& grid, const int32 fromIndex, const int32 toIndex)
	{
		const auto from = GetNavigationGridCell(grid, fromIndex);
		const auto to = GetNavigationGridCell(grid, toIndex);
		return (std::abs(to.x - from.x) + std::abs(to.y - from.y));
	}

	[[nodiscard]] inline Array<Vec2> BuildNavigationPath(const NavigationGrid& sharedGrid, const UnitState& mover, const Vec2& destination)
	{
		const Vec2 clampedDestination = ClampToWorld(sharedGrid.bounds, destination, mover.radius);
		NavigationGrid grid = sharedGrid;

		const int32 startIndex = GetNavigationGridIndex(grid, MakeNavigationGridCell(grid, mover.position).x, MakeNavigationGridCell(grid, mover.position).y);
		const int32 desiredGoalIndex = GetNavigationGridIndex(grid, MakeNavigationGridCell(grid, clampedDestination).x, MakeNavigationGridCell(grid, clampedDestination).y);
		SetNavigationCellBlocked(grid, startIndex, false);

		const int32 goalIndex = FindClosestOpenNavigationCell(grid, desiredGoalIndex);
		if (goalIndex < 0)
		{
			return { clampedDestination };
		}

		SetNavigationCellBlocked(grid, goalIndex, false);
		if (startIndex == goalIndex)
		{
			if (!IsNavigationCellBlocked(sharedGrid, desiredGoalIndex))
			{
				return { clampedDestination };
			}

			return { GetNavigationCellCenter(grid, GetNavigationGridCell(grid, goalIndex)) };
		}

		const int32 cellCount = (grid.columns * grid.rows);
		Array<double> gScore(cellCount, 1e18);
		Array<int32> cameFrom(cellCount, -1);
		Array<char> openSetMask(cellCount, 0);
		Array<char> closedSetMask(cellCount, 0);
		Array<int32> openSet;

		gScore[startIndex] = 0.0;
		openSet << startIndex;
		openSetMask[startIndex] = 1;

		constexpr int32 neighborOffsets[4][2] =
		{
			{ 1, 0 },
			{ -1, 0 },
			{ 0, 1 },
			{ 0, -1 }
		};

		while (!openSet.isEmpty())
		{
			int32 bestOpenSetIndex = 0;
			double bestScore = 1e18;
			for (int32 i = 0; i < openSet.size(); ++i)
			{
				const int32 nodeIndex = openSet[i];
				const double score = gScore[nodeIndex] + EstimateNavigationCost(grid, nodeIndex, goalIndex);
				if (score < bestScore)
				{
					bestScore = score;
					bestOpenSetIndex = i;
				}
			}

			const int32 currentIndex = openSet[bestOpenSetIndex];
			openSet.remove_at(bestOpenSetIndex);
			openSetMask[currentIndex] = 0;
			closedSetMask[currentIndex] = 1;

			if (currentIndex == goalIndex)
			{
				Array<Vec2> reversedPath;
				for (int32 traceIndex = currentIndex; traceIndex >= 0; traceIndex = cameFrom[traceIndex])
				{
					reversedPath << GetNavigationCellCenter(grid, GetNavigationGridCell(grid, traceIndex));
					if (traceIndex == startIndex)
					{
						break;
					}
				}

				std::reverse(reversedPath.begin(), reversedPath.end());
				if (!reversedPath.isEmpty())
				{
					reversedPath.remove_at(0);
				}

				if (!IsNavigationCellBlocked(sharedGrid, desiredGoalIndex))
				{
					if (reversedPath.isEmpty() || (reversedPath.back().distanceFrom(clampedDestination) > 4.0))
					{
						reversedPath << clampedDestination;
					}
				}

				return reversedPath;
			}

			const auto currentCell = GetNavigationGridCell(grid, currentIndex);
			for (const auto& offset : neighborOffsets)
			{
				const int32 nextX = (currentCell.x + offset[0]);
				const int32 nextY = (currentCell.y + offset[1]);
				if ((nextX < 0) || (nextX >= grid.columns) || (nextY < 0) || (nextY >= grid.rows))
				{
					continue;
				}

				const int32 neighborIndex = GetNavigationGridIndex(grid, nextX, nextY);
				if (closedSetMask[neighborIndex] || IsNavigationCellBlocked(grid, neighborIndex))
				{
					continue;
				}

				const double tentativeScore = (gScore[currentIndex] + 1.0);
				if (tentativeScore >= gScore[neighborIndex])
				{
					continue;
				}

				cameFrom[neighborIndex] = currentIndex;
				gScore[neighborIndex] = tentativeScore;
				if (!openSetMask[neighborIndex])
				{
					openSet << neighborIndex;
					openSetMask[neighborIndex] = 1;
				}
			}
		}

		return { clampedDestination };
	}

	[[nodiscard]] inline Vec2 ResolveNavigationWaypoint(const NavigationGrid& sharedGrid, UnitState& mover, const Vec2& strategicDestination)
	{
		const Vec2 clampedDestination = ClampToWorld(sharedGrid.bounds, strategicDestination, mover.radius);
		if (mover.pathDirty
			|| (mover.pathPoints.isEmpty())
			|| (mover.pathIndex >= mover.pathPoints.size())
			|| (mover.pathDestination.distanceFrom(clampedDestination) > 8.0))
		{
			mover.pathPoints = BuildNavigationPath(sharedGrid, mover, clampedDestination);
			mover.pathIndex = 0;
			mover.pathDestination = clampedDestination;
			mover.pathDirty = false;
		}

		while ((mover.pathIndex < mover.pathPoints.size())
			&& (mover.position.distanceFrom(mover.pathPoints[mover.pathIndex]) <= Max(mover.radius * 0.75, 6.0)))
		{
			++mover.pathIndex;
		}

		if (mover.pathIndex < mover.pathPoints.size())
		{
			return mover.pathPoints[mover.pathIndex];
		}

		return clampedDestination;
	}
}
