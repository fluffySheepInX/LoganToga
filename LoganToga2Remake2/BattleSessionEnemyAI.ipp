namespace
{
	enum class EnemyAiObjectiveType
	{
		Rally,
		CaptureResource,
		Defend,
		Assault
	};

	struct EnemyAiDecision
	{
		EnemyAiObjectiveType objective = EnemyAiObjectiveType::Rally;
		Vec2 strategicDestination = Vec2::Zero();
		Optional<int32> targetUnitId;
	};

	struct EnemyAiStrategicTarget
	{
		double score = -Math::Inf;
		Vec2 position = Vec2::Zero();
		Optional<int32> unitId;
	};

	[[nodiscard]] bool IsEnemyCombatUnit(const UnitState& unit)
	{
		return unit.isAlive
			&& (unit.owner == Owner::Enemy)
			&& unit.canMove
			&& (unit.archetype != UnitArchetype::Worker);
	}

	[[nodiscard]] const UnitState* FindOwnerBuilding(const BattleState& state, const Owner owner, const UnitArchetype archetype)
	{
		for (const auto& unit : state.units)
		{
			if (unit.isAlive && (unit.owner == owner) && (unit.archetype == archetype))
			{
				return &unit;
			}
		}

		return nullptr;
	}

	[[nodiscard]] bool IsBaseDefenseTurret(const UnitState& unit, const UnitState& base, const double lockRadius)
	{
		return unit.isAlive
			&& (unit.owner == base.owner)
			&& (unit.archetype == UnitArchetype::Turret)
			&& (unit.position.distanceFrom(base.position) <= lockRadius);
	}

	[[nodiscard]] bool CanAssaultBase(const BattleState& state, const UnitState& base, const EnemyAiConfig& config)
	{
		for (const auto& unit : state.units)
		{
			if (IsBaseDefenseTurret(unit, base, config.baseAssaultLockRadius))
			{
				return false;
			}
		}

		return true;
	}

	void ConsiderStrategicTarget(EnemyAiStrategicTarget& bestTarget, const double score, const Vec2& position, const Optional<int32>& unitId = none)
	{
		if (score <= bestTarget.score)
		{
			return;
		}

		bestTarget.score = score;
		bestTarget.position = position;
		bestTarget.unitId = unitId;
	}

	[[nodiscard]] Vec2 MakeOffsetToward(const Vec2& from, const Vec2& to, const double distance)
	{
		const Vec2 direction = (to - from);
		if (direction.lengthSq() < 1.0)
		{
			return from;
		}

		return from + (direction.normalized() * distance);
	}
}

void BattleSession::updateEnemyAI(const double deltaTime)
{
	m_state.enemyAiDecisionTimer += deltaTime;
	if (m_state.enemyAiDecisionTimer < m_config.enemyAI.decisionInterval)
	{
		return;
	}

	m_state.enemyAiDecisionTimer = 0.0;

	const UnitState* enemyBase = FindOwnerBuilding(m_state, Owner::Enemy, UnitArchetype::Base);
	const UnitState* enemyBarracks = FindOwnerBuilding(m_state, Owner::Enemy, UnitArchetype::Barracks);
	const UnitState* playerBase = FindOwnerBuilding(m_state, Owner::Player, UnitArchetype::Base);
	const Vec2 enemyAnchor = enemyBarracks ? enemyBarracks->position : (enemyBase ? enemyBase->position : m_state.worldBounds.center());
	const bool canAssaultPlayerBase = playerBase && CanAssaultBase(m_state, *playerBase, m_config.enemyAI);

	const UnitState* defenseTarget = nullptr;
	double defenseTargetDistance = m_config.enemyAI.defenseRadius;
	for (const auto& candidate : m_state.units)
	{
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		double nearestAssetDistance = candidate.position.distanceFrom(enemyAnchor);
		for (const auto& building : m_state.buildings)
		{
			const auto* unit = m_state.findUnit(building.unitId);
			if (!(unit && unit->isAlive && (unit->owner == Owner::Enemy)))
			{
				continue;
			}

			nearestAssetDistance = Min(nearestAssetDistance, candidate.position.distanceFrom(unit->position));
		}

		for (const auto& resourcePoint : m_state.resourcePoints)
		{
			if (resourcePoint.owner == Owner::Enemy)
			{
				nearestAssetDistance = Min(nearestAssetDistance, candidate.position.distanceFrom(resourcePoint.position));
			}
		}

		if (nearestAssetDistance <= defenseTargetDistance)
		{
			defenseTargetDistance = nearestAssetDistance;
			defenseTarget = &candidate;
		}
	}

	const ResourcePointState* captureTarget = nullptr;
	double captureTargetDistance = Math::Inf;
	for (const auto& resourcePoint : m_state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Enemy)
		{
			continue;
		}

		const double distance = enemyAnchor.distanceFrom(resourcePoint.position);
		if (distance < captureTargetDistance)
		{
			captureTargetDistance = distance;
			captureTarget = &resourcePoint;
		}
	}

	int32 enemyCombatUnits = 0;
	for (const auto& unit : m_state.units)
	{
		if (IsEnemyCombatUnit(unit))
		{
			++enemyCombatUnits;
		}
	}

	EnemyAiStrategicTarget strategicTarget;
	for (const auto& candidate : m_state.units)
	{
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || !IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		const double distancePenalty = enemyAnchor.distanceFrom(candidate.position) * 0.25;
		double score = -Math::Inf;

		if (candidate.archetype == UnitArchetype::Turret)
		{
			score = playerBase && IsBaseDefenseTurret(candidate, *playerBase, m_config.enemyAI.baseAssaultLockRadius)
				? 960.0
				: 520.0;
		}
		else if (candidate.archetype == UnitArchetype::Barracks)
		{
			score = 760.0;
		}
		else if ((candidate.archetype == UnitArchetype::Base) && canAssaultPlayerBase)
		{
			score = 1000.0;
		}

		if (score > -Math::Inf)
		{
			const double hpRate = (candidate.maxHp > 0) ? (static_cast<double>(candidate.hp) / candidate.maxHp) : 1.0;
			ConsiderStrategicTarget(strategicTarget, score - distancePenalty + ((1.0 - hpRate) * 120.0), candidate.position, Optional<int32>{ candidate.id });
		}
	}

	if (captureTarget)
	{
		const double captureScore = 620.0 - (enemyAnchor.distanceFrom(captureTarget->position) * 0.20);
		ConsiderStrategicTarget(strategicTarget, captureScore, captureTarget->position);
	}

	const bool shouldAssault = (enemyCombatUnits >= m_config.enemyAI.assaultUnitThreshold)
		&& (strategicTarget.score > -Math::Inf);
	const Vec2 rallyPoint = shouldAssault
		? MakeOffsetToward(enemyAnchor, strategicTarget.position, m_config.enemyAI.rallyDistance)
		: enemyAnchor;

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.owner != Owner::Enemy) || !unit.canMove)
		{
			continue;
		}

		EnemyAiDecision decision;
		if (defenseTarget)
		{
			decision.objective = EnemyAiObjectiveType::Defend;
			decision.strategicDestination = defenseTarget->position;
			decision.targetUnitId = defenseTarget->id;
		}
		else if ((unit.archetype == UnitArchetype::Worker) && captureTarget)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = captureTarget->position;
		}
		else if (shouldAssault)
		{
			decision.objective = EnemyAiObjectiveType::Assault;
			decision.strategicDestination = strategicTarget.position;
			if (strategicTarget.unitId)
			{
				decision.targetUnitId = strategicTarget.unitId;
			}
		}
		else if (captureTarget)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = captureTarget->position;
		}
		else
		{
			decision.objective = EnemyAiObjectiveType::Rally;
			decision.strategicDestination = rallyPoint;
		}

		if (!decision.targetUnitId)
		{
			if (const UnitState* nearbyTarget = findNearestEnemy(unit))
			{
				if (!((nearbyTarget->archetype == UnitArchetype::Base) && !canAssaultPlayerBase))
				{
					decision.targetUnitId = nearbyTarget->id;
					decision.strategicDestination = nearbyTarget->position;
				}
			}
		}

		if (decision.targetUnitId)
		{
			unit.order.type = UnitOrderType::AttackTarget;
			unit.order.targetUnitId = decision.targetUnitId;
			unit.order.targetPoint = decision.strategicDestination;
		}
		else
		{
			unit.order.type = UnitOrderType::Move;
			unit.order.targetUnitId.reset();
			unit.order.targetPoint = decision.strategicDestination;
		}

		unit.moveTarget = unit.order.targetPoint;
	}
}
