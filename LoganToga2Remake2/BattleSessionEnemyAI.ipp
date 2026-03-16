# include <future>
# include <thread>
# include <vector>

# include "BattleSessionEnemyAI.Types.ipp"
# include "BattleSessionEnemyAI.Helpers.ipp"
# include "BattleSessionEnemyAI.Strategy.ipp"

const UnitState* BattleSession::findNearestIntrudingPlayerUnit(const Vec2& assetPosition, const double defenseRadius, double& inOutDistanceSq, Array<size_t>& nearbyUnitIndicesScratch) const
{
	gatherNearbyUnitIndices(Owner::Player, assetPosition, defenseRadius, nearbyUnitIndicesScratch);

	const UnitState* nearest = nullptr;
	for (const auto candidateIndex : nearbyUnitIndicesScratch)
	{
		const auto& candidate = m_state.units[candidateIndex];
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		const double distanceSq = candidate.position.distanceFromSq(assetPosition);
		if (distanceSq > inOutDistanceSq)
		{
			continue;
		}

		inOutDistanceSq = distanceSq;
		nearest = &candidate;
	}

	return nearest;
}

void BattleSession::updateEnemyAI(const double deltaTime)
{
	if (m_state.tutorialActive && !m_state.tutorialEnemyWaveStarted)
	{
		return;
	}

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

	m_state.enemyAiDecisionTimer += deltaTime;
	if (m_state.enemyAiDecisionTimer < m_config.enemyAI.decisionInterval)
	{
		return;
	}

	m_state.enemyAiDecisionTimer = 0.0;

	const UnitState* enemyBase = findOwnerUnitByArchetype(Owner::Enemy, UnitArchetype::Base);
	const UnitState* enemyBarracks = findOwnerUnitByArchetype(Owner::Enemy, UnitArchetype::Barracks);
	const UnitState* playerBase = findOwnerUnitByArchetype(Owner::Player, UnitArchetype::Base);
	const auto& enemyUnitIndices = getOwnerUnitIndices(Owner::Enemy);
	const auto& enemyBuildingIndices = getOwnerBuildingIndices(Owner::Enemy);
	const auto& playerBuildingIndices = getOwnerBuildingIndices(Owner::Player);

	EnemyAiContext aiContext;
	aiContext.enemyAnchor = enemyBarracks ? enemyBarracks->position : (enemyBase ? enemyBase->position : m_state.worldBounds.center());
	aiContext.canAssaultPlayerBase = playerBase && !hasBaseDefenseTurret(*playerBase, m_config.enemyAI.baseAssaultLockRadius);

	Array<size_t> nearbyPlayerUnitIndicesScratch;
	Array<size_t> nearbyEnemyUnitIndicesScratch;
	const bool useEnemyCombatUnitsAsync = ShouldRunEnemyAiUnitAsync(enemyUnitIndices.size());
	std::future<int32> enemyCombatUnitsFuture;
	if (useEnemyCombatUnitsAsync)
	{
		enemyCombatUnitsFuture = std::async(std::launch::async, [&]()
		{
			return CountEnemyCombatUnits(m_state, enemyUnitIndices);
		});
	}

	double defenseTargetDistanceSq = (m_config.enemyAI.defenseRadius * m_config.enemyAI.defenseRadius);
	if (enemyBase)
	{
		if (const UnitState* candidate = findNearestIntrudingPlayerUnit(enemyBase->position, m_config.enemyAI.defenseRadius, defenseTargetDistanceSq, nearbyPlayerUnitIndicesScratch))
		{
			aiContext.defenseTargetUnitId = candidate->id;
			aiContext.defenseTargetPosition = candidate->position;
		}
	}
	for (const auto buildingIndex : enemyBuildingIndices)
	{
		const auto& unit = m_state.units[buildingIndex];
		if (!(unit.isAlive && (unit.owner == Owner::Enemy)))
		{
			continue;
		}

		if (const UnitState* candidate = findNearestIntrudingPlayerUnit(unit.position, m_config.enemyAI.defenseRadius, defenseTargetDistanceSq, nearbyPlayerUnitIndicesScratch))
		{
			aiContext.defenseTargetUnitId = candidate->id;
			aiContext.defenseTargetPosition = candidate->position;
		}
	}
	for (size_t resourceIndex = 0; resourceIndex < m_state.resourcePoints.size(); ++resourceIndex)
	{
		const auto& resourcePoint = m_state.resourcePoints[resourceIndex];
		if (resourcePoint.owner != Owner::Enemy)
		{
			continue;
		}

		if (const UnitState* candidate = findNearestIntrudingPlayerUnit(resourcePoint.position, m_config.enemyAI.defenseRadius, defenseTargetDistanceSq, nearbyPlayerUnitIndicesScratch))
		{
			aiContext.defenseTargetUnitId = candidate->id;
			aiContext.defenseTargetPosition = candidate->position;
		}
	}

	double captureTargetDistance = Math::Inf;
	for (size_t resourceIndex = 0; resourceIndex < m_state.resourcePoints.size(); ++resourceIndex)
	{
		const auto& resourcePoint = m_state.resourcePoints[resourceIndex];
		if (resourcePoint.owner == Owner::Enemy)
		{
			continue;
		}

		const double distance = aiContext.enemyAnchor.distanceFrom(resourcePoint.position);
		if (distance < captureTargetDistance)
		{
			captureTargetDistance = distance;
			aiContext.captureTargetIndex = resourceIndex;
			aiContext.captureTargetPosition = resourcePoint.position;
		}
	}

	aiContext.enemyCombatUnits = useEnemyCombatUnitsAsync
		? enemyCombatUnitsFuture.get()
		: CountEnemyCombatUnits(m_state, enemyUnitIndices);
	aiContext.searchGroupCount = GetEnemyAiSearchGroupCount(aiContext.enemyCombatUnits);
	aiContext.currentSearchPhase = (m_state.enemyAiSearchPhase % aiContext.searchGroupCount);
	m_state.enemyAiSearchPhase = ((aiContext.currentSearchPhase + 1) % aiContext.searchGroupCount);
	const bool useStrategicTargetAsync = ShouldRunEnemyAiBuildingAsync(playerBuildingIndices.size());
	std::future<EnemyAiStrategicTarget> strategicTargetFuture;
	if (useStrategicTargetAsync)
	{
		strategicTargetFuture = std::async(std::launch::async, [&]()
		{
			return EvaluateStrategicTarget(
				m_state,
				playerBuildingIndices,
				playerBase,
				aiContext.enemyAnchor,
				aiContext.canAssaultPlayerBase,
				m_config.enemyAI,
				aiContext.captureTargetIndex,
				aiContext.captureTargetPosition);
		});
	}

	Array<size_t> defenseResponderIndices;
	std::unordered_set<size_t> defenseResponderIndexSet;
	if (aiContext.defenseTargetUnitId)
	{
		const int32 defenseResponderLimit = GetDefenseResponderLimit(m_config.enemyAI, aiContext.enemyCombatUnits);
		defenseResponderIndices = SelectDefenseResponderIndices(m_state, enemyUnitIndices, aiContext.defenseTargetPosition, defenseResponderLimit);

		defenseResponderIndexSet.reserve(defenseResponderIndices.size());
		for (const auto index : defenseResponderIndices)
		{
			defenseResponderIndexSet.insert(index);
		}
	}

	aiContext.strategicTarget = useStrategicTargetAsync
		? strategicTargetFuture.get()
		: EvaluateStrategicTarget(
			m_state,
			playerBuildingIndices,
			playerBase,
			aiContext.enemyAnchor,
			aiContext.canAssaultPlayerBase,
			m_config.enemyAI,
			aiContext.captureTargetIndex,
			aiContext.captureTargetPosition);

	aiContext.useStagingAssault = (activeEnemyAiMode == EnemyAiMode::StagingAssault);
	aiContext.hasStrategicTarget = (aiContext.strategicTarget.score > -Math::Inf);
	aiContext.canStageAssault = aiContext.useStagingAssault && !aiContext.defenseTargetUnitId && aiContext.hasStrategicTarget;
	aiContext.stagingRallyPoint = aiContext.canStageAssault
		? MakeOffsetToward(aiContext.enemyAnchor, aiContext.strategicTarget.position, m_config.enemyAI.rallyDistance)
		: aiContext.enemyAnchor;
	aiContext.stagingMinUnits = Max(1, (m_config.enemyAI.stagingAssaultMinUnits > 0)
		? m_config.enemyAI.stagingAssaultMinUnits
		: m_config.enemyAI.assaultUnitThreshold);
	if (aiContext.canStageAssault)
	{
		const double gatherRadius = m_config.enemyAI.stagingAssaultGatherRadius;
		const double gatherRadiusSq = (gatherRadius * gatherRadius);
		gatherNearbyUnitIndices(Owner::Enemy, aiContext.stagingRallyPoint, gatherRadius, nearbyEnemyUnitIndicesScratch);
		aiContext.stagingReadyUnits = CountStagingReadyUnits(m_state, nearbyEnemyUnitIndicesScratch, aiContext.stagingRallyPoint, gatherRadiusSq);
	}
	m_state.enemyAiDebugCombatUnitCount = aiContext.enemyCombatUnits;
	m_state.enemyAiDebugReadyUnitCount = aiContext.stagingReadyUnits;

	if (aiContext.useStagingAssault)
	{
		if ((m_state.enemyAiAssaultCommitTimer > 0.0) && aiContext.hasStrategicTarget)
		{
			m_state.enemyAiAssaultDestination = aiContext.strategicTarget.position;
			m_state.enemyAiAssaultTargetUnitId = aiContext.strategicTarget.unitId;
		}

		if (aiContext.canStageAssault && (m_state.enemyAiAssaultCommitTimer <= 0.0) && (aiContext.enemyCombatUnits >= aiContext.stagingMinUnits))
		{
			m_state.enemyAiStagingTimer += m_config.enemyAI.decisionInterval;
			if ((aiContext.stagingReadyUnits >= aiContext.stagingMinUnits) || (m_state.enemyAiStagingTimer >= m_config.enemyAI.stagingAssaultMaxWait))
			{
				m_state.enemyAiAssaultCommitTimer = m_config.enemyAI.stagingAssaultCommitTime;
				m_state.enemyAiAssaultDestination = aiContext.strategicTarget.position;
				m_state.enemyAiAssaultTargetUnitId = aiContext.strategicTarget.unitId;
				m_state.enemyAiStagingTimer = 0.0;
			}
		}
		else if ((m_state.enemyAiAssaultCommitTimer <= 0.0) || !aiContext.canStageAssault)
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

	aiContext.shouldAssault = aiContext.useStagingAssault
		? (m_state.enemyAiAssaultCommitTimer > 0.0)
		: ((aiContext.enemyCombatUnits >= m_config.enemyAI.assaultUnitThreshold) && aiContext.hasStrategicTarget);
	aiContext.rallyPoint = aiContext.useStagingAssault
		? aiContext.stagingRallyPoint
		: (aiContext.shouldAssault ? MakeOffsetToward(aiContext.enemyAnchor, aiContext.strategicTarget.position, m_config.enemyAI.rallyDistance) : aiContext.enemyAnchor);
	aiContext.assaultDestination = (aiContext.useStagingAssault && (m_state.enemyAiAssaultCommitTimer > 0.0))
		? m_state.enemyAiAssaultDestination
		: aiContext.strategicTarget.position;
	aiContext.assaultTargetUnitId = (aiContext.useStagingAssault && (m_state.enemyAiAssaultCommitTimer > 0.0))
		? m_state.enemyAiAssaultTargetUnitId
		: aiContext.strategicTarget.unitId;

	Array<EnemyAiPendingUpdate> pendingUpdates(m_state.units.size());
	const auto evaluateEnemyUnitUpdate = [&](const size_t unitIndex, Array<size_t>& localNearbyOpponentIndicesScratch)
	{
		EnemyAiPendingUpdate pendingUpdate;
		const auto& unit = m_state.units[unitIndex];
		if (!unit.isAlive || (unit.owner != Owner::Enemy) || !unit.canMove)
		{
			return pendingUpdate;
		}

		EnemyAiDecision decision;
		if (aiContext.defenseTargetUnitId && (defenseResponderIndexSet.find(unitIndex) != defenseResponderIndexSet.end()))
		{
			decision.objective = EnemyAiObjectiveType::Defend;
			decision.strategicDestination = aiContext.defenseTargetPosition;
			decision.targetUnitId = aiContext.defenseTargetUnitId;
		}
		else if ((unit.archetype == UnitArchetype::Worker) && aiContext.captureTargetIndex)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = aiContext.captureTargetPosition;
		}
		else if (aiContext.useStagingAssault && IsEnemyCombatUnit(unit) && !aiContext.shouldAssault && aiContext.canStageAssault)
		{
			decision.objective = EnemyAiObjectiveType::Rally;
			decision.strategicDestination = aiContext.rallyPoint;
		}
		else if (aiContext.shouldAssault)
		{
			decision.objective = EnemyAiObjectiveType::Assault;
			decision.strategicDestination = aiContext.assaultDestination;
			if (aiContext.assaultTargetUnitId)
			{
				decision.targetUnitId = aiContext.assaultTargetUnitId;
			}
		}
		else if (aiContext.captureTargetIndex)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = aiContext.captureTargetPosition;
		}
		else
		{
			decision.objective = EnemyAiObjectiveType::Rally;
			decision.strategicDestination = aiContext.rallyPoint;
		}

		const bool suppressNearbyTargetSelection = aiContext.useStagingAssault && IsEnemyCombatUnit(unit) && !aiContext.shouldAssault && aiContext.canStageAssault;
		if (!decision.targetUnitId && !suppressNearbyTargetSelection)
		{
			if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
			{
				if (const UnitState* currentTarget = findCachedUnit(*unit.order.targetUnitId))
				{
					if (currentTarget->isAlive
						&& IsEnemy(unit, *currentTarget)
						&& !((currentTarget->archetype == UnitArchetype::Base) && !aiContext.canAssaultPlayerBase))
					{
						decision.targetUnitId = currentTarget->id;
						decision.strategicDestination = currentTarget->position;
					}
				}
			}

			if (!decision.targetUnitId && ((unit.id % aiContext.searchGroupCount) == aiContext.currentSearchPhase))
			{
				if (const UnitState* nearbyTarget = findNearestEnemy(unit, localNearbyOpponentIndicesScratch))
				{
					if (!((nearbyTarget->archetype == UnitArchetype::Base) && !aiContext.canAssaultPlayerBase))
					{
						decision.targetUnitId = nearbyTarget->id;
						decision.strategicDestination = nearbyTarget->position;
					}
				}
			}
		}

		if (HasSameEnemyOrder(unit, decision))
		{
			return pendingUpdate;
		}

		pendingUpdate.shouldApply = true;
		pendingUpdate.decision = decision;
		return pendingUpdate;
	};

	if (ShouldRunEnemyAiUnitAsync(enemyUnitIndices.size()))
	{
		rebuildUnitIndex();
		rebuildSpatialQueryCache();

		const size_t taskCount = GetEnemyAiUnitDecisionTaskCount(enemyUnitIndices.size());
		const size_t chunkSize = ((enemyUnitIndices.size() + taskCount - 1) / taskCount);
		std::vector<std::future<void>> decisionTasks;
		decisionTasks.reserve(taskCount);

		for (size_t taskIndex = 0; taskIndex < taskCount; ++taskIndex)
		{
			const size_t startIndex = (taskIndex * chunkSize);
			if (startIndex >= enemyUnitIndices.size())
			{
				break;
			}

			const size_t endIndex = Min(startIndex + chunkSize, enemyUnitIndices.size());
			decisionTasks.emplace_back(std::async(std::launch::async, [&, startIndex, endIndex]()
			{
				Array<size_t> localNearbyOpponentIndicesScratch;
				for (size_t i = startIndex; i < endIndex; ++i)
				{
					const size_t unitIndex = enemyUnitIndices[i];
					pendingUpdates[unitIndex] = evaluateEnemyUnitUpdate(unitIndex, localNearbyOpponentIndicesScratch);
				}
			}));
		}

		for (auto& decisionTask : decisionTasks)
		{
			decisionTask.get();
		}
	}
	else
	{
		Array<size_t> nearbyOpponentIndicesScratch;
		for (const auto unitIndex : enemyUnitIndices)
		{
			pendingUpdates[unitIndex] = evaluateEnemyUnitUpdate(unitIndex, nearbyOpponentIndicesScratch);
		}
	}

	for (const auto unitIndex : enemyUnitIndices)
	{
		if (!pendingUpdates[unitIndex].shouldApply)
		{
			continue;
		}

		auto& unit = m_state.units[unitIndex];
		ApplyEnemyAiDecision(unit, pendingUpdates[unitIndex].decision);
	}
}
