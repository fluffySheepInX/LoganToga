#include "TitleScene.h"

#include "AudioManager.h"
#include "GameSettings.h"

TitleScene::TitleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
{
	PlayMenuBgm();
}

void TitleScene::update()
{
	if (UpdateSceneTransition(getData(), [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	refreshContinueState();

	auto& data = getData();
	const bool hasContinue = m_hasContinue;
	const double continueButtonOffset = 120;
	const double tutorialButtonOffset = hasContinue ? 172 : 120;
	const double quickGuideButtonOffset = hasContinue ? 224 : 172;
	const double startButtonOffset = hasContinue ? 276 : 224;
	const double bonusButtonOffset = hasContinue ? 328 : 276;
	const double debugButtonOffset = hasContinue ? 380 : 328;

	if (m_isQuickGuideOpen)
	{
		updateQuickGuide();
		return;
	}

	if (m_dataClearAction != DataClearAction::None)
	{
		if (KeyEnter.down() || isButtonClicked(getDataClearDialogYesButtonRect()))
		{
			executeDataClearAction();
			m_dataClearAction = DataClearAction::None;
			return;
		}

		if (KeyEscape.down() || isButtonClicked(getDataClearDialogNoButtonRect()))
		{
			m_dataClearAction = DataClearAction::None;
		}

		return;
	}

	if (m_isExitDialogOpen)
	{
		if (KeyEnter.down() || isButtonClicked(getExitDialogYesButtonRect()))
		{
			System::Exit();
			return;
		}

		if (KeyEscape.down() || isButtonClicked(getExitDialogNoButtonRect()))
		{
			m_isExitDialogOpen = false;
		}

		return;
	}

	if (KeyEscape.down())
	{
		m_isExitDialogOpen = true;
		return;
	}

	if (hasContinue && (KeyEnter.down() || isButtonClicked(getMenuButtonRect(continueButtonOffset))))
	{
		ContinueResumeScene resumeScene = ContinueResumeScene::Battle;
		if (LoadContinueRun(data, resumeScene))
		{
			RequestSceneTransition(data, GetContinueResumeSceneName(resumeScene), [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		ClearContinueRunSave();
		refreshContinueState();
	}

	if (isButtonClicked(getMenuButtonRect(quickGuideButtonOffset)))
	{
		m_isQuickGuideOpen = true;
		return;
	}

	if ((!hasContinue && KeyEnter.down()) || isButtonClicked(getMenuButtonRect(startButtonOffset)))
	{
		data.battleLaunchMode = BattleLaunchMode::Run;
		BeginNewRun(data.runState, data.baseBattleConfig, false);
		ResetBonusRoomSceneState(data.bonusRoomProgress);
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (isButtonClicked(getMenuButtonRect(tutorialButtonOffset)))
	{
		data.battleLaunchMode = BattleLaunchMode::Tutorial;
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	auto& bonusRoomProgress = data.bonusRoomProgress;
	const Array<const BonusRoomDefinition*> viewedRooms = CollectViewedBonusRooms(data.bonusRooms, bonusRoomProgress);
	if (!viewedRooms.isEmpty() && isButtonClicked(getMenuButtonRect(bonusButtonOffset)))
	{
		ResetBonusRoomSceneState(bonusRoomProgress);
		bonusRoomProgress.sceneMode = BonusRoomSceneMode::Gallery;
		RequestSceneTransition(data, U"BonusRoom", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

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

	if (isButtonClicked(getSaveLocationButtonRect()))
	{
		const ContinueRunSaveLocation currentLocation = GetContinueRunSaveLocation();
		const ContinueRunSaveLocation nextLocation = CycleContinueRunSaveLocation(currentLocation);
		if (SetContinueRunSaveLocation(nextLocation))
		{
			GameSettings::MoveGameSettingsToLocation(currentLocation, nextLocation);
			refreshContinueState();
		}
	}

	if (hasContinue && isButtonClicked(getClearContinueRunButtonRect()))
	{
		m_dataClearAction = DataClearAction::ContinueRunSave;
		return;
	}

	if (isButtonClicked(getClearSettingsButtonRect()))
	{
		m_dataClearAction = DataClearAction::GameSettings;
		return;
	}

#ifdef _DEBUG
	if (isButtonClicked(getMenuButtonRect(debugButtonOffset)))
	{
		data.battleLaunchMode = BattleLaunchMode::Run;
		BeginNewRun(data.runState, data.baseBattleConfig, true);
		ResetBonusRoomSceneState(data.bonusRoomProgress);
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (isButtonClicked(getTransitionPresetButtonRect()))
	{
		data.sceneTransitionSettings.preset = CycleSceneTransitionPreset(data.sceneTransitionSettings.preset);
		return;
	}

	if (isButtonClicked(getMapEditButtonRect()))
	{
		RequestSceneTransition(data, U"MapEdit", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (isButtonClicked(getBalanceEditButtonRect()))
	{
		RequestSceneTransition(data, U"BalanceEdit", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}
#endif
}

void TitleScene::updateQuickGuide()
{
	if (KeyEscape.down() || isButtonClicked(getQuickGuideCloseButtonRect()))
	{
		m_isQuickGuideOpen = false;
		return;
	}

	if (isButtonClicked(getQuickGuideTutorialButtonRect()))
	{
		m_isQuickGuideOpen = false;
		auto& data = getData();
		data.battleLaunchMode = BattleLaunchMode::Tutorial;
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}
}

void TitleScene::refreshContinueState()
{
	if (!HasContinueRunSave())
	{
		m_continuePreview.reset();
		m_hasContinue = false;
		return;
	}

	m_continuePreview = LoadContinueRunPreview();
	if (!m_continuePreview)
	{
		ClearContinueRunSave();
	}

	m_hasContinue = m_continuePreview.has_value();
}

void TitleScene::executeDataClearAction()
{
	auto& data = getData();

	switch (m_dataClearAction)
	{
	case DataClearAction::ContinueRunSave:
		ClearContinueRunSave();
		data.runState = {};
		data.bonusRoomProgress = {};
		refreshContinueState();
		return;

	case DataClearAction::GameSettings:
		if (GameSettings::ClearGameSettings())
		{
			const PersistentGameSettings settings = GameSettings::GetGameSettings();
			data.displaySettings = settings.displaySettings;
			ApplyDisplaySettings(data.displaySettings);
			s3d::GlobalAudio::SetVolume(settings.masterVolume);
			s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::BgmBus, settings.bgmVolume);
			s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::SeBus, settings.seVolume);
			Window::SetFullscreen(settings.fullscreen);
		}
		return;

	case DataClearAction::None:
	default:
		return;
	}
}

bool TitleScene::isButtonClicked(const RectF& rect)
{
	return IsMenuButtonClicked(rect);
}

void TitleScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}

RectF TitleScene::getPanelRect()
{
	const Vec2 panelSize{ 1040, 520 };
	return RectF{ Arg::center = Scene::CenterF(), panelSize };
}

RectF TitleScene::getMenuButtonRect(const double yOffset)
{
	return RectF{ Scene::CenterF().movedBy(-110, yOffset), 220, 36 };
}

RectF TitleScene::getQuickGuidePanelRect()
{
	return RectF{ Arg::center = Scene::CenterF(), 860, 520 };
}

RectF TitleScene::getQuickGuideTutorialButtonRect()
{
	const RectF panel = getQuickGuidePanelRect();
	return RectF{ Arg::center(panel.center().movedBy(-120, 168)), 200, 40 };
}

RectF TitleScene::getQuickGuideCloseButtonRect()
{
	const RectF panel = getQuickGuidePanelRect();
	return RectF{ Arg::center(panel.center().movedBy(120, 168)), 200, 40 };
}

RectF TitleScene::getDataClearDialogRect()
{
	return RectF{ Arg::center = Scene::CenterF(), 520, 210 };
}

RectF TitleScene::getDataClearDialogYesButtonRect()
{
	const RectF dialog = getDataClearDialogRect();
	return RectF{ Arg::center(dialog.center().movedBy(-110, 62)), 160, 40 };
}

RectF TitleScene::getDataClearDialogNoButtonRect()
{
	const RectF dialog = getDataClearDialogRect();
	return RectF{ Arg::center(dialog.center().movedBy(110, 62)), 160, 40 };
}

RectF TitleScene::getExitDialogRect()
{
	return RectF{ Arg::center = Scene::CenterF(), 420, 180 };
}

RectF TitleScene::getExitDialogYesButtonRect()
{
	const RectF dialog = getExitDialogRect();
	return RectF{ Arg::center(dialog.center().movedBy(-90, 44)), 140, 40 };
}

RectF TitleScene::getExitDialogNoButtonRect()
{
	const RectF dialog = getExitDialogRect();
	return RectF{ Arg::center(dialog.center().movedBy(90, 44)), 140, 40 };
}

Vec2 TitleScene::getResolutionLabelPos()
{
	return Scene::CenterF().movedBy(-150, 250);
}

RectF TitleScene::getResolutionButtonRect(const size_t index)
{
	return RectF{ getResolutionLabelPos().movedBy(100 + (index * 108), -4), 96, 32 };
}

Vec2 TitleScene::getSaveLocationLabelPos()
{
	return Scene::CenterF().movedBy(250, 250);
}

RectF TitleScene::getSaveLocationButtonRect()
{
	return RectF{ getSaveLocationLabelPos().movedBy(112, -4), 180, 32 };
}

RectF TitleScene::getClearContinueRunButtonRect()
{
	const RectF panel = getPanelRect();
	return RectF{ Arg::center(panel.center().movedBy(-92, 212)), 170, 32 };
}

RectF TitleScene::getClearSettingsButtonRect()
{
	const RectF panel = getPanelRect();
	return RectF{ Arg::center(panel.center().movedBy(92, 212)), 170, 32 };
}

RectF TitleScene::getMapEditButtonRect()
{
	const RectF panel = getPanelRect();
	return RectF{ panel.x + panel.w - 150, panel.y + 18, 128, 30 };
}

RectF TitleScene::getTransitionPresetButtonRect()
{
	const RectF panel = getPanelRect();
	return RectF{ panel.x + panel.w - 214, panel.y + 94, 192, 30 };
}

RectF TitleScene::getBalanceEditButtonRect()
{
	const RectF panel = getPanelRect();
	return RectF{ panel.x + panel.w - 150, panel.y + 56, 128, 30 };
}
