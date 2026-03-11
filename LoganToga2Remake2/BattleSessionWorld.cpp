#include "BattleSession.h"
#include "BattleSessionInternal.h"

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
		auto* worker = m_state.findUnit(order.workerUnitId);
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

void BattleSession::updateResourcePoints(const double deltaTime)
{
	for (auto& resourcePoint : m_state.resourcePoints)
	{
		bool playerPresent = false;
		bool enemyPresent = false;

		for (const auto& unit : m_state.units)
		{
			if (!unit.isAlive || IsBuildingArchetype(unit.archetype))
			{
				continue;
			}

			if (!Circle{ resourcePoint.position, resourcePoint.radius }.intersects(unit.position))
			{
				continue;
			}

			if (unit.owner == Owner::Player)
			{
				playerPresent = true;
			}
			else if (unit.owner == Owner::Enemy)
			{
				enemyPresent = true;
			}
		}

		if (playerPresent == enemyPresent)
		{
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
			continue;
		}

		const Owner occupier = playerPresent ? Owner::Player : Owner::Enemy;
		if (resourcePoint.owner == occupier)
		{
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
			continue;
		}

		if (!resourcePoint.capturingOwner || (*resourcePoint.capturingOwner != occupier))
		{
			resourcePoint.capturingOwner = occupier;
			resourcePoint.captureProgress = 0.0;
		}

		resourcePoint.captureProgress = Min(resourcePoint.captureProgress + deltaTime, resourcePoint.captureTime);
		if (resourcePoint.captureProgress >= resourcePoint.captureTime)
		{
			resourcePoint.owner = occupier;
			resourcePoint.capturingOwner.reset();
			resourcePoint.captureProgress = 0.0;
		}
	}
}

void BattleSession::cleanupDeadUnits()
{
	Array<int32> deadUnitIds;
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			unit.isSelected = false;
			deadUnitIds << unit.id;
		}
	}

	cancelPendingConstructionOrders(deadUnitIds, true);

	m_state.units.remove_if([](const UnitState& unit)
	{
		return !unit.isAlive && !IsBuildingArchetype(unit.archetype);
	});

	m_state.buildings.remove_if([this](const BuildingState& building)
	{
		const auto* unit = m_state.findUnit(building.unitId);
		return !(unit && unit->isAlive);
	});

	cleanupSquads();
}

void BattleSession::updateVictoryState()
{
	bool hasPlayerBase = false;
	bool hasEnemyBase = false;

	for (const auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.archetype != UnitArchetype::Base))
		{
			continue;
		}

		if (unit.owner == Owner::Player)
		{
			hasPlayerBase = true;
		}
		else
		{
			hasEnemyBase = true;
		}
	}

	if (!hasEnemyBase)
	{
		m_state.winner = Owner::Player;
	}
	else if (!hasPlayerBase)
	{
		m_state.winner = Owner::Enemy;
	}
}

bool BattleSession::tryPlaceBuilding(const Owner owner, const UnitArchetype archetype, const Vec2& position, const Optional<int32> builderUnitId, const bool chargeCost)
{
	if (m_state.winner || !IsBuildingArchetype(archetype) || (archetype == UnitArchetype::Base))
	{
		return false;
	}

	if (owner == Owner::Player)
	{
		if (builderUnitId)
		{
			const auto* builder = m_state.findUnit(*builderUnitId);
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

int32 BattleSession::spawnUnit(const Owner owner, const UnitArchetype archetype, const Vec2& position)
{
	const int32 id = m_state.nextUnitId++;
	m_state.units << makeUnit(id, owner, archetype, position);
	if (IsBuildingArchetype(archetype))
	{
		m_state.buildings << BuildingState{ id };
	}
	return id;
}

UnitState BattleSession::makeUnit(const int32 id, const Owner owner, const UnitArchetype archetype, const Vec2& position) const
{
	const auto& definition = getUnitDefinition(archetype);

	UnitState unit;
	unit.id = id;
	unit.owner = owner;
	unit.archetype = archetype;
	unit.position = position;
	unit.moveTarget = position;
	unit.radius = definition.radius;
	unit.moveSpeed = definition.moveSpeed;
	unit.attackRange = definition.attackRange;
	unit.attackCooldown = definition.attackCooldown;
	unit.attackPower = definition.attackPower;
	unit.hp = definition.hp;
	unit.maxHp = definition.hp;
	unit.canMove = definition.canMove;
	if ((owner == Owner::Player) && FindPlayerUnitModifier(m_config, archetype))
	{
		const auto& modifier = *FindPlayerUnitModifier(m_config, archetype);
		unit.moveSpeed += modifier.moveSpeedDelta;
		unit.attackRange += modifier.attackRangeDelta;
		unit.attackPower += modifier.attackPowerDelta;
		unit.hp += modifier.hpDelta;
		unit.maxHp += modifier.hpDelta;
	}

	return unit;
}
