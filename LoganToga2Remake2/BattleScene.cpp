#include "BattleScene.h"
#include "BattleCommandUi.h"
#include "ContinueRunSave.h"

#include "BattleScenePause.ipp"

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
	if (!getData().runState.isActive)
	{
		BeginNewRun(getData().runState);
	}

	m_session.reset(BuildBattleConfigForRun(getData().baseBattleConfig, getData().runState, getData().rewardCards));
	const Vec2 initialCameraCenter = clampCameraCenter(GetInitialCameraCenter(m_session.state()));
	m_camera.setTargetCenter(initialCameraCenter);
	m_camera.jumpTo(initialCameraCenter, 1.0);
	SaveContinueRun(getData(), ContinueResumeScene::Battle);
}

void BattleScene::update()
{
	if (m_session.state().winner)
	{
		m_isPaused = false;
		auto& data = getData();
		auto& runState = data.runState;
		const bool playerWon = (*m_session.state().winner == Owner::Player);
		const bool hasNextBattle = playerWon && ((runState.currentBattleIndex + 1) < runState.totalBattles);
		if (KeyEnter.down())
		{
			if (hasNextBattle)
			{
				runState.pendingRewardCardIds = BuildRewardCardChoices(runState, data.rewardCards);
				SaveContinueRun(data, ContinueResumeScene::Reward);
				changeScene(U"Reward");
			}
			else
			{
				if (playerWon)
				{
					runState.isCleared = true;
					runState.isActive = false;
					if (PrepareBonusRoomSelection(data.bonusRoomProgress, data.bonusRooms))
					{
						SaveContinueRun(data, ContinueResumeScene::BonusRoom);
						changeScene(U"BonusRoom");
					}
					else
					{
						ClearContinueRunSave();
						changeScene(U"Title");
					}
				}
				else
				{
					runState.isFailed = true;
					runState.isActive = false;
					ResetBonusRoomSceneState(data.bonusRoomProgress);
					ClearContinueRunSave();
					changeScene(U"Title");
				}
			}
			return;
		}

		if (KeyR.down())
		{
			BeginNewRun(runState);
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			SaveContinueRun(data, ContinueResumeScene::Battle);
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

	const Vec2 cursorScreenPos = Cursor::PosF();
	const Vec2 constructionCursorWorldPos = screenToWorld(cursorScreenPos);
	m_constructionController.handleInput(m_session, constructionCursorWorldPos);

	if (!(m_session.state().pendingConstructionArchetype || m_session.state().pendingRepairTargeting))
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

bool BattleScene::isCommandSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 1:
		return Key1.down();
	case 2:
		return Key2.down();
	case 3:
		return Key3.down();
	case 4:
		return Key4.down();
	case 5:
		return Key5.down();
	case 6:
		return Key6.down();
	case 7:
		return Key7.down();
	case 8:
		return Key8.down();
	case 9:
		return Key9.down();
	case 0:
		return Key0.down();
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

	for (const auto& command : CollectCommandEntries(m_session.state(), m_session.config()))
	{
		if (!isCommandSlotTriggered(command.slot) || !command.isEnabled)
		{
			continue;
		}

		switch (command.kind)
		{
		case CommandKind::Production:
			m_session.trySpawnPlayerUnit(command.archetype);
			return;
		case CommandKind::Repair:
			m_session.state().pendingConstructionArchetype.reset();
			m_session.state().pendingRepairTargeting = true;
			m_session.state().isSelecting = false;
			m_session.state().selectionRect = RectF{ 0, 0, 0, 0 };
			return;
		case CommandKind::Upgrade:
			if (command.turretUpgradeType)
			{
				m_session.tryUpgradeSelectedTurret(*command.turretUpgradeType);
			}
			return;
		default:
			break;
		}
	}
}
