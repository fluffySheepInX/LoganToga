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

	[[nodiscard]] inline Array<Vec2> MakeFormationOffsets(const Array<int32>& unitIds, const BattleState& state, const Vec2& destination, const FormationType formation)
	{
		Array<Vec2> offsets;
		offsets.reserve(unitIds.size());

		if (unitIds.isEmpty())
		{
			return offsets;
		}

		Vec2 center = Vec2::Zero();
		int32 count = 0;
		for (const auto unitId : unitIds)
		{
			if (const auto* unit = state.findUnit(unitId))
			{
				center += unit->position;
				++count;
			}
		}

		if (count <= 0)
		{
			return offsets;
		}

		center /= count;
		Vec2 forward = (destination - center);
		if (forward.lengthSq() < 1.0)
		{
			forward = Vec2{ 1.0, 0.0 };
		}
		else
		{
			forward = forward.normalized();
		}

		const Vec2 right{ -forward.y, forward.x };
		const double spacing = 28.0;

		for (int32 i = 0; i < unitIds.size(); ++i)
		{
			if (formation == FormationType::Column)
			{
				offsets << (forward * (-spacing * i));
			}
			else
			{
				const double sideOffset = (i - ((unitIds.size() - 1) * 0.5)) * spacing;
				offsets << (right * sideOffset);
			}
		}

		return offsets;
	}
}
