namespace
{
	[[nodiscard]] int32 CountStagingReadyUnitsRange(
		const BattleState& state,
		const Array<size_t>& nearbyEnemyUnitIndices,
		const Vec2& stagingRallyPoint,
		const double gatherRadiusSq,
		const size_t startIndex,
		const size_t endIndex)
	{
		int32 stagingReadyUnits = 0;

		for (size_t i = startIndex; i < endIndex; ++i)
		{
			const auto& unit = state.units[nearbyEnemyUnitIndices[i]];
			if (!IsEnemyCombatUnit(unit))
			{
				continue;
			}

			if (unit.position.distanceFromSq(stagingRallyPoint) <= gatherRadiusSq)
			{
				++stagingReadyUnits;
			}
		}

		return stagingReadyUnits;
	}

	[[nodiscard]] int32 CountStagingReadyUnits(
		const BattleState& state,
		const Array<size_t>& nearbyEnemyUnitIndices,
		const Vec2& stagingRallyPoint,
		const double gatherRadiusSq)
	{
		if (!ShouldRunEnemyAiUnitAsync(nearbyEnemyUnitIndices.size()))
		{
			return CountStagingReadyUnitsRange(state, nearbyEnemyUnitIndices, stagingRallyPoint, gatherRadiusSq, 0, nearbyEnemyUnitIndices.size());
		}

		const size_t taskCount = GetEnemyAiUnitDecisionTaskCount(nearbyEnemyUnitIndices.size());
		const size_t chunkSize = ((nearbyEnemyUnitIndices.size() + taskCount - 1) / taskCount);
		std::vector<std::future<int32>> stagingTasks;
		stagingTasks.reserve(taskCount);

		for (size_t taskIndex = 0; taskIndex < taskCount; ++taskIndex)
		{
			const size_t startIndex = (taskIndex * chunkSize);
			if (startIndex >= nearbyEnemyUnitIndices.size())
			{
				break;
			}

			const size_t endIndex = Min(startIndex + chunkSize, nearbyEnemyUnitIndices.size());
			stagingTasks.emplace_back(std::async(std::launch::async, [&, startIndex, endIndex]()
			{
				return CountStagingReadyUnitsRange(state, nearbyEnemyUnitIndices, stagingRallyPoint, gatherRadiusSq, startIndex, endIndex);
			}));
		}

		int32 stagingReadyUnits = 0;
		for (auto& stagingTask : stagingTasks)
		{
			stagingReadyUnits += stagingTask.get();
		}

		return stagingReadyUnits;
	}

	void ConsiderDefenseResponderCandidate(Array<EnemyAiDefenseResponderCandidate>& bestCandidates, const int32 defenseResponderLimit, const size_t unitIndex, const double distanceSq)
	{
		size_t insertIndex = 0;
		while ((insertIndex < bestCandidates.size()) && (bestCandidates[insertIndex].distanceSq <= distanceSq))
		{
			++insertIndex;
		}

		if (insertIndex >= static_cast<size_t>(defenseResponderLimit))
		{
			return;
		}

		bestCandidates.insert(bestCandidates.begin() + static_cast<ptrdiff_t>(insertIndex), EnemyAiDefenseResponderCandidate{ unitIndex, distanceSq });
		if (bestCandidates.size() > static_cast<size_t>(defenseResponderLimit))
		{
			bestCandidates.pop_back();
		}
	}

	[[nodiscard]] Array<EnemyAiDefenseResponderCandidate> CollectDefenseResponderCandidates(
		const BattleState& state,
		const Array<size_t>& enemyUnitIndices,
		const Vec2& defenseTargetPosition,
		const int32 defenseResponderLimit,
		const size_t startIndex,
		const size_t endIndex)
	{
		Array<EnemyAiDefenseResponderCandidate> bestCandidates;
		bestCandidates.reserve(defenseResponderLimit);

		for (size_t i = startIndex; i < endIndex; ++i)
		{
			const size_t unitIndex = enemyUnitIndices[i];
			const auto& unit = state.units[unitIndex];
			if (!IsEnemyCombatUnit(unit))
			{
				continue;
			}

			ConsiderDefenseResponderCandidate(bestCandidates, defenseResponderLimit, unitIndex, unit.position.distanceFromSq(defenseTargetPosition));
		}

		return bestCandidates;
	}

	[[nodiscard]] Array<size_t> SelectDefenseResponderIndices(
		const BattleState& state,
		const Array<size_t>& enemyUnitIndices,
		const Vec2& defenseTargetPosition,
		const int32 defenseResponderLimit)
	{
		if (defenseResponderLimit <= 0)
		{
			return {};
		}

		Array<EnemyAiDefenseResponderCandidate> bestCandidates;
		bestCandidates.reserve(defenseResponderLimit);

		if (ShouldRunEnemyAiUnitAsync(enemyUnitIndices.size()))
		{
			const size_t taskCount = GetEnemyAiUnitDecisionTaskCount(enemyUnitIndices.size());
			const size_t chunkSize = ((enemyUnitIndices.size() + taskCount - 1) / taskCount);
			std::vector<std::future<Array<EnemyAiDefenseResponderCandidate>>> responderTasks;
			responderTasks.reserve(taskCount);

			for (size_t taskIndex = 0; taskIndex < taskCount; ++taskIndex)
			{
				const size_t startIndex = (taskIndex * chunkSize);
				if (startIndex >= enemyUnitIndices.size())
				{
					break;
				}

				const size_t endIndex = Min(startIndex + chunkSize, enemyUnitIndices.size());
				responderTasks.emplace_back(std::async(std::launch::async, [&, startIndex, endIndex]()
				{
					return CollectDefenseResponderCandidates(state, enemyUnitIndices, defenseTargetPosition, defenseResponderLimit, startIndex, endIndex);
				}));
			}

			for (auto& responderTask : responderTasks)
			{
				for (const auto& candidate : responderTask.get())
				{
					ConsiderDefenseResponderCandidate(bestCandidates, defenseResponderLimit, candidate.unitIndex, candidate.distanceSq);
				}
			}
		}
		else
		{
			bestCandidates = CollectDefenseResponderCandidates(state, enemyUnitIndices, defenseTargetPosition, defenseResponderLimit, 0, enemyUnitIndices.size());
		}

		Array<size_t> defenseResponderIndices;
		defenseResponderIndices.reserve(bestCandidates.size());
		for (const auto& candidate : bestCandidates)
		{
			defenseResponderIndices << candidate.unitIndex;
		}

		return defenseResponderIndices;
	}

	[[nodiscard]] EnemyAiStrategicTarget EvaluateStrategicTarget(
		const BattleState& state,
		const Array<size_t>& playerBuildingIndices,
		const UnitState* playerBase,
		const Vec2& enemyAnchor,
		const bool canAssaultPlayerBase,
		const EnemyAiConfig& config,
		const Optional<size_t>& captureTargetIndex,
		const Vec2& captureTargetPosition)
	{
		EnemyAiStrategicTarget strategicTarget;

		for (const auto candidateIndex : playerBuildingIndices)
		{
			const auto& candidate = state.units[candidateIndex];
			if (!candidate.isAlive || (candidate.owner != Owner::Player) || !IsBuildingArchetype(candidate.archetype))
			{
				continue;
			}

			const double distancePenalty = enemyAnchor.distanceFrom(candidate.position) * 0.25;
			double score = -Math::Inf;

			if (candidate.archetype == UnitArchetype::Turret)
			{
				score = playerBase && IsBaseDefenseTurret(candidate, *playerBase, config.baseAssaultLockRadius)
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

		if (captureTargetIndex)
		{
			const double captureScore = 620.0 - (enemyAnchor.distanceFrom(captureTargetPosition) * 0.20);
			ConsiderStrategicTarget(strategicTarget, captureScore, captureTargetPosition);
		}

		return strategicTarget;
	}
}
