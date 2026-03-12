#pragma once

#include "BattleRenderer.h"

namespace BattleRendererWorldInternal
{
	[[nodiscard]] inline bool IsMeleeAttackArchetype(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Worker)
			|| (archetype == UnitArchetype::Soldier);
	}

	[[nodiscard]] inline double GetAttackAnimationProgress(const AttackVisualEffect& effect)
	{
		if (effect.totalFrames <= 0)
		{
			return 0.0;
		}

		return Clamp(1.0 - (static_cast<double>(effect.framesRemaining) / effect.totalFrames), 0.0, 1.0);
	}

	[[nodiscard]] inline double GetThrustAmount(const double progress)
	{
		return (progress <= 0.5) ? (progress * 2.0) : ((1.0 - progress) * 2.0);
	}

	[[nodiscard]] inline Vec2 GetMeleeAttackOffset(const UnitArchetype archetype, const Vec2& attackVector, const double progress)
	{
		if (attackVector.lengthSq() <= 0.001)
		{
			return Vec2::Zero();
		}

		const Vec2 direction = attackVector.normalized();
		const double thrust = GetThrustAmount(progress);
		const double offsetDistance = (archetype == UnitArchetype::Worker) ? 5.0 : 7.0;
		return (direction * offsetDistance * thrust);
	}
}
