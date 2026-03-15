#include "BattleScene.h"
#include "AudioManager.h"
#include "BattleCommandUi.h"
#include "ContinueRunSave.h"
#include "SceneTransition.h"

#include "BattleScenePause.ipp"
#include "BattleSceneResult.ipp"

namespace
{
	[[nodiscard]] Vec2 GetInitialCameraCenter(const BattleState& state)
	{
		for (const auto& unit : state.units)
		{
			if ((unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Base))
			{
				return unit.position;
			}
		}

		return state.worldBounds.center();
	}
}

BattleScene::BattleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_clock{ 1.0 / 60.0 }
{
	PlayBattleBgm();

	if (IsTutorialBattle(getData()))
	{
		m_session.reset(getData().tutorialBattleConfig);
		const Vec2 initialCameraCenter = clampCameraCenter(GetInitialCameraCenter(m_session.state()));
		m_camera.setTargetCenter(initialCameraCenter);
		m_camera.jumpTo(initialCameraCenter, 1.0);
		return;
	}

	if (!getData().runState.isActive)
	{
		BeginNewRun(getData().runState, getData().baseBattleConfig);
	}

	m_session.reset(BuildBattleConfigForRun(getData().baseBattleConfig, getData().runState, getData().rewardCards));
	const Vec2 initialCameraCenter = clampCameraCenter(GetInitialCameraCenter(m_session.state()));
	m_camera.setTargetCenter(initialCameraCenter);
	m_camera.jumpTo(initialCameraCenter, 1.0);
	SaveContinueRun(getData(), ContinueResumeScene::Battle);
}

void BattleScene::update()
{
	if (UpdateSceneTransition(getData(), [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	if (handleResultInput())
	{
		return;
	}

	if (KeyEscape.down())
	{
		togglePause();
		return;
	}

	if (m_isPaused)
	{
		updatePauseMenu();
		return;
	}

	updateActiveBattle();
}

void BattleScene::draw() const
{
	m_renderer.draw(m_session.state(), m_session.config(), getData(), m_camera);
	if (m_session.state().winner)
	{
		DrawBattleResultOverlay(getData(), m_session.state());
	}
	if (m_isPaused)
	{
		drawPauseMenu();
	}

	DrawSceneTransitionOverlay(getData());
}

#include "BattleSceneCamera.ipp"
#include "BattleSceneInput.ipp"
