# pragma once
# include <Siv3D.hpp>
# include "EnemyDirectorConstruction.h"

namespace LT3
{
	inline Optional<Vec2> ResolveEnemyAiReclaimTargetPosition(const BattleWorld& world)
	{
		return ResolveEnemyAiResourceTarget(world);
	}

	inline Array<UnitId> CollectEnemyNonWaveCombatUnits(const BattleWorld& world, const DefinitionStores& defs)
	{
		Array<UnitId> units;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsEnemyAttackWaveCandidate(world, defs, unit) || IsEnemyAttackWaveUnit(world, unit))
			{
				continue;
			}

			units << unit;
		}

		return units;
	}

	inline void RebuildEnemyAiNonWaveRoles(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		ClearEnemyAiNonWaveRoleAssignments(world);
		const Array<UnitId> candidates = CollectEnemyNonWaveCombatUnits(world, defs);
		if (candidates.isEmpty())
		{
			return;
		}

		const int32 total = static_cast<int32>(candidates.size());
		const int32 reclaimCount = ResolveEnemyAiReclaimTargetPosition(world) ? Min(2, total) : 0;
		const int32 guardCount = Min(Max(1, total / 3), Max(0, total - reclaimCount));
		const int32 skirmishCount = Max(0, total - reclaimCount - guardCount);

		int32 cursor = 0;
		for (int32 i = 0; i < reclaimCount && cursor < total; ++i, ++cursor)
		{
			world.aiRuntime.resourceReclaimUnits << candidates[cursor];
		}
		for (int32 i = 0; i < guardCount && cursor < total; ++i, ++cursor)
		{
			world.aiRuntime.guardUnits << candidates[cursor];
		}
		for (int32 i = 0; i < skirmishCount && cursor < total; ++i, ++cursor)
		{
			world.aiRuntime.skirmishUnits << candidates[cursor];
		}

		if (const Optional<Vec2> reclaimTarget = ResolveEnemyAiReclaimTargetPosition(world))
		{
			world.aiRuntime.resourceTargetPosition = *reclaimTarget;
			world.aiRuntime.hasResourceTargetPosition = true;
		}
		world.aiRuntime.guardAnchorPosition = ResolveEnemyAiGuardAnchor(world, defs);
		world.aiRuntime.hasGuardAnchorPosition = true;
		world.aiRuntime.skirmishAnchorPosition = ResolveEnemyAiSkirmishAnchor(world, defs);
		world.aiRuntime.hasSkirmishAnchorPosition = true;
	}

	inline void RefreshEnemyAiNonWaveRoles(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile, bool forceRebalance)
	{
		RemoveInvalidEnemyAiNonWaveRoleUnits(world, defs);

		const Array<UnitId> candidates = CollectEnemyNonWaveCombatUnits(world, defs);
		if (candidates.isEmpty())
		{
			ClearEnemyAiNonWaveRoleAssignments(world);
			world.aiRuntime.nonWaveRoleReassignmentTimerSec = 0.0;
			return;
		}

		if (forceRebalance)
		{
			ClearEnemyAiNonWaveRoleAssignments(world);
		}

		const int32 total = static_cast<int32>(candidates.size());
		const int32 reclaimCount = ResolveEnemyAiReclaimTargetPosition(world) ? Min(2, total) : 0;
		const int32 guardCount = Min(Max(1, total / 3), Max(0, total - reclaimCount));
		const int32 skirmishCount = Max(0, total - reclaimCount - guardCount);

		if (static_cast<int32>(world.aiRuntime.resourceReclaimUnits.size()) > reclaimCount)
		{
			world.aiRuntime.resourceReclaimUnits.resize(reclaimCount);
		}
		if (static_cast<int32>(world.aiRuntime.guardUnits.size()) > guardCount)
		{
			world.aiRuntime.guardUnits.resize(guardCount);
		}
		if (static_cast<int32>(world.aiRuntime.skirmishUnits.size()) > skirmishCount)
		{
			world.aiRuntime.skirmishUnits.resize(skirmishCount);
		}

		FillEnemyAiNonWaveRoleUnits(world.aiRuntime.resourceReclaimUnits, candidates, world, reclaimCount);
		FillEnemyAiNonWaveRoleUnits(world.aiRuntime.guardUnits, candidates, world, guardCount);
		FillEnemyAiNonWaveRoleUnits(world.aiRuntime.skirmishUnits, candidates, world, skirmishCount);

		if (const Optional<Vec2> reclaimTarget = ResolveEnemyAiReclaimTargetPosition(world))
		{
			world.aiRuntime.resourceTargetPosition = *reclaimTarget;
			world.aiRuntime.hasResourceTargetPosition = true;
		}
		else
		{
			world.aiRuntime.hasResourceTargetPosition = false;
		}
		world.aiRuntime.guardAnchorPosition = ResolveEnemyAiGuardAnchor(world, defs);
		world.aiRuntime.hasGuardAnchorPosition = true;
		world.aiRuntime.skirmishAnchorPosition = ResolveEnemyAiSkirmishAnchor(world, defs);
		world.aiRuntime.hasSkirmishAnchorPosition = true;
		world.aiRuntime.nonWaveRoleReassignmentTimerSec = 0.0;
	}

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

	inline Optional<Vec2> ResolveEnemyAiRoleMoveDestination(const BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile, UnitId unit)
	{
		if (IsEnemyResourceReclaimUnit(world, unit) && world.aiRuntime.hasResourceTargetPosition)
		{
			return world.aiRuntime.resourceTargetPosition;
		}
		if (IsEnemyGuardUnit(world, unit) && world.aiRuntime.hasGuardAnchorPosition)
		{
			return world.aiRuntime.guardAnchorPosition + Vec2{ Random(-48.0, 48.0), Random(-36.0, 36.0) };
		}
		if (IsEnemySkirmishUnit(world, unit) && world.aiRuntime.hasSkirmishAnchorPosition)
		{
			const Vec2 anchor = world.aiRuntime.skirmishAnchorPosition;
			const double leash = ResolveEnemyAiSkirmishLeashRadius(aiProfile);
			const Vec2 position = world.units.position[unit];
			if (position.distanceFrom(anchor) > leash)
			{
				return anchor;
			}
			return anchor + Vec2{ Random(-leash * 0.35, leash * 0.35), Random(-leash * 0.22, leash * 0.22) };
		}

		return none;
	}

	inline void UpdateEnemyAiTactics(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile, UnitId unitCount)
	{
		const bool forceRebalance = (world.aiRuntime.nonWaveRoleReassignmentTimerSec >= ResolveEnemyAiNonWaveRoleReassignmentIntervalSec(aiProfile));
		RefreshEnemyAiNonWaveRoles(world, defs, aiProfile, forceRebalance);
		const double targetSearchRange = ResolveAiTargetSearchRange(aiProfile);
		const double moveRangeRate = ResolveAiMoveRangeRate(aiProfile);
		for (UnitId unit = 0; unit < unitCount; ++unit)
		{
			if (!IsValidUnit(world, unit)) continue;
			if (world.units.faction[unit] != Faction::Enemy) continue;
			const UnitDef& unitDef = defs.units[world.units.defId[unit]];
			if (unitDef.role == UnitRole::Base) continue;
			const Array<SkillDefId> skillIds = ResolveUnitSkillIds(unitDef, defs);
			if (skillIds.isEmpty()) continue;

			const UnitId target = FindNearestEnemy(world, unit, targetSearchRange);
			if (IsValidUnit(world, target))
			{
				double maxRange = 0.0;
				for (const SkillDefId skillId : skillIds)
				{
					maxRange = Max(maxRange, ResolveEffectiveAttackRange(unitDef, defs.skills[skillId]));
				}
				const double distanceToTarget = world.units.position[unit].distanceFrom(world.units.position[target]);
				if (distanceToTarget > maxRange * moveRangeRate && !ShouldSkipIndividualAiMoveForAttackWaveUnit(world, unit))
				{
					if (const Optional<Vec2> roleMove = ResolveEnemyAiRoleMoveDestination(world, defs, aiProfile, unit))
					{
						if (ShouldReissueEnemyAiMove(world, unit, *roleMove))
						{
							IssueEnemyAiMove(world, unit, *roleMove, aiProfile);
						}
					}
					else if (ShouldReissueEnemyAiMove(world, unit, world.units.position[target]))
					{
						IssueEnemyAiMove(world, unit, world.units.position[target], aiProfile);
					}
				}
				if (distanceToTarget <= maxRange * 1.05 || !ShouldSkipIndividualAiMoveForAttackWaveUnit(world, unit))
				{
					if (IsEnemyGuardUnit(world, unit) || IsEnemySkirmishUnit(world, unit) || IsEnemyResourceReclaimUnit(world, unit))
					{
						const Optional<Vec2> roleMove = ResolveEnemyAiRoleMoveDestination(world, defs, aiProfile, unit);
						const bool nearbyThreat = roleMove ? (world.units.position[target].distanceFromSq(*roleMove) <= Square(220.0)) : true;
						if (!nearbyThreat)
						{
							continue;
						}
					}
					SetUnitAttackTarget(world, unit, target);
				}
			}
			else if (!ShouldSkipIndividualAiMoveForAttackWaveUnit(world, unit))
			{
				if (const Optional<Vec2> roleMove = ResolveEnemyAiRoleMoveDestination(world, defs, aiProfile, unit))
				{
					if (ShouldReissueEnemyAiMove(world, unit, *roleMove))
					{
						IssueEnemyAiMove(world, unit, *roleMove, aiProfile);
					}
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
		UpdateEnemyAiConstruction(world, defs, aiProfile);
		UpdateEnemyAiAttackWave(world, defs, aiProfile);
		UpdateEnemyAiTactics(world, defs, aiProfile, unitCount);
	}
}
