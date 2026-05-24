# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "BattleUnitState.h"

namespace LT3
{
	inline const AiProfileDef* GetActiveEnemyAiProfile(const BattleWorld& world, const DefinitionStores& defs)
	{
		const AiProfileDefId profileId = world.aiRuntime.profileId;
		if (profileId != InvalidAiProfileDefId && profileId < defs.aiProfiles.size())
		{
			return &defs.aiProfiles[profileId];
		}

		if (!world.aiRuntime.profileTag.isEmpty() && defs.aiProfileByTag.contains(world.aiRuntime.profileTag))
		{
			return &defs.aiProfiles[defs.aiProfileByTag.at(world.aiRuntime.profileTag)];
		}

		return nullptr;
	}

	inline void AdvanceEnemyAiRuntimeClock(BattleWorld& world, double dt)
	{
		world.aiRuntime.phaseTimerSec += dt;
		world.aiRuntime.spawnTimerSec += dt;
		world.aiRuntime.productionTimerSec += dt;
		world.aiRuntime.attackWaveTimerSec += dt;
		world.aiRuntime.tacticalTimerSec += dt;
		world.enemySpawnTimerSec = world.aiRuntime.spawnTimerSec;
	}

	inline void ResetEnemyAiSpawnTimer(BattleWorld& world)
	{
		world.aiRuntime.spawnTimerSec = 0.0;
		world.enemySpawnTimerSec = 0.0;
	}

	inline double ResolveAiOpeningDelaySec(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(0.0, aiProfile->openingDelaySec) : 0.0;
	}

	inline bool ShouldEnemyAiIgnoreCombatWhileMoving(const AiProfileDef* aiProfile)
	{
		return aiProfile && aiProfile->contactBehavior.lowercased() == U"ignore";
	}

	inline void IssueEnemyAiMove(BattleWorld& world, UnitId unit, const Vec2& destination, const AiProfileDef* aiProfile)
	{
		IssueMove(world, unit, destination, ShouldEnemyAiIgnoreCombatWhileMoving(aiProfile));
	}

	inline Vec2 ResolveEnemySpawnOrigin(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (def.role == UnitRole::Base && def.unit_id.lowercased() == U"home")
			{
				return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
			}
		}

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (def.role == UnitRole::Base)
			{
				return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
			}
		}

		return Vec2{ 1325, 450.0 };
	}

	inline bool IsEnemySpawnCandidate(const UnitDef& def)
	{
		if (def.role == UnitRole::Base || def.role == UnitRole::Barrier)
		{
			return false;
		}
		if (def.speed <= 0.0)
		{
			return false;
		}

		return true;
	}

	inline Array<UnitDefId> CollectEnemySpawnCandidates(const DefinitionStores& defs)
	{
		Array<UnitDefId> candidates;
		for (UnitDefId unitDefId = 0; unitDefId < defs.units.size(); ++unitDefId)
		{
			if (IsEnemySpawnCandidate(defs.units[unitDefId]))
			{
				candidates << unitDefId;
			}
		}

		return candidates;
	}

	inline int32 CountAliveEnemyCombatUnits(const BattleWorld& world, const DefinitionStores& defs)
	{
		int32 count = 0;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			if (IsEnemySpawnCandidate(defs.units[world.units.defId[unit]]))
			{
				++count;
			}
		}

		return count;
	}

	inline bool IsEnemyAttackWaveCandidate(const BattleWorld& world, const DefinitionStores& defs, UnitId unit)
	{
		if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
		{
			return false;
		}
		if (world.units.defId[unit] >= defs.units.size())
		{
			return false;
		}

		const UnitDef& def = defs.units[world.units.defId[unit]];
		return IsEnemySpawnCandidate(def) && def.skill != InvalidSkillDefId && def.skill < defs.skills.size();
	}

	inline Array<UnitId> CollectEnemyAttackWaveCandidates(const BattleWorld& world, const DefinitionStores& defs)
	{
		Array<UnitId> candidates;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (IsEnemyAttackWaveCandidate(world, defs, unit))
			{
				candidates << unit;
			}
		}

		return candidates;
	}

	inline bool IsEnemyAttackWaveUnit(const BattleWorld& world, UnitId unit)
	{
		return world.aiRuntime.attackWaveUnits.contains(unit);
	}
}
