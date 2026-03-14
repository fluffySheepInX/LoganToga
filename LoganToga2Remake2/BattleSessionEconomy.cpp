#include "BattleSession.h"

bool BattleSession::trySpawnPlayerUnit(const UnitArchetype archetype)
{
	return tryQueueUnitProduction(Owner::Player, archetype);
}

bool BattleSession::cancelLastPlayerProduction()
{
	BuildingState* selectedBuilding = nullptr;
	BuildingState* fallbackBuilding = nullptr;

	for (auto& building : m_state.buildings)
	{
		const auto* unit = findCachedUnit(building.unitId);
		if (!(unit && unit->isAlive && (unit->owner == Owner::Player) && !building.productionQueue.isEmpty()))
		{
			continue;
		}

		if (unit->isSelected)
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

	BuildingState* targetBuilding = selectedBuilding ? selectedBuilding : fallbackBuilding;
	if (!targetBuilding)
	{
		return false;
	}

	const int32 refundCost = targetBuilding->productionQueue.back().queuedCost;
	targetBuilding->productionQueue.pop_back();
	m_state.playerGold += refundCost;
	return true;
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
	if (!(m_state.tutorialActive && m_config.tutorial.enabled))
	{
		m_state.enemySpawnTimer += deltaTime;
	}

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

	if (!(m_state.tutorialActive && m_config.tutorial.enabled) && (m_state.enemySpawnTimer >= m_config.enemySpawn.interval))
	{
		m_state.enemySpawnTimer -= m_config.enemySpawn.interval;
		const UnitArchetype archetype = ((m_state.enemyGold >= getUnitCost(m_config.enemySpawn.advancedArchetype)) && RandomBool(m_config.enemySpawn.advancedProbability))
			? m_config.enemySpawn.advancedArchetype
			: m_config.enemySpawn.basicArchetype;
		[[maybe_unused]] const bool queued = tryQueueUnitProduction(Owner::Enemy, archetype);
	}
}
