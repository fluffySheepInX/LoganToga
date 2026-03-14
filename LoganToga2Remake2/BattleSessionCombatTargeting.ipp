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
