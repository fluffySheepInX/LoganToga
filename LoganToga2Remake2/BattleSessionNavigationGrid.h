#pragma once

#include "BattleSessionNavigationTypes.h"

namespace BattleSessionInternal
{
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
}
