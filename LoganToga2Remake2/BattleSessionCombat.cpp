#include "BattleSession.h"

#include "BattleSessionInternal.h"

namespace
{
	constexpr double KatyushaSplashRadius = 54.0;
	constexpr double KatyushaClusterScoreWeight = 1000.0;
	constexpr double KatyushaBacklineBonus = 120.0;
	constexpr double KatyushaBuildingBonus = 40.0;
	constexpr double KatyushaDistancePenalty = 0.002;
	constexpr double KatyushaBuildingDamageMultiplier = 0.6;
	constexpr double GoliathExplosionRadius = 56.0;
	constexpr int32 GoliathExplosionDamage = 95;
	constexpr double GoliathBuildingDamageMultiplier = 1.68;

	[[nodiscard]] bool IsMovingAttackArchetype(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Spinner);
	}

	[[nodiscard]] bool IsBacklinePriorityArchetype(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Archer)
			|| (archetype == UnitArchetype::Sniper)
			|| (archetype == UnitArchetype::Katyusha)
			|| (archetype == UnitArchetype::MachineGun)
			|| (archetype == UnitArchetype::Healer);
	}

	[[nodiscard]] const UnitState* FindHealTarget(const BattleState& state, const UnitState& source, const Array<size_t>& candidateIndices, const double searchRadius)
	{
		const double searchRadiusSq = (searchRadius * searchRadius);
		const UnitState* bestTarget = nullptr;
		double bestHpRate = 1.0;
		double nearestDistanceSq = Math::Inf;

		for (const auto candidateIndex : candidateIndices)
		{
			const auto& candidate = state.units[candidateIndex];
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

	[[nodiscard]] int32 GetKatyushaDamage(const UnitState& attacker, const UnitState& target)
	{
		if (IsBuildingArchetype(target.archetype))
		{
			return Max(1, static_cast<int32>(std::ceil(attacker.attackPower * KatyushaBuildingDamageMultiplier)));
		}

		return attacker.attackPower;
	}

	[[nodiscard]] const UnitState* FindBestKatyushaTarget(const BattleState& state, const UnitState& source, const Array<size_t>& candidateIndices)
	{
		const UnitState* bestTarget = nullptr;
		double bestScore = -Math::Inf;

		for (const auto candidateIndex : candidateIndices)
		{
			const auto& candidate = state.units[candidateIndex];
			if (!candidate.isAlive || !IsEnemy(source, candidate))
			{
				continue;
			}

			const double distanceSq = source.position.distanceFromSq(candidate.position);
			const double attackRange = BattleSessionInternal::GetEffectiveAttackRange(source, candidate);
			if (distanceSq > (attackRange * attackRange))
			{
				continue;
			}

			int32 affectedCount = 0;
			for (const auto splashIndex : candidateIndices)
			{
				const auto& splashCandidate = state.units[splashIndex];
				if (!splashCandidate.isAlive || !IsEnemy(source, splashCandidate))
				{
					continue;
				}

				const double splashRadius = KatyushaSplashRadius + (splashCandidate.radius * 0.35);
				if (candidate.position.distanceFromSq(splashCandidate.position) <= (splashRadius * splashRadius))
				{
					++affectedCount;
				}
			}

			double score = (affectedCount * KatyushaClusterScoreWeight) - (distanceSq * KatyushaDistancePenalty);
			if (IsBacklinePriorityArchetype(candidate.archetype))
			{
				score += KatyushaBacklineBonus;
			}
			if (IsBuildingArchetype(candidate.archetype))
			{
				score += KatyushaBuildingBonus;
			}

			if (score > bestScore)
			{
				bestScore = score;
				bestTarget = &candidate;
			}
		}

		return bestTarget;
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
		case UnitArchetype::Katyusha:
			return 18;
		case UnitArchetype::Sniper:
			return 11;
		case UnitArchetype::MachineGun:
			return 4;
		case UnitArchetype::Goliath:
			return 14;
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

void BattleSession::applyUnitHpDelta(UnitState& target, const int32 hpDelta)
{
	if (!target.isAlive)
	{
		return;
	}

	target.hp = Clamp(target.hp - hpDelta, 0, target.maxHp);
	if ((hpDelta > 0) && (target.hp <= 0))
	{
		if (target.archetype == UnitArchetype::Goliath)
		{
			triggerGoliathExplosion(target);
			return;
		}

		target.hp = 0;
		target.isAlive = false;
	}
}

void BattleSession::triggerGoliathExplosion(UnitState& unit)
{
	if (!unit.isAlive)
	{
		return;
	}

	const Vec2 explosionCenter = unit.position;
	const int32 effectFrames = GetAttackEffectFrames(UnitArchetype::Goliath);
	unit.hp = 0;
	unit.isAlive = false;
	unit.isSelected = false;
	unit.isDetonating = false;
	unit.detonationFramesRemaining = 0;
	unit.order.type = UnitOrderType::Idle;
	unit.order.targetUnitId.reset();
	unit.order.targetPoint = explosionCenter;
	unit.moveTarget = explosionCenter;
	BattleSessionInternal::ClearNavigationPath(unit);

	m_state.attackVisualEffects << AttackVisualEffect{
		.sourceUnitId = unit.id,
		.start = explosionCenter,
		.end = explosionCenter,
		.owner = unit.owner,
		.sourceArchetype = UnitArchetype::Goliath,
		.framesRemaining = effectFrames,
		.totalFrames = effectFrames,
		.areaRadius = GoliathExplosionRadius,
	};

	const Owner targetOwner = (unit.owner == Owner::Enemy) ? Owner::Player : Owner::Enemy;
	gatherNearbyUnitIndices(targetOwner, explosionCenter, GoliathExplosionRadius + m_spatialQueryCellSize, m_nearbyUnitIndicesScratch);
	for (const auto targetIndex : m_nearbyUnitIndicesScratch)
	{
		auto& target = m_state.units[targetIndex];
		if (!target.isAlive || (target.owner == unit.owner) || (target.id == unit.id))
		{
			continue;
		}

		const double splashRadius = GoliathExplosionRadius + (target.radius * 0.35);
		if (target.position.distanceFromSq(explosionCenter) > (splashRadius * splashRadius))
		{
			continue;
		}

		const int32 damage = IsBuildingArchetype(target.archetype)
			? Max(1, static_cast<int32>(std::round(GoliathExplosionDamage * GoliathBuildingDamageMultiplier)))
			: GoliathExplosionDamage;
		applyUnitHpDelta(target, damage);
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

			combatEvents << CombatEvent{ unit.id, unit.position, target->id, target->position, -unit.attackPower, 0.0, unit.owner, unit.archetype };
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
