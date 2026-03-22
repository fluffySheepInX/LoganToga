#include "BattleSession.h"

namespace
{
	[[nodiscard]] constexpr bool IsEnemyAiDebugBuildEnabled()
	{
#if _DEBUG
		return true;
#else
		return false;
#endif
	}

	[[nodiscard]] bool HasConstructedPlayerBuilding(const BattleState& state, const UnitArchetype archetype)
	{
		for (const auto& building : state.buildings)
		{
			const auto* unit = state.findUnit(building.unitId);
			if (unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == archetype) && building.isConstructed)
			{
				return true;
			}
		}

		return false;
	}

	[[nodiscard]] int32 CountLivingEnemyCombatUnits(const BattleState& state)
	{
		int32 count = 0;
		for (const auto& unit : state.units)
		{
			if (unit.isAlive && (unit.owner == Owner::Enemy) && unit.canMove && (unit.archetype != UnitArchetype::Worker))
			{
				++count;
			}
		}

		return count;
	}
}

BattleSession::BattleSession()
	: BattleSession{ LoadBattleConfig(U"config/battle.toml") }

{
}

BattleSession::BattleSession(const BattleConfigData& config)
	: m_config{ config }
{
	setupInitialState();
}

void BattleSession::reset(const BattleConfigData& config)
{
	m_config = config;
	m_state = BattleState{};
	m_pendingCommands.clear();
	invalidateUnitIndex();
	invalidateBuildingIndex();
	setupInitialState();
}

void BattleSession::enqueue(BattleCommand command)
{
	m_pendingCommands << std::move(command);
}

void BattleSession::update(const double deltaTime)
{
	rebuildUnitIndex();

	m_state.statusMessageTimer = Max(m_state.statusMessageTimer - deltaTime, 0.0);
	if (m_state.statusMessageTimer <= 0.0)
	{
		m_state.statusMessage.clear();
	}

	for (auto& unit : m_state.units)
	{
		unit.damageFlashTime = Max(unit.damageFlashTime - deltaTime, 0.0);
	}

	for (auto& effect : m_state.deathVisualEffects)
	{
		effect.remainingTime = Max(effect.remainingTime - deltaTime, 0.0);
	}

	m_state.deathVisualEffects.remove_if([](const DeathVisualEffect& effect)
	{
		return (effect.remainingTime <= 0.0);
	});

	if (m_state.winner)
	{
		return;
	}

	for (auto& unit : m_state.units)
	{
		unit.previousPosition = unit.position;
	}

	processCommands();
	updateEconomy(deltaTime);
	updateProduction(deltaTime);
	updateEnemyAI(deltaTime);
	updateMovement(deltaTime);
	updateConstructionOrders();
	updateTutorial(deltaTime);
	updateResourcePoints(deltaTime);
	updateCombat();
	cleanupDeadUnits();
	updateVictoryState();
}

const BattleState& BattleSession::state() const noexcept
{
	return m_state;
}

BattleState& BattleSession::state() noexcept
{
	invalidateUnitIndex();
	invalidateBuildingIndex();
	return m_state;
}

const BattleConfigData& BattleSession::config() const noexcept
{
	return m_config;
}

void BattleSession::setupInitialState()
{
	m_state.worldBounds = RectF{ 0, 0, m_config.world.width, m_config.world.height };
	m_state.playerGold = m_config.playerGold;
	m_state.enemyGold = m_config.enemyGold;
	m_state.enemyAiResolvedMode = m_config.enemyAI.mode;

	for (const auto& placement : m_config.initialUnits)
	{
		spawnUnit(placement.owner, placement.archetype, placement.position);
	}

	for (const auto& resourcePoint : m_config.resourcePoints)
	{
		m_state.resourcePoints << ResourcePointState{
			.label = resourcePoint.label,
			.position = resourcePoint.position,
			.radius = resourcePoint.radius,
			.incomeAmount = resourcePoint.incomeAmount,
			.captureTime = resourcePoint.captureTime,
			.owner = resourcePoint.owner,
		};
	}

	if (m_config.tutorial.enabled)
	{
		m_state.tutorialActive = true;
		for (const auto& unit : m_state.units)
		{
			if ((unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
			{
				m_state.tutorialWorkerUnitId = unit.id;
				break;
			}
		}
		beginTutorialPhase(TutorialPhase::MoveUnit);
	}
}
