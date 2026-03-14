namespace
{
	[[nodiscard]] String GetConstructionFailureReason(const BattleSession& session, const UnitArchetype archetype, const Vec2& position, const Optional<int32> ignoredUnitId)
	{
		const auto* definition = FindUnitDefinition(session.config(), archetype);
		if (!definition)
		{
			return U"Construction unavailable";
		}

		for (const auto& obstacle : session.config().obstacles)
		{
			if (BattleSessionInternal::IntersectsObstacle(position, definition->radius + 6.0, obstacle))
			{
				return U"Blocked by obstacle";
			}
		}

		for (const auto& unit : session.state().units)
		{
			if (!unit.isAlive)
			{
				continue;
			}

			if (ignoredUnitId && (unit.id == *ignoredUnitId))
			{
				continue;
			}

			const double padding = ((unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker)) ? 10.0 : 6.0;
			if (position.distanceFrom(unit.position) < (definition->radius + unit.radius + padding))
			{
				return U"Too close to another unit";
			}
		}

		for (const auto& resourcePoint : session.state().resourcePoints)
		{
			if (position.distanceFrom(resourcePoint.position) < (definition->radius + resourcePoint.radius + 8.0))
			{
				return U"Too close to resource point";
			}
		}

		return U"Construction canceled";
	}
}

void BattleSession::updateConstructionOrders()
{
	for (size_t index = 0; index < m_state.pendingConstructionOrders.size();)
	{
		const PendingConstructionOrder order = m_state.pendingConstructionOrders[index];
		auto* worker = findCachedUnit(order.workerUnitId);
		if (!(worker && worker->isAlive && (worker->owner == Owner::Player) && (worker->archetype == UnitArchetype::Worker)))
		{
			m_state.playerGold += order.reservedCost;
			m_state.statusMessage = U"Builder lost";
			m_state.statusMessageTimer = 2.0;
			m_state.pendingConstructionOrders.remove_at(index);
			continue;
		}

		const auto& definition = getUnitDefinition(order.archetype);
		const double buildStartDistance = (worker->radius + definition.radius + 8.0);
		if (worker->position.distanceFrom(order.position) > buildStartDistance)
		{
			++index;
			continue;
		}

		if (tryPlaceBuilding(Owner::Player, order.archetype, order.position, order.workerUnitId, false))
		{
			worker->order.type = UnitOrderType::Idle;
			worker->order.targetUnitId.reset();
			worker->order.targetPoint = worker->position;
			worker->moveTarget = worker->position;
			BattleSessionInternal::ClearNavigationPath(*worker);
			m_state.pendingConstructionOrders.remove_at(index);
			continue;
		}

		if (!canPlaceBuilding(Owner::Player, order.archetype, order.position, order.workerUnitId))
		{
			m_state.playerGold += order.reservedCost;
			worker->order.type = UnitOrderType::Idle;
			worker->order.targetUnitId.reset();
			worker->order.targetPoint = worker->position;
			worker->moveTarget = worker->position;
			BattleSessionInternal::ClearNavigationPath(*worker);
			m_state.statusMessage = GetConstructionFailureReason(*this, order.archetype, order.position, order.workerUnitId);
			m_state.statusMessageTimer = 2.0;
			m_state.pendingConstructionOrders.remove_at(index);
			continue;
		}

		++index;
	}
}

void BattleSession::cancelPendingConstructionOrders(const Array<int32>& unitIds, const bool refundReservedCost)
{
	for (size_t index = 0; index < m_state.pendingConstructionOrders.size();)
	{
		if (!unitIds.contains(m_state.pendingConstructionOrders[index].workerUnitId))
		{
			++index;
			continue;
		}

		if (refundReservedCost)
		{
			m_state.playerGold += m_state.pendingConstructionOrders[index].reservedCost;
		}

		m_state.pendingConstructionOrders.remove_at(index);
	}
}

bool BattleSession::tryPlaceBuilding(const Owner owner, const UnitArchetype archetype, const Vec2& position, const Optional<int32> builderUnitId, const bool chargeCost)
{
	if (m_state.winner || !IsBuildingArchetype(archetype) || (archetype == UnitArchetype::Base))
	{
		return false;
	}

	if ((owner == Owner::Player) && m_state.tutorialActive)
	{
		if ((m_state.tutorialPhase != TutorialPhase::BuildStructure) || (archetype != m_config.tutorial.requiredConstruction))
		{
			return false;
		}
	}

	if (owner == Owner::Player)
	{
		if (builderUnitId)
		{
			const auto* builder = findCachedUnit(*builderUnitId);
			if (!(builder && builder->isAlive && (builder->owner == Owner::Player) && (builder->archetype == UnitArchetype::Worker)))
			{
				return false;
			}
		}
		else
		{
			bool hasSelectedWorker = false;
			for (const auto& unit : m_state.units)
			{
				if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
				{
					hasSelectedWorker = true;
					break;
				}
			}

			if (!hasSelectedWorker)
			{
				return false;
			}
		}
	}

	const int32 cost = getUnitCost(archetype);
	int32& gold = (owner == Owner::Player) ? m_state.playerGold : m_state.enemyGold;
	if (chargeCost && (gold < cost))
	{
		return false;
	}

	const Vec2 clampedPosition = ClampToWorld(m_state.worldBounds, position, getUnitDefinition(archetype).radius);
	if (!canPlaceBuilding(owner, archetype, clampedPosition, builderUnitId))
	{
		return false;
	}

	if (chargeCost)
	{
		gold -= cost;
	}
	const int32 buildingUnitId = spawnUnit(owner, archetype, clampedPosition);
	if (auto* building = m_state.findBuildingByUnitId(buildingUnitId))
	{
		building->isConstructed = false;
		building->constructionRemaining = getProductionTime(owner, archetype);
		building->constructionTotal = getProductionTime(owner, archetype);
	}

	return true;
}

bool BattleSession::canPlaceBuilding(const Owner owner, const UnitArchetype archetype, const Vec2& position, const Optional<int32> ignoredUnitId) const
{
	const auto& definition = getUnitDefinition(archetype);

	for (const auto& obstacle : m_config.obstacles)
	{
		if (BattleSessionInternal::IntersectsObstacle(position, definition.radius + 6.0, obstacle))
		{
			return false;
		}
	}

	for (const auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		if (ignoredUnitId && (unit.id == *ignoredUnitId))
		{
			continue;
		}

		const double padding = ((unit.owner == owner) && (unit.archetype == UnitArchetype::Worker)) ? 10.0 : 6.0;
		if (position.distanceFrom(unit.position) < (definition.radius + unit.radius + padding))
		{
			return false;
		}
	}

	for (const auto& resourcePoint : m_state.resourcePoints)
	{
		if (position.distanceFrom(resourcePoint.position) < (definition.radius + resourcePoint.radius + 8.0))
		{
			return false;
		}
	}

	return true;
}
