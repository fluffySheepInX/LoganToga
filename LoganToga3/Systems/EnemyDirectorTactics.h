# pragma once
# include <Siv3D.hpp>
# include "EnemyDirectorSpawn.h"

namespace LT3
{
	inline double ResolveAiTargetSearchRange(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 360.0 + aggression * 520.0;
	}

	inline double ResolveAiMoveRangeRate(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 0.72 + aggression * 0.20;
	}

	inline bool ShouldReissueEnemyAiMove(const BattleWorld& world, UnitId unit, const Vec2& destination)
	{
		if (!IsValidUnit(world, unit))
		{
			return false;
		}
		if (world.units.task[unit] != UnitTask::Moving)
		{
			return true;
		}

		return world.units.targetPosition[unit].distanceFromSq(destination) > Square(48.0);
	}

	inline bool ShouldSkipIndividualAiMoveForAttackWaveUnit(const BattleWorld& world, UnitId unit)
	{
		if (!IsEnemyAttackWaveUnit(world, unit))
		{
			return false;
		}

		return world.aiRuntime.phase == AiRuntimePhase::BuildUp
			|| world.aiRuntime.phase == AiRuntimePhase::AttackWave;
	}

	inline void UpdateEnemyAiTactics(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile, UnitId unitCount)
	{
		const double targetSearchRange = ResolveAiTargetSearchRange(aiProfile);
		const double moveRangeRate = ResolveAiMoveRangeRate(aiProfile);
		for (UnitId unit = 0; unit < unitCount; ++unit)
		{
			if (!IsValidUnit(world, unit)) continue;
			if (world.units.faction[unit] != Faction::Enemy) continue;
			const UnitDef& unitDef = defs.units[world.units.defId[unit]];
			if (unitDef.role == UnitRole::Base) continue;
			if (unitDef.skill == InvalidSkillDefId || unitDef.skill >= defs.skills.size()) continue;

			const UnitId target = FindNearestEnemy(world, unit, targetSearchRange);
			if (IsValidUnit(world, target))
			{
				const SkillDef& skill = defs.skills[unitDef.skill];
				const double distanceToTarget = world.units.position[unit].distanceFrom(world.units.position[target]);
				if (distanceToTarget > skill.range * moveRangeRate && !ShouldSkipIndividualAiMoveForAttackWaveUnit(world, unit))
				{
					if (ShouldReissueEnemyAiMove(world, unit, world.units.position[target]))
					{
						IssueEnemyAiMove(world, unit, world.units.position[target], aiProfile);
					}
				}
				if (distanceToTarget <= skill.range * 1.05 || !ShouldSkipIndividualAiMoveForAttackWaveUnit(world, unit))
				{
					SetUnitAttackTarget(world, unit, target);
				}
			}
		}
	}

	inline void UpdateEnemyDirector(BattleWorld& world, const DefinitionStores& defs, double dt)
	{
		const UnitId unitCount = static_cast<UnitId>(world.units.size());
		const AiProfileDef* aiProfile = GetActiveEnemyAiProfile(world, defs);

		AdvanceEnemyAiRuntimeClock(world, dt);
		UpdateEnemyAiSpawning(world, defs, aiProfile);
		UpdateEnemyAiAttackWave(world, defs, aiProfile);
		UpdateEnemyAiTactics(world, defs, aiProfile, unitCount);
	}
}
