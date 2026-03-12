#pragma once

#include "BattleSession.h"

namespace BattleSessionInternal
{
	[[nodiscard]] inline bool UsesContactAttackRange(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Worker)
			|| (archetype == UnitArchetype::Soldier)
			|| (archetype == UnitArchetype::Spinner);
	}

	[[nodiscard]] inline double GetEffectiveAttackRange(const UnitState& attacker, const UnitState& target)
	{
		double range = attacker.attackRange;
		if (UsesContactAttackRange(attacker.archetype))
		{
			range += target.radius;
		}

		return range;
	}
}
