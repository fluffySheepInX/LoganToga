#include "BattleSession.h"

#include "BattleUiText.h"

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

}

void BattleSession::toggleEnemyAiDebugPanel()
{
	if (!(IsEnemyAiDebugBuildEnabled() && m_config.debug.enableEnemyAiSwitcher))
	{
		return;
	}

	m_state.enemyAiDebugPanelVisible = !m_state.enemyAiDebugPanelVisible;
	m_state.statusMessage = m_state.enemyAiDebugPanelVisible
        ? Localization::GetText(U"battle.enemy_ai_debug.panel_on")
		: Localization::GetText(U"battle.enemy_ai_debug.panel_off");
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
      ? Localization::FormatText(U"battle.enemy_ai_debug.status_override", BattleUiText::GetEnemyAiModeLabel(*mode))
		: Localization::FormatText(U"battle.enemy_ai_debug.status_toml", BattleUiText::GetEnemyAiModeLabel(m_config.enemyAI.mode));
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
