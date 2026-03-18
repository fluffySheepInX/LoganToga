#include "TitleScene.h"

#include "AudioManager.h"
#include "GameSettings.h"

namespace
{
	constexpr double TitleIntroDuration = 1.10;
	constexpr double TitleIntroSeDelay = 0.12;
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

	const bool hasContinue = m_hasContinue;
	if (m_isQuickGuideOpen)
	{
		updateQuickGuide();
		return;
	}

	if (handleDialogInput())
	{
		return;
	}

	if (handleExitOpenInput())
	{
		return;
	}

	if (handleContinueInput(hasContinue))
	{
		return;
	}

	if (handlePrimaryActionInput(hasContinue))
	{
		return;
	}

	handleResolutionInput();
	handleSaveLocationInput();

	if (handleDataManagementInput(hasContinue))
	{
		return;
	}

#ifdef _DEBUG
	if (handleDebugInput(hasContinue))
	{
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
