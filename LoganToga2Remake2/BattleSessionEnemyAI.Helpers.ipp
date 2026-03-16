namespace
{
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

	[[nodiscard]] int32 GetDefenseResponderLimit(const EnemyAiConfig& config, const int32 enemyCombatUnits)
	{
		const int32 desiredLimit = Max(2, config.assaultUnitThreshold + 1);
		return Min(enemyCombatUnits, desiredLimit);
	}

	[[nodiscard]] bool HasSameEnemyOrder(const UnitState& unit, const EnemyAiDecision& decision)
	{
		const UnitOrderType desiredOrderType = decision.targetUnitId
			? UnitOrderType::AttackTarget
			: UnitOrderType::Move;
		const double positionToleranceSq = (12.0 * 12.0);
		return (unit.order.type == desiredOrderType)
			&& (unit.order.targetUnitId == decision.targetUnitId)
			&& (unit.order.targetPoint.distanceFromSq(decision.strategicDestination) <= positionToleranceSq)
			&& (unit.moveTarget.distanceFromSq(decision.strategicDestination) <= positionToleranceSq);
	}

	[[nodiscard]] int32 GetEnemyAiSearchGroupCount(const int32 enemyCombatUnits)
	{
		return (enemyCombatUnits >= 8)
			? EnemyAiSearchGroupCountHeavy
			: EnemyAiSearchGroupCountLight;
	}

	[[nodiscard]] bool ShouldRunEnemyAiUnitAsync(const size_t workItemCount)
	{
		return (workItemCount >= EnemyAiAsyncMinUnitCount);
	}

	[[nodiscard]] bool ShouldRunEnemyAiBuildingAsync(const size_t workItemCount)
	{
		return (workItemCount >= EnemyAiAsyncMinBuildingCount);
	}

	[[nodiscard]] size_t GetEnemyAiUnitDecisionTaskCount(const size_t workItemCount)
	{
		size_t hardwareThreadCount = static_cast<size_t>(std::thread::hardware_concurrency());
		if (hardwareThreadCount == 0)
		{
			hardwareThreadCount = 1;
		}

		size_t taskCount = ((workItemCount + EnemyAiAsyncMinUnitCount - 1) / EnemyAiAsyncMinUnitCount);
		if (taskCount < 2)
		{
			taskCount = 2;
		}

		if (taskCount > hardwareThreadCount)
		{
			taskCount = hardwareThreadCount;
		}

		if (taskCount > workItemCount)
		{
			taskCount = workItemCount;
		}

		return taskCount;
	}

	[[nodiscard]] int32 CountEnemyCombatUnits(const BattleState& state, const Array<size_t>& enemyUnitIndices)
	{
		int32 enemyCombatUnits = 0;
		for (const auto index : enemyUnitIndices)
		{
			if (IsEnemyCombatUnit(state.units[index]))
			{
				++enemyCombatUnits;
			}
		}

		return enemyCombatUnits;
	}

	void ApplyEnemyAiDecision(UnitState& unit, const EnemyAiDecision& decision)
	{
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
		BattleSessionInternal::InvalidateNavigationPath(unit);
	}
}
