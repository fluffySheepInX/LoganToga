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
}

void BattleSession::resetEnemyAiAssaultState()
{
	m_state.enemyAiStagingTimer = 0.0;
	m_state.enemyAiAssaultCommitTimer = 0.0;
	m_state.enemyAiAssaultDestination = Vec2::Zero();
	m_state.enemyAiAssaultTargetUnitId.reset();
	m_state.enemyAiDebugReadyUnitCount = 0;
}
