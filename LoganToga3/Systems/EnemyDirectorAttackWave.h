# pragma once
# include <Siv3D.hpp>
# include "EnemyDirectorCommon.h"
# include "TargetingSystem.h"

namespace LT3
{
	struct EnemyAiAttackWaveTarget
	{
		Vec2 position{ 0.0, 0.0 };
		UnitId unit = InvalidUnitId;
	};

	inline EnemyAiAttackWaveTarget MakeEnemyAiAttackWaveTarget(const BattleWorld& world, UnitId unit)
	{
		return EnemyAiAttackWaveTarget{ world.units.position[unit], unit };
	}

	inline Optional<EnemyAiAttackWaveTarget> ResolveEnemyAiPlayerBaseAttackWaveTarget(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Player)
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
				return MakeEnemyAiAttackWaveTarget(world, unit);
			}
		}

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Player)
			{
				continue;
			}
			if (world.units.defId[unit] < defs.units.size() && defs.units[world.units.defId[unit]].role == UnitRole::Base)
			{
				return MakeEnemyAiAttackWaveTarget(world, unit);
			}
		}

		return none;
	}

	inline Optional<Vec2> ResolveEnemyAiResourceTarget(const BattleWorld& world)
	{
		Optional<Vec2> neutralTarget;
		for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
		{
			if (node >= world.resourceNodes.owner.size() || node >= world.resourceNodes.amount.size())
			{
				continue;
			}
			if (world.resourceNodes.amount[node] <= 0)
			{
				continue;
			}
			if (world.resourceNodes.owner[node] == Faction::Player)
			{
				return world.resourceNodes.position[node];
			}
			if (!neutralTarget && world.resourceNodes.owner[node] == Faction::Neutral)
			{
				neutralTarget = world.resourceNodes.position[node];
			}
		}

		return neutralTarget;
	}

	inline Optional<EnemyAiAttackWaveTarget> ResolveEnemyAiPlayerUnitAttackWaveTarget(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Player)
			{
				continue;
			}
			if (world.units.defId[unit] < defs.units.size() && defs.units[world.units.defId[unit]].role != UnitRole::Base)
			{
				return MakeEnemyAiAttackWaveTarget(world, unit);
			}
		}

		return none;
	}

	inline Optional<EnemyAiAttackWaveTarget> ResolveEnemyAiAttackWaveTargetByPriority(const BattleWorld& world, const DefinitionStores& defs, const String& priority)
	{
		const String key = priority.lowercased();
		if (key == U"base")
		{
			return ResolveEnemyAiPlayerBaseAttackWaveTarget(world, defs);
		}
		if (key == U"resource")
		{
			if (const Optional<Vec2> target = ResolveEnemyAiResourceTarget(world))
			{
				return EnemyAiAttackWaveTarget{ *target, InvalidUnitId };
			}
			return none;
		}
		if (key == U"unit")
		{
			return ResolveEnemyAiPlayerUnitAttackWaveTarget(world, defs);
		}

		return none;
	}

	inline Optional<EnemyAiAttackWaveTarget> ResolveEnemyAiAttackWaveTarget(const BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		if (aiProfile)
		{
			for (const auto& priority : aiProfile->targetPriority)
			{
				if (const Optional<EnemyAiAttackWaveTarget> target = ResolveEnemyAiAttackWaveTargetByPriority(world, defs, priority))
				{
					return target;
				}
			}
		}

		if (const Optional<EnemyAiAttackWaveTarget> baseTarget = ResolveEnemyAiPlayerBaseAttackWaveTarget(world, defs))
		{
			return baseTarget;
		}
		if (const Optional<EnemyAiAttackWaveTarget> unitTarget = ResolveEnemyAiPlayerUnitAttackWaveTarget(world, defs))
		{
			return unitTarget;
		}

		return none;
	}

	inline int32 ResolveAiAttackGroupSize(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(1, aiProfile->attackGroupSize) : 4;
	}

	inline double ResolveAiAttackWaveIntervalSec(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(1.0, aiProfile->attackWaveIntervalSec) : 35.0;
	}

	inline double ResolveAiAttackWaveRallySec(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 5.0 - aggression * 2.0;
	}

	inline double ResolveAiAttackWaveReadyRatioThreshold(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 0.78 - aggression * 0.18;
	}

	inline Vec2 ResolveEnemyAiRallyPosition(const BattleWorld& world, const DefinitionStores& defs)
	{
		return ResolveEnemySpawnOrigin(world, defs) + Vec2{ -QuarterTileStep * 0.8, 0.0 };
	}

	inline Vec2 ResolveEnemyAiAttackMoveDestination(const BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId targetUnit, const Vec2& fallbackTargetPosition)
	{
		if (!IsValidUnit(world, attacker) || !IsValidUnit(world, targetUnit))
		{
			return fallbackTargetPosition;
		}
		if (world.units.defId[attacker] >= defs.units.size() || world.units.defId[targetUnit] >= defs.units.size())
		{
			return fallbackTargetPosition;
		}

		const UnitDef& attackerDef = defs.units[world.units.defId[attacker]];
		const UnitDef& targetDef = defs.units[world.units.defId[targetUnit]];
		if (targetDef.role != UnitRole::Base)
		{
			return fallbackTargetPosition;
		}

		const double margin = 12.0;
		double stopDistance = targetDef.radius + attackerDef.radius + margin;
		for (const SkillDefId skillId : ResolveUnitSkillIds(attackerDef, defs))
		{
			const double effectiveRange = ResolveEffectiveAttackRange(attackerDef, defs.skills[skillId]);
			stopDistance = Max(stopDistance, effectiveRange - targetDef.radius - margin);
		}

		Vec2 direction = world.units.position[attacker] - world.units.position[targetUnit];
		if (direction.lengthSq() < 1.0)
		{
			direction = fallbackTargetPosition - world.units.position[targetUnit];
		}
		if (direction.lengthSq() < 1.0)
		{
			direction = Vec2{ -1.0, 0.0 };
		}

		return world.units.position[targetUnit] + direction.normalized() * stopDistance;
	}

	inline void ResetEnemyAiAttackWave(BattleWorld& world, AiRuntimePhase nextPhase = AiRuntimePhase::Recover)
	{
		world.aiRuntime.phase = nextPhase;
		world.aiRuntime.phaseTimerSec = 0.0;
		world.aiRuntime.attackWaveTimerSec = 0.0;
		world.aiRuntime.attackWaveUnits.clear();
		world.aiRuntime.attackTargetUnit = InvalidUnitId;
		world.aiRuntime.hasRallyPosition = false;
		world.aiRuntime.hasAttackTargetPosition = false;
	}

	inline bool IsEnemyAiAttackWaveActive(const BattleWorld& world)
	{
		return world.aiRuntime.phase == AiRuntimePhase::BuildUp
			|| world.aiRuntime.phase == AiRuntimePhase::AttackWave;
	}

	inline void StartEnemyAiAttackWave(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile, const Array<UnitId>& candidates, const EnemyAiAttackWaveTarget& target)
	{
		const int32 groupSize = Min(ResolveAiAttackGroupSize(aiProfile), static_cast<int32>(candidates.size()));
		world.aiRuntime.attackWaveUnits.clear();
		for (int32 i = 0; i < groupSize; ++i)
		{
			world.aiRuntime.attackWaveUnits << candidates[i];
		}

		++world.aiRuntime.attackWaveIndex;
		world.aiRuntime.phase = AiRuntimePhase::BuildUp;
		world.aiRuntime.phaseTimerSec = 0.0;
		world.aiRuntime.attackWaveTimerSec = 0.0;
		world.aiRuntime.rallyPosition = ResolveEnemyAiRallyPosition(world, defs);
		world.aiRuntime.attackTargetPosition = target.position;
		world.aiRuntime.attackTargetUnit = target.unit;
		world.aiRuntime.hasRallyPosition = true;
		world.aiRuntime.hasAttackTargetPosition = true;
		const Vec2 facing = target.position - world.aiRuntime.rallyPosition;
		IssueFormationMove(world, defs, world.aiRuntime.attackWaveUnits, world.aiRuntime.rallyPosition, facing);
	}

	inline void RemoveInvalidEnemyAiAttackWaveUnits(BattleWorld& world, const DefinitionStores& defs)
	{
		world.aiRuntime.attackWaveUnits.remove_if([&](UnitId unit)
		{
			return !IsEnemyAttackWaveCandidate(world, defs, unit);
		});
	}

	inline double ResolveEnemyAiAttackWaveReadyRatio(const BattleWorld& world)
	{
		if (!world.aiRuntime.hasRallyPosition || world.aiRuntime.attackWaveUnits.isEmpty())
		{
			return 0.0;
		}

		int32 readyCount = 0;
		for (const UnitId unit : world.aiRuntime.attackWaveUnits)
		{
			if (!IsValidUnit(world, unit))
			{
				continue;
			}
			if (world.units.position[unit].distanceFromSq(world.aiRuntime.rallyPosition) <= Square(72.0))
			{
				++readyCount;
			}
		}

		return static_cast<double>(readyCount) / Max(1.0, static_cast<double>(world.aiRuntime.attackWaveUnits.size()));
	}

	inline void UpdateEnemyAiAttackWave(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		RemoveInvalidEnemyAiAttackWaveUnits(world, defs);
		RemoveInvalidEnemyAiNonWaveRoleUnits(world, defs);

		if (world.aiRuntime.phase == AiRuntimePhase::BuildUp)
		{
			if (world.aiRuntime.attackWaveUnits.isEmpty() || !world.aiRuntime.hasAttackTargetPosition)
			{
				ResetEnemyAiAttackWave(world);
				return;
			}

			if (world.aiRuntime.phaseTimerSec >= ResolveAiAttackWaveRallySec(aiProfile)
				|| ResolveEnemyAiAttackWaveReadyRatio(world) >= ResolveAiAttackWaveReadyRatioThreshold(aiProfile))
			{
				world.aiRuntime.phase = AiRuntimePhase::AttackWave;
				world.aiRuntime.phaseTimerSec = 0.0;
				const Vec2 facing = world.aiRuntime.attackTargetPosition - world.aiRuntime.rallyPosition;
				IssueFormationMove(world, defs, world.aiRuntime.attackWaveUnits, world.aiRuntime.attackTargetPosition, facing);
				for (const UnitId unit : world.aiRuntime.attackWaveUnits)
				{
					if (IsValidUnit(world, unit) && world.aiRuntime.attackTargetUnit != InvalidUnitId)
					{
						SetUnitAttackTarget(world, unit, world.aiRuntime.attackTargetUnit);
					}
				}
			}
			return;
		}

		if (world.aiRuntime.phase == AiRuntimePhase::AttackWave)
		{
			if (world.aiRuntime.attackWaveUnits.isEmpty())
			{
				ResetEnemyAiAttackWave(world);
				return;
			}
			if (world.aiRuntime.phaseTimerSec >= 28.0)
			{
				ResetEnemyAiAttackWave(world);
			}
			return;
		}

		if (world.elapsedSec < ResolveAiOpeningDelaySec(aiProfile))
		{
			world.aiRuntime.phase = AiRuntimePhase::Opening;
			return;
		}

		const Array<UnitId> candidates = CollectEnemyAttackWaveCandidates(world, defs);
		const int32 groupSize = ResolveAiAttackGroupSize(aiProfile);
		if (static_cast<int32>(candidates.size()) < groupSize)
		{
			world.aiRuntime.phase = AiRuntimePhase::Recover;
			world.aiRuntime.attackWaveUnits.clear();
			world.aiRuntime.hasRallyPosition = false;
			world.aiRuntime.hasAttackTargetPosition = false;
			return;
		}

		if (world.aiRuntime.attackWaveTimerSec < ResolveAiAttackWaveIntervalSec(aiProfile))
		{
			world.aiRuntime.phase = AiRuntimePhase::Recover;
			return;
		}

		if (const Optional<EnemyAiAttackWaveTarget> target = ResolveEnemyAiAttackWaveTarget(world, defs, aiProfile))
		{
			StartEnemyAiAttackWave(world, defs, aiProfile, candidates, *target);
		}
	}
}
