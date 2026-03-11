#include "BattleSession.h"

bool BattleSession::trySpawnPlayerUnit(const UnitArchetype archetype)
{
	return tryQueueUnitProduction(Owner::Player, archetype);
}

bool BattleSession::cancelLastPlayerProduction()
{
	for (auto& building : m_state.buildings)
	{
		const auto* unit = m_state.findUnit(building.unitId);
		if (!(unit && unit->isAlive && (unit->owner == Owner::Player) && !building.productionQueue.isEmpty()))
		{
			continue;
		}

		const UnitArchetype archetype = building.productionQueue.back().archetype;
		building.productionQueue.pop_back();
		m_state.playerGold += getUnitCost(archetype);
		return true;
	}

	return false;
}

void BattleSession::updateEconomy(const double deltaTime)
{
	int32 playerResourceIncome = 0;
	int32 enemyResourceIncome = 0;
	for (const auto& resourcePoint : m_state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Player)
		{
			playerResourceIncome += resourcePoint.incomeAmount;
		}
		else if (resourcePoint.owner == Owner::Enemy)
		{
			enemyResourceIncome += resourcePoint.incomeAmount;
		}
	}

	m_state.playerIncomeTimer += deltaTime;
	m_state.enemyIncomeTimer += deltaTime;
	m_state.enemySpawnTimer += deltaTime;

	if (m_state.playerIncomeTimer >= m_config.income.interval)
	{
		m_state.playerIncomeTimer -= m_config.income.interval;
		m_state.playerGold += (m_config.income.playerAmount + playerResourceIncome);
	}

	if (m_state.enemyIncomeTimer >= m_config.income.interval)
	{
		m_state.enemyIncomeTimer -= m_config.income.interval;
		m_state.enemyGold += (m_config.income.enemyAmount + enemyResourceIncome);
	}

	if (m_state.enemySpawnTimer >= m_config.enemySpawn.interval)
	{
		m_state.enemySpawnTimer -= m_config.enemySpawn.interval;
		const UnitArchetype archetype = ((m_state.enemyGold >= getUnitCost(m_config.enemySpawn.advancedArchetype)) && RandomBool(m_config.enemySpawn.advancedProbability))
			? m_config.enemySpawn.advancedArchetype
			: m_config.enemySpawn.basicArchetype;
		[[maybe_unused]] const bool queued = tryQueueUnitProduction(Owner::Enemy, archetype);
	}
}

void BattleSession::updateProduction(const double deltaTime)
{
	for (auto& building : m_state.buildings)
	{
		if (!building.isConstructed)
		{
			building.constructionRemaining = Max(building.constructionRemaining - deltaTime, 0.0);
			if (building.constructionRemaining <= 0.0)
			{
				building.isConstructed = true;
			}
			continue;
		}

		if (building.productionQueue.isEmpty())
		{
			continue;
		}

		const auto* buildingUnit = m_state.findUnit(building.unitId);
		if (!(buildingUnit && buildingUnit->isAlive))
		{
			continue;
		}

		auto& currentItem = building.productionQueue.front();
		currentItem.remainingTime = Max(currentItem.remainingTime - deltaTime, 0.0);

		if (currentItem.remainingTime > 0.0)
		{
			continue;
		}

		const Vec2 spawnPosition = getProductionSpawnPoint(*buildingUnit, currentItem.archetype);
		spawnUnit(buildingUnit->owner, currentItem.archetype, spawnPosition);
		building.productionQueue.remove_at(0);
	}
}

BuildingState* BattleSession::findProductionBuilding(const Owner owner, const UnitArchetype producerArchetype)
{
	for (auto& building : m_state.buildings)
	{
		if (const auto* unit = m_state.findUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == producerArchetype) && building.isConstructed)
			{
				return &building;
			}
		}
	}

	return nullptr;
}

const BuildingState* BattleSession::findProductionBuilding(const Owner owner, const UnitArchetype producerArchetype) const
{
	for (const auto& building : m_state.buildings)
	{
		if (const auto* unit = m_state.findUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == producerArchetype) && building.isConstructed)
			{
				return &building;
			}
		}
	}

	return nullptr;
}

const ProductionSlot* BattleSession::findProductionSlot(const UnitArchetype archetype) const
{
	for (const auto& slot : m_config.playerProductionSlots)
	{
		if (slot.archetype == archetype)
		{
			return &slot;
		}
	}

	return nullptr;
}

bool BattleSession::tryQueueUnitProduction(const Owner owner, const UnitArchetype archetype)
{
	if (m_state.winner)
	{
		return false;
	}

	if ((owner == Owner::Player) && !ContainsArchetype(m_config.playerAvailableProductionArchetypes, archetype))
	{
		return false;
	}

	const auto* slot = findProductionSlot(archetype);
	if (!slot)
	{
		return false;
	}

	auto* building = findProductionBuilding(owner, slot->producer);
	if (!building)
	{
		return false;
	}

	const int32 cost = getUnitCost(archetype);
	if (cost <= 0)
	{
		return false;
	}

	int32& gold = (owner == Owner::Player) ? m_state.playerGold : m_state.enemyGold;
	if (gold < cost)
	{
		return false;
	}

	gold -= cost;
	const double productionTime = getProductionTime(owner, archetype);
	building->productionQueue << ProductionQueueItem{ archetype, productionTime, productionTime };
	return true;
}

Vec2 BattleSession::getProductionSpawnPoint(const UnitState& buildingUnit, const UnitArchetype archetype) const
{
	const auto& unitDefinition = getUnitDefinition(archetype);
	const double direction = (buildingUnit.owner == Owner::Player) ? 1.0 : -1.0;
	const Vec2 spawnPosition = buildingUnit.position.movedBy(
		direction * (buildingUnit.radius + unitDefinition.radius + 20.0),
		Random(-buildingUnit.radius * 0.6, buildingUnit.radius * 0.6));
	return ClampToWorld(m_state.worldBounds, spawnPosition, unitDefinition.radius);
}

const UnitDefinition& BattleSession::getUnitDefinition(const UnitArchetype archetype) const
{
	for (const auto& definition : m_config.unitDefinitions)
	{
		if (definition.archetype == archetype)
		{
			return definition;
		}
	}

	throw Error{ U"Unit definition not found" };
}

int32 BattleSession::getUnitCost(const UnitArchetype archetype) const
{
	return getUnitDefinition(archetype).cost;
}

double BattleSession::getProductionTime(const Owner owner, const UnitArchetype archetype) const
{
	double productionTime = getUnitDefinition(archetype).productionTime;
	if ((owner == Owner::Player) && FindPlayerUnitModifier(m_config, archetype))
	{
		productionTime = Max(0.2, productionTime - FindPlayerUnitModifier(m_config, archetype)->productionTimeDelta);
	}

	return productionTime;
}

double BattleSession::getAggroRange(const Owner owner, const UnitArchetype archetype) const
{
	double aggroRange = getUnitDefinition(archetype).aggroRange;
	if ((owner == Owner::Player) && FindPlayerUnitModifier(m_config, archetype))
	{
		aggroRange += FindPlayerUnitModifier(m_config, archetype)->attackRangeDelta;
	}

	return aggroRange;
}
