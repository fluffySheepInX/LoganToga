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

void BattleSession::resetEnemyAiAssaultState()
{
	m_state.enemyAiStagingTimer = 0.0;
	m_state.enemyAiAssaultCommitTimer = 0.0;
	m_state.enemyAiAssaultDestination = Vec2::Zero();
	m_state.enemyAiAssaultTargetUnitId.reset();
	m_state.enemyAiDebugReadyUnitCount = 0;
}
