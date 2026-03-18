#include "TitleScene.h"

#include "GameSettings.h"

bool TitleScene::handleDialogInput()
{
	if (m_dataClearAction != DataClearAction::None)
	{
		if (KeyEnter.down() || isButtonClicked(getDataClearDialogYesButtonRect()))
		{
			executeDataClearAction();
			m_dataClearAction = DataClearAction::None;
			return true;
		}

		if (KeyEscape.down() || isButtonClicked(getDataClearDialogNoButtonRect()))
		{
			m_dataClearAction = DataClearAction::None;
		}

		return true;
	}

	if (m_isExitDialogOpen)
	{
		if (KeyEnter.down() || isButtonClicked(getExitDialogYesButtonRect()))
		{
			System::Exit();
			return true;
		}

		if (KeyEscape.down() || isButtonClicked(getExitDialogNoButtonRect()))
		{
			m_isExitDialogOpen = false;
		}

		return true;
	}

	return false;
}

bool TitleScene::handleExitOpenInput()
{
	if (KeyEscape.down())
	{
		m_isExitDialogOpen = true;
		return true;
	}

	if (isButtonClicked(getExitButtonRect()))
	{
		m_isExitDialogOpen = true;
		return true;
	}

	return false;
}

bool TitleScene::handleContinueInput(const bool hasContinue)
{
	auto& data = getData();
	if (!hasContinue || !(KeyEnter.down() || isButtonClicked(getContinueButtonRect())))
	{
		return false;
	}

	ContinueResumeScene resumeScene = ContinueResumeScene::Battle;
	if (LoadContinueRun(data, resumeScene))
	{
		RequestSceneTransition(data, GetContinueResumeSceneName(resumeScene), [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	ClearContinueRunSave();
	refreshContinueState();
	return false;
}

bool TitleScene::handlePrimaryActionInput(const bool hasContinue)
{
	auto& data = getData();

	if (isButtonClicked(getQuickGuideButtonRect(hasContinue)))
	{
		m_isQuickGuideOpen = true;
		return true;
	}

	if ((!hasContinue && KeyEnter.down()) || isButtonClicked(getStartButtonRect(hasContinue)))
	{
		data.battleLaunchMode = BattleLaunchMode::Run;
		BeginNewRun(data.runState, data.baseBattleConfig, false);
		ResetBonusRoomSceneState(data.bonusRoomProgress);
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getTutorialButtonRect(hasContinue)))
	{
		data.battleLaunchMode = BattleLaunchMode::Tutorial;
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	auto& bonusRoomProgress = data.bonusRoomProgress;
	const Array<const BonusRoomDefinition*> viewedRooms = CollectViewedBonusRooms(data.bonusRooms, bonusRoomProgress);
	if (!viewedRooms.isEmpty() && isButtonClicked(getBonusButtonRect(hasContinue)))
	{
		ResetBonusRoomSceneState(bonusRoomProgress);
		bonusRoomProgress.sceneMode = BonusRoomSceneMode::Gallery;
		RequestSceneTransition(data, U"BonusRoom", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	return false;
}

void TitleScene::handleResolutionInput()
{
	auto& data = getData();
	const Array<WindowResolutionPreset> presets =
	{
		WindowResolutionPreset::Small,
		WindowResolutionPreset::Medium,
		WindowResolutionPreset::Large,
	};

	for (size_t i = 0; i < presets.size(); ++i)
	{
		const WindowResolutionPreset preset = presets[i];
		if (!isButtonClicked(getResolutionButtonRect(i)))
		{
			continue;
		}

		data.displaySettings.resolutionPreset = preset;
		ApplyDisplaySettings(data.displaySettings);
		GameSettings::SetPersistedDisplaySettings(data.displaySettings);
	}
}

void TitleScene::handleSaveLocationInput()
{
	if (!isButtonClicked(getSaveLocationButtonRect()))
	{
		return;
	}

	const ContinueRunSaveLocation currentLocation = GetContinueRunSaveLocation();
	const ContinueRunSaveLocation nextLocation = CycleContinueRunSaveLocation(currentLocation);
	if (SetContinueRunSaveLocation(nextLocation))
	{
		GameSettings::MoveGameSettingsToLocation(currentLocation, nextLocation);
		TitleUi::MoveTitleUiLayoutToLocation(currentLocation, nextLocation);
		refreshContinueState();
	}
}

bool TitleScene::handleDataManagementInput(const bool hasContinue)
{
	if (hasContinue && isButtonClicked(getClearContinueRunButtonRect()))
	{
		m_dataClearAction = DataClearAction::ContinueRunSave;
		return true;
	}

	if (isButtonClicked(getClearSettingsButtonRect()))
	{
		m_dataClearAction = DataClearAction::GameSettings;
		return true;
	}

	return false;
}

#ifdef _DEBUG
bool TitleScene::handleDebugInput(const bool hasContinue)
{
	auto& data = getData();

	if (isButtonClicked(getDebugButtonRect(hasContinue)))
	{
		data.battleLaunchMode = BattleLaunchMode::Run;
		BeginNewRun(data.runState, data.baseBattleConfig, true);
		ResetBonusRoomSceneState(data.bonusRoomProgress);
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getTransitionPresetButtonRect()))
	{
		data.sceneTransitionSettings.preset = CycleSceneTransitionPreset(data.sceneTransitionSettings.preset);
		return true;
	}

	if (isButtonClicked(getTitleUiEditorButtonRect()))
	{
		RequestSceneTransition(data, U"TitleUiEditor", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getRewardEditorButtonRect()))
	{
		RequestSceneTransition(data, U"RewardEditor", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getBonusRoomEditorButtonRect()))
	{
		RequestSceneTransition(data, U"BonusRoomEditor", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getMapEditButtonRect()))
	{
		RequestSceneTransition(data, U"MapEdit", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	if (isButtonClicked(getBalanceEditButtonRect()))
	{
		RequestSceneTransition(data, U"BalanceEdit", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return true;
	}

	return false;
}
#endif
