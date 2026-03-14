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

	[[nodiscard]] String GetEnemyAiModeLabel(const EnemyAiMode mode)
	{
		switch (mode)
		{
		case EnemyAiMode::StagingAssault:
			return U"STAGING";
		case EnemyAiMode::Default:
		default:
			return U"DEFAULT";
		}
	}

	[[nodiscard]] String GetTutorialObjective(const BattleConfigData& config, const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
			return config.tutorial.objectiveMove;
		case TutorialPhase::BuildStructure:
			return config.tutorial.objectiveBuild;
		case TutorialPhase::PrepareDefense:
			return config.tutorial.objectivePrepare;
		case TutorialPhase::ProduceUnit:
			return config.tutorial.objectiveProduce;
		case TutorialPhase::DefendWave:
			return config.tutorial.objectiveDefend;
		case TutorialPhase::Completed:
			return config.tutorial.objectiveComplete;
		case TutorialPhase::None:
		default:
			return U"";
		}
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

	if (m_state.winner)
	{
		return;
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
	return m_state;
}

const BattleConfigData& BattleSession::config() const noexcept
{
	return m_config;
}

void BattleSession::toggleEnemyAiDebugPanel()
{
	if (!(IsEnemyAiDebugBuildEnabled() && m_config.debug.enableEnemyAiSwitcher))
	{
		return;
	}

	m_state.enemyAiDebugPanelVisible = !m_state.enemyAiDebugPanelVisible;
	m_state.statusMessage = m_state.enemyAiDebugPanelVisible
		? U"Enemy AI debug panel ON"
		: U"Enemy AI debug panel OFF";
	m_state.statusMessageTimer = 1.2;
}

void BattleSession::cycleEnemyAiDebugMode()
{
	if (!(IsEnemyAiDebugBuildEnabled() && m_config.debug.enableEnemyAiSwitcher))
	{
		return;
	}

	if (!m_state.enemyAiDebugOverrideMode)
	{
		setEnemyAiDebugOverrideMode(EnemyAiMode::Default);
		return;
	}

	if (*m_state.enemyAiDebugOverrideMode == EnemyAiMode::Default)
	{
		setEnemyAiDebugOverrideMode(EnemyAiMode::StagingAssault);
		return;
	}

	setEnemyAiDebugOverrideMode(none);
}

void BattleSession::setEnemyAiDebugOverrideMode(const Optional<EnemyAiMode>& mode)
{
	if (!(IsEnemyAiDebugBuildEnabled() && m_config.debug.enableEnemyAiSwitcher))
	{
		return;
	}

	const bool isSameMode = (!m_state.enemyAiDebugOverrideMode && !mode)
		|| (m_state.enemyAiDebugOverrideMode && mode && (*m_state.enemyAiDebugOverrideMode == *mode));
	if (isSameMode)
	{
		return;
	}

	m_state.enemyAiDebugOverrideMode = mode;
	resetEnemyAiAssaultState();
	m_state.enemyAiResolvedMode = mode ? *mode : m_config.enemyAI.mode;
	m_state.enemyAiDecisionTimer = m_config.enemyAI.decisionInterval;
	m_state.statusMessage = mode
		? (U"Enemy AI: " + GetEnemyAiModeLabel(*mode))
		: (U"Enemy AI: TOML (" + GetEnemyAiModeLabel(m_config.enemyAI.mode) + U")");
	m_state.statusMessageTimer = 1.6;
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

void BattleSession::resetEnemyAiAssaultState()
{
	m_state.enemyAiStagingTimer = 0.0;
	m_state.enemyAiAssaultCommitTimer = 0.0;
	m_state.enemyAiAssaultDestination = Vec2::Zero();
	m_state.enemyAiAssaultTargetUnitId.reset();
	m_state.enemyAiDebugReadyUnitCount = 0;
}

void BattleSession::beginTutorialPhase(const TutorialPhase phase, const double timer)
{
	m_state.tutorialPhase = phase;
	m_state.tutorialPhaseTimer = timer;
	m_state.tutorialObjective = GetTutorialObjective(m_config, phase);
}

void BattleSession::spawnTutorialEnemyWave()
{
	if (m_state.tutorialEnemyWaveStarted)
	{
		return;
	}

	m_state.tutorialEnemyWaveStarted = true;
	for (const auto& placement : m_config.tutorial.enemyWaveUnits)
	{
		spawnUnit(placement.owner, placement.archetype, placement.position);
	}
	beginTutorialPhase(TutorialPhase::DefendWave);
	m_state.statusMessage = U"Enemy wave incoming";
	m_state.statusMessageTimer = 2.0;
}

void BattleSession::updateTutorial(const double deltaTime)
{
	if (!(m_state.tutorialActive && m_config.tutorial.enabled) || m_state.winner)
	{
		return;
	}

	if (!m_state.tutorialWorkerUnitId)
	{
		for (const auto& unit : m_state.units)
		{
			if (unit.isAlive && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
			{
				m_state.tutorialWorkerUnitId = unit.id;
				break;
			}
		}
	}

	if (m_state.tutorialPhaseTimer > 0.0)
	{
		m_state.tutorialPhaseTimer = Max(0.0, m_state.tutorialPhaseTimer - deltaTime);
	}

	switch (m_state.tutorialPhase)
	{
	case TutorialPhase::MoveUnit:
		if (m_state.tutorialWorkerUnitId)
		{
			if (const auto* worker = findCachedUnit(*m_state.tutorialWorkerUnitId))
			{
				if (worker->isAlive && (worker->position.distanceFrom(m_config.tutorial.moveTarget) <= m_config.tutorial.moveTargetRadius))
				{
					beginTutorialPhase(TutorialPhase::BuildStructure);
					m_state.statusMessage = U"Now build a Barracks";
					m_state.statusMessageTimer = 2.0;
				}
			}
		}
		break;

	case TutorialPhase::BuildStructure:
		if (HasConstructedPlayerBuilding(m_state, m_config.tutorial.requiredConstruction))
		{
			beginTutorialPhase(TutorialPhase::PrepareDefense, m_config.tutorial.prepareDelay);
			m_state.statusMessage = U"Enemy forces spotted";
			m_state.statusMessageTimer = 2.0;
		}
		break;

	case TutorialPhase::PrepareDefense:
		if (m_state.tutorialPhaseTimer <= 0.0)
		{
			beginTutorialPhase(TutorialPhase::ProduceUnit, m_config.tutorial.enemyWaveDelay);
			m_state.statusMessage = U"Produce a Soldier before the enemy arrives";
			m_state.statusMessageTimer = 2.0;
		}
		break;

	case TutorialPhase::ProduceUnit:
		if (m_state.tutorialProducedUnitCount >= m_config.tutorial.requiredProductionCount)
		{
			constexpr double postProductionGraceTime = 3.0;
			if (m_state.tutorialPhaseTimer > postProductionGraceTime)
			{
				m_state.tutorialPhaseTimer = postProductionGraceTime;
				m_state.statusMessage = U"Production complete. Prepare to intercept.";
				m_state.statusMessageTimer = 2.0;
			}

			if (m_state.tutorialPhaseTimer <= 0.0)
			{
				spawnTutorialEnemyWave();
			}
		}
		else if (m_state.tutorialPhaseTimer <= 0.0)
		{
			spawnTutorialEnemyWave();
		}
		break;

	case TutorialPhase::DefendWave:
		if (m_state.tutorialEnemyWaveStarted && (CountLivingEnemyCombatUnits(m_state) <= 0))
		{
			beginTutorialPhase(TutorialPhase::Completed);
			m_state.winner = Owner::Player;
		}
		break;

	case TutorialPhase::Completed:
	case TutorialPhase::None:
	default:
		break;
	}
}
