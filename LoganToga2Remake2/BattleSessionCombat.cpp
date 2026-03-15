#include "BattleSession.h"

#include "BattleSessionInternal.h"

#include "BattleSessionCombatTargeting.ipp"
#include "BattleSessionCombatEffects.ipp"

void BattleSession::updateCombat()
{
	invalidateSpatialQueryCache();

	struct CombatEvent
	{
		int32 sourceUnitId = -1;
		Vec2 sourcePosition = Vec2::Zero();
		int32 targetId = -1;
		Vec2 targetPosition = Vec2::Zero();
		int32 hpDelta = 0;
		double splashRadius = 0.0;
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

		if (unit.archetype == UnitArchetype::Goliath)
		{
			if (unit.isDetonating)
			{
				unit.detonationFramesRemaining = Max(unit.detonationFramesRemaining - 1, 0);
				if (unit.detonationFramesRemaining <= 0)
				{
					triggerGoliathExplosion(unit);
				}
			}

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

				combatEvents << CombatEvent{ unit.id, unit.position, candidate.id, candidate.position, GetMovingAttackDamage(unit, candidate), 0.0, unit.owner, unit.archetype };
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
			gatherNearbyUnitIndices(unit.owner, unit.position, searchRadius, m_nearbyUnitIndicesScratch);
			if (const auto* target = FindHealTarget(m_state, unit, m_nearbyUnitIndicesScratch, searchRadius))
			{
				combatEvents << CombatEvent{ unit.id, unit.position, target->id, target->position, -unit.attackPower, 0.0, unit.owner, unit.archetype };
				unit.attackCooldownRemaining = unit.attackCooldown;
			}

			continue;
		}

		if (unit.archetype == UnitArchetype::Katyusha)
		{
			if (unit.attackCooldownRemaining > 0.0)
			{
				continue;
			}

			gatherNearbyOpponentIndices(unit, unit.attackRange + KatyushaSplashRadius + m_spatialQueryCellSize, m_nearbyOpponentIndicesScratch);
			if (const auto* target = FindBestKatyushaTarget(m_state, unit, m_nearbyOpponentIndicesScratch))
			{
				combatEvents << CombatEvent{ unit.id, unit.position, target->id, target->position, unit.attackPower, KatyushaSplashRadius, unit.owner, unit.archetype };
				unit.attackCooldownRemaining = unit.attackCooldown;
			}

			continue;
		}

		if ((unit.order.type == UnitOrderType::RepairTarget) && unit.order.targetUnitId)
		{
			const auto* target = findCachedUnit(*unit.order.targetUnitId);
			const auto* building = target ? findCachedBuilding(*unit.order.targetUnitId) : nullptr;
			if (!(target
				&& target->isAlive
				&& (target->owner == unit.owner)
				&& ((target->archetype == UnitArchetype::Turret) || (target->archetype == UnitArchetype::Base))
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

			combatEvents << CombatEvent{ unit.id, unit.position, target->id, target->position, -unit.attackPower, 0.0, unit.owner, unit.archetype };
			unit.attackCooldownRemaining = unit.attackCooldown;
			continue;
		}

		if (unit.attackCooldownRemaining > 0.0)
		{
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

		combatEvents << CombatEvent{ unit.id, unit.position, target->id, target->position, unit.attackPower, 0.0, unit.owner, unit.archetype };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : combatEvents)
	{
		const int32 effectFrames = GetAttackEffectFrames(event.sourceArchetype);
		m_state.attackVisualEffects << AttackVisualEffect{
			.sourceUnitId = event.sourceUnitId,
			.start = event.sourcePosition,
			.end = event.targetPosition,
			.owner = event.owner,
			.sourceArchetype = event.sourceArchetype,
			.framesRemaining = effectFrames,
			.totalFrames = effectFrames,
		};

		if (event.splashRadius > 0.0)
		{
			const Owner targetOwner = (event.owner == Owner::Enemy) ? Owner::Player : Owner::Enemy;
			gatherNearbyUnitIndices(targetOwner, event.targetPosition, event.splashRadius + m_spatialQueryCellSize, m_nearbyUnitIndicesScratch);
			for (const auto targetIndex : m_nearbyUnitIndicesScratch)
			{
				auto& target = m_state.units[targetIndex];
				if (!target.isAlive || (target.owner == event.owner))
				{
					continue;
				}

				const double splashRadius = event.splashRadius + (target.radius * 0.35);
				if (target.position.distanceFromSq(event.targetPosition) > (splashRadius * splashRadius))
				{
					continue;
				}

				const int32 hpDelta = GetKatyushaDamage(UnitState{ .attackPower = event.hpDelta }, target);
				applyUnitHpDelta(target, hpDelta);
			}

			continue;
		}

		if (auto* target = findCachedUnit(event.targetId))
		{
			applyUnitHpDelta(*target, event.hpDelta);
		}
	}
}
