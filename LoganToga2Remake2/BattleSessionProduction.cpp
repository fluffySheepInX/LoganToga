#include "BattleSession.h"

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

		const auto* buildingUnit = findCachedUnit(building.unitId);
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
	BuildingState* selectedBuilding = nullptr;
	BuildingState* fallbackBuilding = nullptr;

	for (auto& building : m_state.buildings)
	{
		if (const auto* unit = findCachedUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == producerArchetype) && building.isConstructed)
			{
				if ((owner == Owner::Player) && unit->isSelected)
				{
					if ((!selectedBuilding) || (building.productionQueue.size() < selectedBuilding->productionQueue.size()))
					{
						selectedBuilding = &building;
					}
				}

				if ((!fallbackBuilding) || (building.productionQueue.size() < fallbackBuilding->productionQueue.size()))
				{
					fallbackBuilding = &building;
				}
			}
		}
	}

	return selectedBuilding ? selectedBuilding : fallbackBuilding;
}

const BuildingState* BattleSession::findProductionBuilding(const Owner owner, const UnitArchetype producerArchetype) const
{
	const BuildingState* selectedBuilding = nullptr;
	const BuildingState* fallbackBuilding = nullptr;

	for (const auto& building : m_state.buildings)
	{
		if (const auto* unit = findCachedUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == producerArchetype) && building.isConstructed)
			{
				if ((owner == Owner::Player) && unit->isSelected)
				{
					if ((!selectedBuilding) || (building.productionQueue.size() < selectedBuilding->productionQueue.size()))
					{
						selectedBuilding = &building;
					}
				}

				if ((!fallbackBuilding) || (building.productionQueue.size() < fallbackBuilding->productionQueue.size()))
				{
					fallbackBuilding = &building;
				}
			}
		}
	}

	return selectedBuilding ? selectedBuilding : fallbackBuilding;
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

	const int32 cost = (slot->cost > 0) ? slot->cost : getUnitCost(archetype);
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
	for (int32 count = 0; count < slot->batchCount; ++count)
	{
		building->productionQueue << ProductionQueueItem{ archetype, productionTime, productionTime };
	}
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
	static_cast<void>(owner);
	return getUnitDefinition(archetype).aggroRange;
}
