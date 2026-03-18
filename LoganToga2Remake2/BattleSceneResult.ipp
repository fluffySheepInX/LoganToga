#include "BattleTutorialText.h"

namespace
{
	[[nodiscard]] bool IsTutorialBattle(const GameData& data)
	{
		return data.battleLaunchMode == BattleLaunchMode::Tutorial;
	}

	[[nodiscard]] bool HasAvailableBonusRoom(const GameData& data)
	{
		return !BuildBonusRoomChoices(data.bonusRoomProgress, data.bonusRooms).isEmpty();
	}

	struct BattleResultOverlayContent
	{
		String title;
		String subtitle;
		String enterAction;
		String retryAction;
		String footer;
		ColorF titleColor{ Palette::White };
	};

	[[nodiscard]] BattleResultOverlayContent BuildBattleResultOverlayContent(const GameData& data, const BattleState& state)
	{
		const bool playerWon = state.winner && (*state.winner == Owner::Player);
		if (IsTutorialBattle(data))
		{
			return BattleResultOverlayContent{
             .title = BattleTutorialText::GetResultTitle(playerWon),
				.subtitle = BattleTutorialText::GetResultSubtitle(playerWon),
				.enterAction = BattleTutorialText::GetResultEnterAction(),
				.retryAction = BattleTutorialText::GetResultRetryAction(),
				.footer = BattleTutorialText::GetResultFooter(),
				.titleColor = playerWon ? ColorF{ 1.0, 0.92, 0.55 } : ColorF{ 1.0, 0.66, 0.66 }
			};
		}

		const auto& runState = data.runState;
		const bool hasNextBattle = playerWon && ((runState.currentBattleIndex + 1) < runState.totalBattles);
		const bool hasBonusRoom = playerWon && !hasNextBattle && HasAvailableBonusRoom(data);
		BattleResultOverlayContent content;
		content.title = playerWon ? (hasNextBattle ? U"Victory" : U"Run Cleared") : U"Defeat";
		content.subtitle = s3d::Format(
			U"Battle ",
			runState.currentBattleIndex + 1,
			U"/",
			runState.totalBattles,
			U"   Cards selected: ",
			runState.selectedCardIds.size());
		content.retryAction = U"R: Start New Run";
		content.titleColor = playerWon ? ColorF{ 1.0, 0.92, 0.55 } : ColorF{ 1.0, 0.66, 0.66 };

		if (!playerWon)
		{
			content.enterAction = U"Enter: Return to Title";
			content.footer = U"This run has ended";
			return content;
		}

		if (hasNextBattle)
		{
			content.enterAction = U"Enter: Choose Reward";
			content.footer = U"Choose 1 reward card before the next battle";
			return content;
		}

		if (hasBonusRoom)
		{
			content.enterAction = U"Enter: Bonus Room";
			content.footer = U"Choose 1 bonus room after clearing the run";
			return content;
		}

		content.enterAction = U"Enter: Return to Title";
		content.footer = U"All bonus rooms already viewed. Run complete";
		return content;
	}

	void DrawBattleResultOverlay(const GameData& data, const BattleState& state)
	{
		const BattleResultOverlayContent content = BuildBattleResultOverlayContent(data, state);
		const bool playerWon = state.winner && (*state.winner == Owner::Player);
		const double pulse = (0.5 + (0.5 * Math::Sin(Scene::Time() * 6.0)));
		const RectF panelRect{ Arg::center = Scene::CenterF().movedBy(0, -12), 760, 250 };

		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.36 });
		if (!playerWon)
		{
			Scene::Rect().draw(ColorF{ 0.22, 0.02, 0.03, 0.18 + (0.10 * pulse) });
			Scene::Rect().stretched(-16).drawFrame(10, 0, ColorF{ 0.92, 0.18, 0.18, 0.16 + (0.18 * pulse) });
		}
		const ColorF panelFillColor = playerWon
			? ColorF{ 0.05, 0.07, 0.11, 0.95 }
			: ColorF{ 0.11, 0.05, 0.07, 0.96 };
		const ColorF panelFrameColor = playerWon
			? ColorF{ 0.34, 0.48, 0.82, 0.98 }
			: ColorF{ 0.80, 0.20, 0.24, 0.84 + (0.12 * pulse) };
		RoundRect{ panelRect, 18 }.draw(panelFillColor);
		RoundRect{ panelRect, 18 }.drawFrame(3, 0, panelFrameColor);
		data.titleFont(content.title).drawAt(panelRect.center().movedBy(0, -76), content.titleColor);
		data.uiFont(content.subtitle).drawAt(panelRect.center().movedBy(0, -28), Palette::White);
		data.uiFont(content.enterAction).drawAt(panelRect.center().movedBy(0, 24), Palette::White);
		data.uiFont(content.retryAction).drawAt(panelRect.center().movedBy(0, 60), ColorF{ 0.86, 0.92, 1.0 });
		data.smallFont(content.footer).drawAt(panelRect.center().movedBy(0, 100), Palette::Yellow);
	}
}

bool BattleScene::handleResultInput()
{
	if (!m_session.state().winner)
	{
		return false;
	}

	m_isPaused = false;
	auto& data = getData();
	if (IsTutorialBattle(data))
	{
		if (KeyEnter.down())
		{
			data.battleLaunchMode = BattleLaunchMode::Run;
			RequestSceneTransition(data, U"Title", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
		}
		else if (KeyR.down())
		{
			RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
		}

		return true;
	}

	auto& runState = data.runState;
	const bool playerWon = (*m_session.state().winner == Owner::Player);
	const bool hasNextBattle = playerWon && ((runState.currentBattleIndex + 1) < runState.totalBattles);
	if (KeyEnter.down())
	{
		if (hasNextBattle)
		{
			runState.pendingRewardCardIds = BuildRewardCardChoices(runState, data.rewardCards);
			SaveContinueRun(data, ContinueResumeScene::Reward);
			RequestSceneTransition(data, U"Reward", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
		}
		else if (playerWon)
		{
			runState.isCleared = true;
			runState.isActive = false;
			if (PrepareBonusRoomSelection(data.bonusRoomProgress, data.bonusRooms))
			{
				SaveContinueRun(data, ContinueResumeScene::BonusRoom);
				RequestSceneTransition(data, U"BonusRoom", [this](const String& sceneName)
				{
					changeScene(sceneName);
				});
			}
			else
			{
				ClearContinueRunSave();
				RequestSceneTransition(data, U"Title", [this](const String& sceneName)
				{
					changeScene(sceneName);
				});
			}
		}
		else
		{
			runState.isFailed = true;
			runState.isActive = false;
			ResetBonusRoomSceneState(data.bonusRoomProgress);
			ClearContinueRunSave();
			RequestSceneTransition(data, U"Title", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
		}

		return true;
	}

	if (KeyR.down())
	{
		BeginNewRun(runState, data.baseBattleConfig);
		ResetBonusRoomSceneState(data.bonusRoomProgress);
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}

	return true;
}
