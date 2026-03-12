#pragma once

#include "BattleSession.h"

namespace BattleSessionInternal
{
	[[nodiscard]] inline bool IntersectsObstacle(const Vec2& position, const double radius, const ObstacleConfig& obstacle)
	{
		if (!obstacle.blocksMovement)
		{
			return false;
		}

		const double closestX = Clamp(position.x, obstacle.rect.leftX(), obstacle.rect.rightX());
		const double closestY = Clamp(position.y, obstacle.rect.topY(), obstacle.rect.bottomY());
		const double dx = (position.x - closestX);
		const double dy = (position.y - closestY);
		return ((dx * dx) + (dy * dy)) < (radius * radius);
	}

	[[nodiscard]] inline bool IsBlockedByObstacle(const Vec2& position, const double radius, const Array<ObstacleConfig>& obstacles)
	{
		for (const auto& obstacle : obstacles)
		{
			if (IntersectsObstacle(position, radius, obstacle))
			{
				return true;
			}
		}

		return false;
	}

	[[nodiscard]] inline Vec2 ResolveObstacleMove(const Vec2& currentPosition, const Vec2& proposedPosition, const double radius, const Array<ObstacleConfig>& obstacles)
	{
		if (!IsBlockedByObstacle(proposedPosition, radius, obstacles))
		{
			return proposedPosition;
		}

		const Vec2 slideX{ proposedPosition.x, currentPosition.y };
		if (!IsBlockedByObstacle(slideX, radius, obstacles))
		{
			return slideX;
		}

		const Vec2 slideY{ currentPosition.x, proposedPosition.y };
		if (!IsBlockedByObstacle(slideY, radius, obstacles))
		{
			return slideY;
		}

		return currentPosition;
	}

	[[nodiscard]] inline bool IntersectsBuilding(const Vec2& position, const double radius, const UnitState& building)
	{
		return (position.distanceFrom(building.position) < (radius + building.radius + 2.0));
	}
}
