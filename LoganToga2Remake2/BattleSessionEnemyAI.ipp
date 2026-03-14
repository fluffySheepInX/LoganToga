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

	[[nodiscard]] bool IsBaseDefenseTurret(const UnitState& unit, const UnitState& base, const double lockRadius)
	{
		return unit.isAlive
			&& (unit.owner == base.owner)
			&& (unit.archetype == UnitArchetype::Turret)
			&& (unit.position.distanceFrom(base.position) <= lockRadius);
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
	const EnemyAiMode activeEnemyAiMode = m_state.enemyAiDebugOverrideMode
		? *m_state.enemyAiDebugOverrideMode
		: m_config.enemyAI.mode;
	m_state.enemyAiResolvedMode = activeEnemyAiMode;

	if (m_state.enemyAiAssaultCommitTimer > 0.0)
	{
		m_state.enemyAiAssaultCommitTimer = Max(0.0, m_state.enemyAiAssaultCommitTimer - deltaTime);
		if (m_state.enemyAiAssaultCommitTimer <= 0.0)
		{
			m_state.enemyAiAssaultTargetUnitId.reset();
		}
	}

	invalidateSpatialQueryCache();

	m_state.enemyAiDecisionTimer += deltaTime;
	if (m_state.enemyAiDecisionTimer < m_config.enemyAI.decisionInterval)
	{
		return;
	}

	m_state.enemyAiDecisionTimer = 0.0;

	const UnitState* enemyBase = findOwnerUnitByArchetype(Owner::Enemy, UnitArchetype::Base);
	const UnitState* enemyBarracks = findOwnerUnitByArchetype(Owner::Enemy, UnitArchetype::Barracks);
	const UnitState* playerBase = findOwnerUnitByArchetype(Owner::Player, UnitArchetype::Base);
	const Vec2 enemyAnchor = enemyBarracks ? enemyBarracks->position : (enemyBase ? enemyBase->position : m_state.worldBounds.center());
	const bool canAssaultPlayerBase = playerBase && !hasBaseDefenseTurret(*playerBase, m_config.enemyAI.baseAssaultLockRadius);

	const UnitState* defenseTarget = nullptr;
	double defenseTargetDistance = m_config.enemyAI.defenseRadius;
	for (const auto candidateIndex : getOwnerUnitIndices(Owner::Player))
	{
		const auto& candidate = m_state.units[candidateIndex];
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		double nearestAssetDistance = candidate.position.distanceFrom(enemyAnchor);
		for (const auto buildingIndex : getOwnerBuildingIndices(Owner::Enemy))
		{
			const auto& unit = m_state.units[buildingIndex];
			if (!(unit.isAlive && (unit.owner == Owner::Enemy)))
			{
				continue;
			}

			nearestAssetDistance = Min(nearestAssetDistance, candidate.position.distanceFrom(unit.position));
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
	for (const auto index : getOwnerUnitIndices(Owner::Enemy))
	{
		const auto& unit = m_state.units[index];
		if (IsEnemyCombatUnit(unit))
		{
			++enemyCombatUnits;
		}
	}

	EnemyAiStrategicTarget strategicTarget;
	for (const auto candidateIndex : getOwnerBuildingIndices(Owner::Player))
	{
		const auto& candidate = m_state.units[candidateIndex];
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

	const bool useStagingAssault = (activeEnemyAiMode == EnemyAiMode::StagingAssault);
	const bool hasStrategicTarget = (strategicTarget.score > -Math::Inf);
	const bool canStageAssault = useStagingAssault && !defenseTarget && hasStrategicTarget;
	const Vec2 stagingRallyPoint = canStageAssault
		? MakeOffsetToward(enemyAnchor, strategicTarget.position, m_config.enemyAI.rallyDistance)
		: enemyAnchor;
	const int32 stagingMinUnits = Max(1, (m_config.enemyAI.stagingAssaultMinUnits > 0)
		? m_config.enemyAI.stagingAssaultMinUnits
		: m_config.enemyAI.assaultUnitThreshold);
	int32 stagingReadyUnits = 0;
	if (canStageAssault)
	{
		const double gatherRadius = m_config.enemyAI.stagingAssaultGatherRadius;
		const double gatherRadiusSq = (gatherRadius * gatherRadius);
		gatherNearbyUnitIndices(Owner::Enemy, stagingRallyPoint, gatherRadius, m_nearbyUnitIndicesScratch);
		for (const auto index : m_nearbyUnitIndicesScratch)
		{
			const auto& unit = m_state.units[index];
			if (!IsEnemyCombatUnit(unit))
			{
				continue;
			}

			if (unit.position.distanceFromSq(stagingRallyPoint) <= gatherRadiusSq)
			{
				++stagingReadyUnits;
			}
		}
	}
	m_state.enemyAiDebugCombatUnitCount = enemyCombatUnits;
	m_state.enemyAiDebugReadyUnitCount = stagingReadyUnits;

	if (useStagingAssault)
	{
		if ((m_state.enemyAiAssaultCommitTimer > 0.0) && hasStrategicTarget)
		{
			m_state.enemyAiAssaultDestination = strategicTarget.position;
			m_state.enemyAiAssaultTargetUnitId = strategicTarget.unitId;
		}

		if (canStageAssault && (m_state.enemyAiAssaultCommitTimer <= 0.0) && (enemyCombatUnits >= stagingMinUnits))
		{
			m_state.enemyAiStagingTimer += m_config.enemyAI.decisionInterval;
			if ((stagingReadyUnits >= stagingMinUnits) || (m_state.enemyAiStagingTimer >= m_config.enemyAI.stagingAssaultMaxWait))
			{
				m_state.enemyAiAssaultCommitTimer = m_config.enemyAI.stagingAssaultCommitTime;
				m_state.enemyAiAssaultDestination = strategicTarget.position;
				m_state.enemyAiAssaultTargetUnitId = strategicTarget.unitId;
				m_state.enemyAiStagingTimer = 0.0;
			}
		}
		else if ((m_state.enemyAiAssaultCommitTimer <= 0.0) || !canStageAssault)
		{
			m_state.enemyAiStagingTimer = 0.0;
		}
	}
	else
	{
		m_state.enemyAiStagingTimer = 0.0;
		m_state.enemyAiAssaultCommitTimer = 0.0;
		m_state.enemyAiAssaultTargetUnitId.reset();
	}

	const bool shouldAssault = useStagingAssault
		? (m_state.enemyAiAssaultCommitTimer > 0.0)
		: ((enemyCombatUnits >= m_config.enemyAI.assaultUnitThreshold) && hasStrategicTarget);
	const Vec2 rallyPoint = useStagingAssault
		? stagingRallyPoint
		: (shouldAssault ? MakeOffsetToward(enemyAnchor, strategicTarget.position, m_config.enemyAI.rallyDistance) : enemyAnchor);
	const Vec2 assaultDestination = (useStagingAssault && (m_state.enemyAiAssaultCommitTimer > 0.0))
		? m_state.enemyAiAssaultDestination
		: strategicTarget.position;
	const Optional<int32> assaultTargetUnitId = (useStagingAssault && (m_state.enemyAiAssaultCommitTimer > 0.0))
		? m_state.enemyAiAssaultTargetUnitId
		: strategicTarget.unitId;

	for (const auto unitIndex : getOwnerUnitIndices(Owner::Enemy))
	{
		auto& unit = m_state.units[unitIndex];
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
		else if (useStagingAssault && IsEnemyCombatUnit(unit) && !shouldAssault && canStageAssault)
		{
			decision.objective = EnemyAiObjectiveType::Rally;
			decision.strategicDestination = rallyPoint;
		}
		else if (shouldAssault)
		{
			decision.objective = EnemyAiObjectiveType::Assault;
			decision.strategicDestination = assaultDestination;
			if (assaultTargetUnitId)
			{
				decision.targetUnitId = assaultTargetUnitId;
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

		const bool suppressNearbyTargetSelection = useStagingAssault && IsEnemyCombatUnit(unit) && !shouldAssault && canStageAssault;
		if (!decision.targetUnitId && !suppressNearbyTargetSelection)
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
