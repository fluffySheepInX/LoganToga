#include "BattleSession.h"
#include "BattleSessionInternal.h"

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
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			unit.isSelected = false;
		}
	}

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

bool BattleSession::tryPlaceBuilding(const Owner owner, const UnitArchetype archetype, const Vec2& position)
{
	if (m_state.winner || !IsBuildingArchetype(archetype) || (archetype == UnitArchetype::Base))
	{
		return false;
	}

	if (owner == Owner::Player)
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

	const int32 cost = getUnitCost(archetype);
	int32& gold = (owner == Owner::Player) ? m_state.playerGold : m_state.enemyGold;
	if (gold < cost)
	{
		return false;
	}

	const Vec2 clampedPosition = ClampToWorld(m_state.worldBounds, position, getUnitDefinition(archetype).radius);
	if (!canPlaceBuilding(owner, archetype, clampedPosition))
	{
		return false;
	}

	gold -= cost;
	const int32 buildingUnitId = spawnUnit(owner, archetype, clampedPosition);
	if (auto* building = m_state.findBuildingByUnitId(buildingUnitId))
	{
		building->isConstructed = false;
		building->constructionRemaining = getProductionTime(archetype);
		building->constructionTotal = getProductionTime(archetype);
	}

	return true;
}

bool BattleSession::canPlaceBuilding(const Owner owner, const UnitArchetype archetype, const Vec2& position) const
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

	return unit;
}
