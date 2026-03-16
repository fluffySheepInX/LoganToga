#pragma once

#include "BattleSessionNavigationPathfinding.h"

namespace BattleSessionInternal
{
	inline void ClearNavigationPath(UnitState& unit)
	{
		unit.pathPoints.clear();
		unit.pathIndex = 0;
		unit.pathDestination = unit.position;
		unit.pathDirty = false;
		unit.pathStuckFrames = 0;
	}

	inline void InvalidateNavigationPath(UnitState& unit)
	{
		unit.pathPoints.clear();
		unit.pathIndex = 0;
		unit.pathDestination = unit.position;
		unit.pathDirty = true;
		unit.pathStuckFrames = 0;
	}

	[[nodiscard]] inline Vec2 ResolveNavigationWaypoint(const NavigationGrid& sharedGrid, UnitState& mover, const Vec2& strategicDestination)
	{
		const Vec2 clampedDestination = ClampToWorld(sharedGrid.bounds, strategicDestination, mover.radius);
		if (mover.pathDirty
			|| (mover.pathPoints.isEmpty())
			|| (mover.pathIndex >= mover.pathPoints.size())
			|| (mover.pathDestination.distanceFrom(clampedDestination) > 16.0))
		{
			mover.pathPoints = BuildNavigationPath(sharedGrid, mover, clampedDestination);
			mover.pathIndex = 0;
			mover.pathDestination = clampedDestination;
			mover.pathDirty = false;
			mover.pathStuckFrames = 0;
		}

		while ((mover.pathIndex < mover.pathPoints.size())
			&& (mover.position.distanceFrom(mover.pathPoints[mover.pathIndex]) <= Max(mover.radius * 0.5, 4.0)))
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
