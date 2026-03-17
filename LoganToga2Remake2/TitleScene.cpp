#include "TitleScene.h"

#include "AudioManager.h"
#include "GameSettings.h"

namespace
{
	constexpr double TitleIntroDuration = 1.10;
	constexpr double TitleIntroSeDelay = 0.12;

	[[nodiscard]] MenuButtonStyle GetTitleMenuButtonStyle()
	{
		MenuButtonStyle style;
		style.hoverExpand = 4.0;
		style.hoverBorderThickness = 3.5;
		style.hoverFillColor = ColorF{ 0.30, 0.36, 0.47, 0.99 };
		style.selectedHoverFillColor = ColorF{ 0.40, 0.54, 0.82, 0.99 };
		style.hoverFrameColor = ColorF{ 0.88, 0.94, 1.0, 0.99 };
		return style;
	}
}

TitleScene::TitleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
{
	PlayMenuBgm();
}

void TitleScene::update()
{
	updateIntro();
	if (isIntroPlaying())
	{
		return;
	}

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

	if (isButtonClicked(getExitButtonRect()))
	{
		m_isExitDialogOpen = true;
		return;
	}

	if (hasContinue && (KeyEnter.down() || isButtonClicked(getContinueButtonRect())))
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

	if (isButtonClicked(getQuickGuideButtonRect(hasContinue)))
	{
		m_isQuickGuideOpen = true;
		return;
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
		return;
	}

	if (isButtonClicked(getTutorialButtonRect(hasContinue)))
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
	if (!viewedRooms.isEmpty() && isButtonClicked(getBonusButtonRect(hasContinue)))
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
			TitleUi::MoveTitleUiLayoutToLocation(currentLocation, nextLocation);
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
		return;
	}

	if (isButtonClicked(getTransitionPresetButtonRect()))
	{
		data.sceneTransitionSettings.preset = CycleSceneTransitionPreset(data.sceneTransitionSettings.preset);
		return;
	}

	if (isButtonClicked(getTitleUiEditorButtonRect()))
	{
		RequestSceneTransition(data, U"TitleUiEditor", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (isButtonClicked(getRewardEditorButtonRect()))
	{
		RequestSceneTransition(data, U"RewardEditor", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
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

void TitleScene::updateIntro()
{
	if (!m_hasPlayedIntroSe && m_introElapsed >= TitleIntroSeDelay)
	{
		PlayTitleIntroSe();
		m_hasPlayedIntroSe = true;
	}

	if (m_introElapsed >= TitleIntroDuration)
	{
		m_introElapsed = TitleIntroDuration;
		return;
	}

	m_introElapsed += Scene::DeltaTime();
	if (m_introElapsed > TitleIntroDuration)
	{
		m_introElapsed = TitleIntroDuration;
	}
}

bool TitleScene::isIntroPlaying() const
{
	return m_introElapsed < TitleIntroDuration;
}

double TitleScene::getIntroProgress() const
{
	return Clamp(m_introElapsed / TitleIntroDuration, 0.0, 1.0);
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
			Localization::InitializeLanguage(settings.language);
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
	DrawMenuButton(rect, label, font, selected, GetTitleMenuButtonStyle());
}

RectF TitleScene::getPanelRect()
{
	return TitleUi::GetTitleUiLayout().panelRect;
}

RectF TitleScene::getMenuButtonRect(const double yOffset)
{
	return RectF{ Scene::CenterF().movedBy(-110, yOffset), 220, 36 };
}

RectF TitleScene::getContinueButtonRect()
{
	return TitleUi::GetTitleUiLayout().continueButtonRect;
}

RectF TitleScene::getTutorialButtonRect(const bool hasContinue)
{
	return TitleUi::GetTutorialButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getQuickGuideButtonRect(const bool hasContinue)
{
	return TitleUi::GetQuickGuideButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getStartButtonRect(const bool hasContinue)
{
	return TitleUi::GetStartButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getBonusButtonRect(const bool hasContinue)
{
	return TitleUi::GetBonusButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getDebugButtonRect(const bool hasContinue)
{
	return TitleUi::GetDebugButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getTitleUiEditorButtonRect()
{
	return TitleUi::GetTitleUiLayout().titleUiEditorButtonRect;
}

RectF TitleScene::getQuickGuidePanelRect()
{
	return TitleUi::GetTitleUiLayout().quickGuidePanelRect;
}

RectF TitleScene::getQuickGuideTutorialButtonRect()
{
	return TitleUi::GetTitleUiLayout().quickGuideTutorialButtonRect;
}

RectF TitleScene::getQuickGuideCloseButtonRect()
{
	return TitleUi::GetTitleUiLayout().quickGuideCloseButtonRect;
}

RectF TitleScene::getDataClearDialogRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogRect;
}

RectF TitleScene::getDataClearDialogYesButtonRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogYesButtonRect;
}

RectF TitleScene::getDataClearDialogNoButtonRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogNoButtonRect;
}

RectF TitleScene::getExitDialogRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogRect;
}

RectF TitleScene::getExitDialogYesButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogYesButtonRect;
}

RectF TitleScene::getExitDialogNoButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogNoButtonRect;
}

Vec2 TitleScene::getResolutionLabelPos()
{
	return TitleUi::GetTitleUiLayout().resolutionLabelPos;
}

RectF TitleScene::getResolutionButtonRect(const size_t index)
{
	return TitleUi::GetResolutionButtonRect(TitleUi::GetTitleUiLayout(), index);
}

Vec2 TitleScene::getSaveLocationLabelPos()
{
	return TitleUi::GetTitleUiLayout().saveLocationLabelPos;
}

RectF TitleScene::getSaveLocationButtonRect()
{
	return TitleUi::GetTitleUiLayout().saveLocationButtonRect;
}

RectF TitleScene::getClearContinueRunButtonRect()
{
	return TitleUi::GetTitleUiLayout().clearContinueRunButtonRect;
}

RectF TitleScene::getClearSettingsButtonRect()
{
	return TitleUi::GetTitleUiLayout().clearSettingsButtonRect;
}

RectF TitleScene::getExitButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitButtonRect;
}

RectF TitleScene::getMapEditButtonRect()
{
	return TitleUi::GetTitleUiLayout().mapEditButtonRect;
}

RectF TitleScene::getTransitionPresetButtonRect()
{
	return TitleUi::GetTitleUiLayout().transitionPresetButtonRect;
}

RectF TitleScene::getBalanceEditButtonRect()
{
	return TitleUi::GetTitleUiLayout().balanceEditButtonRect;
}

RectF TitleScene::getRewardEditorButtonRect()
{
	return TitleUi::GetTitleUiLayout().titleUiEditorButtonRect.movedBy(0, 38);
}
