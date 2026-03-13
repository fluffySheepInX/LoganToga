#include "BattleSession.h"

#include "BattleSessionInternal.h"

namespace
{
	[[nodiscard]] bool IsMovingAttackArchetype(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Spinner);
	}

	[[nodiscard]] const UnitState* FindHealTarget(const BattleState& state, const UnitState& source, const double searchRadius)
	{
		const double searchRadiusSq = (searchRadius * searchRadius);
		const UnitState* bestTarget = nullptr;
		double bestHpRate = 1.0;
		double nearestDistanceSq = Math::Inf;

		for (const auto& candidate : state.units)
		{
			if (!candidate.isAlive
				|| (candidate.owner != source.owner)
				|| (candidate.id == source.id)
				|| IsBuildingArchetype(candidate.archetype)
				|| (candidate.hp >= candidate.maxHp)
				|| (candidate.maxHp <= 0))
			{
				continue;
			}

			const Vec2 delta = (candidate.position - source.position);
			const double distanceSq = delta.lengthSq();
			if (distanceSq > searchRadiusSq)
			{
				continue;
			}

			const double hpRate = (static_cast<double>(candidate.hp) / candidate.maxHp);
			if ((hpRate < bestHpRate)
				|| ((hpRate == bestHpRate) && (distanceSq < nearestDistanceSq)))
			{
				bestHpRate = hpRate;
				nearestDistanceSq = distanceSq;
				bestTarget = &candidate;
			}
		}

		return bestTarget;
	}

	[[nodiscard]] int32 GetMovingAttackDamage(const UnitState& attacker, const UnitState& target)
	{
		if (IsBuildingArchetype(target.archetype))
		{
			return Max(1, attacker.attackPower / 3);
		}

		return attacker.attackPower;
	}

	[[nodiscard]] int32 GetAttackEffectFrames(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return 5;
		case UnitArchetype::Soldier:
			return 6;
		case UnitArchetype::Archer:
			return 9;
		case UnitArchetype::MachineGun:
			return 4;
		case UnitArchetype::Healer:
			return 7;
		case UnitArchetype::Spinner:
			return 8;
		case UnitArchetype::Turret:
			return 5;
		default:
			return 4;
		}
	}
}

const UnitState* BattleSession::tryReacquireCombatTarget(const UnitState& source, UnitOrder& order) const
{
	const double searchRadius = Max(getAggroRange(source.owner, source.archetype), source.attackRange + m_spatialQueryCellSize);
	gatherNearbyOpponentIndices(source, searchRadius, m_nearbyOpponentIndicesScratch);

	const UnitState* inRangeTarget = nullptr;
	double nearestDistanceSq = Math::Inf;

	for (const auto index : m_nearbyOpponentIndicesScratch)
	{
		const auto& candidate = m_state.units[index];
		if (!candidate.isAlive || !IsEnemy(source, candidate))
		{
			continue;
		}

		const Vec2 delta = (candidate.position - source.position);
		const double distanceSq = delta.lengthSq();
		const double attackRange = BattleSessionInternal::GetEffectiveAttackRange(source, candidate);
		if (distanceSq > (attackRange * attackRange))
		{
			continue;
		}

		if (distanceSq < nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;
			inRangeTarget = &candidate;
		}
	}

	if (inRangeTarget)
	{
		order.type = UnitOrderType::AttackTarget;
		order.targetUnitId = inRangeTarget->id;
		order.targetPoint = inRangeTarget->position;
		return inRangeTarget;
	}

	order.type = UnitOrderType::Idle;
	order.targetUnitId.reset();
	return nullptr;
}

void BattleSession::updateCombat()
{
	invalidateSpatialQueryCache();

	struct CombatEvent
	{
		int32 sourceUnitId = -1;
		Vec2 sourcePosition = Vec2::Zero();
		int32 targetId = -1;
		int32 hpDelta = 0;
		Owner owner = Owner::Player;
		UnitArchetype sourceArchetype = UnitArchetype::Soldier;
	};

	for (auto& effect : m_state.attackVisualEffects)
	{
		effect.framesRemaining = Max(effect.framesRemaining - 1, 0);
	}

	m_state.attackVisualEffects.remove_if([](const AttackVisualEffect& effect)
	{
		return (effect.framesRemaining <= 0);
	});

	Array<CombatEvent> combatEvents;
	combatEvents.reserve(m_state.units.size());

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		if (IsMovingAttackArchetype(unit.archetype))
		{
			if ((unit.attackCooldownRemaining > 0.0) || (unit.movementDistanceLastFrame <= 0.1))
			{
				continue;
			}

			gatherNearbyOpponentIndices(unit, unit.attackRange + unit.radius + m_spatialQueryCellSize, m_nearbyOpponentIndicesScratch);
			bool didHit = false;
			for (const auto index : m_nearbyOpponentIndicesScratch)
			{
				const auto& candidate = m_state.units[index];
				if (!candidate.isAlive || !IsEnemy(unit, candidate))
				{
					continue;
				}

				const Vec2 delta = (candidate.position - unit.position);
				const double attackRange = BattleSessionInternal::GetEffectiveAttackRange(unit, candidate);
				if (delta.lengthSq() > (attackRange * attackRange))
				{
					continue;
				}

				combatEvents << CombatEvent{ unit.id, unit.position, candidate.id, GetMovingAttackDamage(unit, candidate), unit.owner, unit.archetype };
				didHit = true;
			}

			if (didHit)
			{
				unit.attackCooldownRemaining = unit.attackCooldown;
			}

			continue;
		}

		if (unit.archetype == UnitArchetype::Healer)
		{
			if (unit.attackCooldownRemaining > 0.0)
			{
				continue;
			}

			const double searchRadius = Max(unit.attackRange, getAggroRange(unit.owner, unit.archetype));
			if (const auto* target = FindHealTarget(m_state, unit, searchRadius))
			{
				combatEvents << CombatEvent{ unit.id, unit.position, target->id, -unit.attackPower, unit.owner, unit.archetype };
				unit.attackCooldownRemaining = unit.attackCooldown;
			}

			continue;
		}

		if ((unit.order.type == UnitOrderType::RepairTarget) && unit.order.targetUnitId)
		{
			const auto* target = findCachedUnit(*unit.order.targetUnitId);
			const auto* building = target ? m_state.findBuildingByUnitId(*unit.order.targetUnitId) : nullptr;
			if (!(target
				&& target->isAlive
				&& (target->owner == unit.owner)
				&& (target->archetype == UnitArchetype::Turret)
				&& building
				&& building->isConstructed))
			{
				unit.order.type = UnitOrderType::Idle;
				unit.order.targetUnitId.reset();
				continue;
			}

			if (target->hp >= target->maxHp)
			{
				unit.order.type = UnitOrderType::Idle;
				unit.order.targetUnitId.reset();
				continue;
			}

			const double repairRange = (unit.radius + target->radius + 8.0);
			if (unit.position.distanceFromSq(target->position) > (repairRange * repairRange))
			{
				continue;
			}

			if (unit.attackCooldownRemaining > 0.0)
			{
				continue;
			}

			combatEvents << CombatEvent{ unit.id, unit.position, target->id, -unit.attackPower, unit.owner, unit.archetype };
			unit.attackCooldownRemaining = unit.attackCooldown;
			continue;
		}

		const UnitState* target = nullptr;

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			target = findCachedUnit(*unit.order.targetUnitId);
			if (!(target && target->isAlive && IsEnemy(unit, *target)))
			{
				target = tryReacquireCombatTarget(unit, unit.order);
			}
		}

		if (!target)
		{
			target = findNearestEnemy(unit);
		}

		if (!target)
		{
			continue;
		}

		const Vec2 delta = (target->position - unit.position);
		const double attackRange = BattleSessionInternal::GetEffectiveAttackRange(unit, *target);
		if (delta.lengthSq() > (attackRange * attackRange))
		{
			continue;
		}

		if (unit.attackCooldownRemaining > 0.0)
		{
			continue;
		}

		combatEvents << CombatEvent{ unit.id, unit.position, target->id, unit.attackPower, unit.owner, unit.archetype };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : combatEvents)
	{
		if (auto* target = findCachedUnit(event.targetId))
		{
			const int32 effectFrames = GetAttackEffectFrames(event.sourceArchetype);
			m_state.attackVisualEffects << AttackVisualEffect{
				.sourceUnitId = event.sourceUnitId,
				.start = event.sourcePosition,
				.end = target->position,
				.owner = event.owner,
				.sourceArchetype = event.sourceArchetype,
				.framesRemaining = effectFrames,
				.totalFrames = effectFrames,
			};

			target->hp = Clamp(target->hp - event.hpDelta, 0, target->maxHp);
			if ((event.hpDelta > 0) && (target->hp <= 0))
			{
				target->hp = 0;
				target->isAlive = false;
			}
		}
	}
}
