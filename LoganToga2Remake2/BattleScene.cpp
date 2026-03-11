#include "BattleScene.h"

#include "BattleScenePause.ipp"

BattleScene::BattleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_clock{ 1.0 / 60.0 }
{
	if (!getData().runState.isActive)
	{
		BeginNewRun(getData().runState);
	}

	m_session.reset(BuildBattleConfigForRun(getData().baseBattleConfig, getData().runState, getData().rewardCards));
	m_camera.jumpTo(m_session.state().worldBounds.center(), 1.0);
}

void BattleScene::update()
{
	if (m_session.state().winner)
	{
		m_isPaused = false;
		auto& runState = getData().runState;
		const bool playerWon = (*m_session.state().winner == Owner::Player);
		const bool hasNextBattle = playerWon && ((runState.currentBattleIndex + 1) < runState.totalBattles);
		if (KeyEnter.down())
		{
			if (hasNextBattle)
			{
				runState.pendingRewardCardIds = BuildRewardCardChoices(runState, getData().rewardCards);
				changeScene(U"Reward");
			}
			else
			{
				if (playerWon)
				{
					runState.isCleared = true;
				}
				else
				{
					runState.isFailed = true;
				}
				changeScene(U"Title");
			}
			return;
		}

		if (KeyR.down())
		{
			BeginNewRun(runState);
			changeScene(U"Battle");
			return;
		}
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

	m_camera.update();

	m_inputController.handleFormationInput(m_session);

	const Vec2 cursorScreenPos = Cursor::PosF();
	const Vec2 constructionCursorWorldPos = screenToWorld(cursorScreenPos);
	m_constructionController.handleInput(m_session, constructionCursorWorldPos);

	if (!m_session.state().pendingConstructionArchetype)
	{
		const bool isCursorOnCommandPanel = m_inputController.isCursorOnCommandPanel(m_session, cursorScreenPos);
		const bool handledCommandPanelClick = m_inputController.handleCommandPanelClick(m_session, cursorScreenPos);
		if (isCursorOnCommandPanel)
		{
			resetCameraPan();
		}

		const Vec2 cursorWorldPos = screenToWorld(cursorScreenPos);
		if (!isCursorOnCommandPanel)
		{
			const bool allowClickSelection = updateCameraPan();
			m_inputController.handleSelectionInput(m_session, cursorWorldPos, allowClickSelection);
			m_inputController.handleCommandInput(m_session, cursorWorldPos);
		}
		else if (handledCommandPanelClick)
		{
			m_session.state().isSelecting = false;
			m_session.state().selectionRect = RectF{ 0, 0, 0, 0 };
		}
	}
	else
	{
		resetCameraPan();
	}

	handleProductionInput();

	const size_t fixedSteps = m_clock.beginFrame();
	for (size_t i = 0; i < fixedSteps; ++i)
	{
		m_session.update(m_clock.stepSeconds());
	}
}

void BattleScene::draw() const
{
	m_renderer.draw(m_session.state(), m_session.config(), getData(), m_camera);
	if (m_isPaused)
	{
		drawPauseMenu();
	}
}

#include "BattleSceneCamera.ipp"

bool BattleScene::isProductionSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 1:
		return Key1.down();
	case 2:
		return Key2.down();
	case 3:
		return Key3.down();
	default:
		return false;
	}
}

void BattleScene::handleProductionInput()
{
	if (KeyX.down())
	{
		m_session.cancelLastPlayerProduction();
	}

	for (const auto& slot : m_session.config().playerProductionSlots)
	{
		if (isProductionSlotTriggered(slot.slot))
		{
			m_session.trySpawnPlayerUnit(slot.archetype);
		}
	}
}
