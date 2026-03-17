#include "WindowChromeAddon.h"

void WindowChromeAddon::handleCloseButton()
{
	if (getCloseButtonRect().leftClicked())
	{
		openCloseDialog();
	}
}

void WindowChromeAddon::handleLanguageButton()
{
	if (getLanguageButtonRect().leftClicked())
	{
		m_isLanguageButtonHintActive = false;
		m_languageButtonHintElapsed = 0.0;
		m_isLanguagePanelOpen = !m_isLanguagePanelOpen;
		if (m_isLanguagePanelOpen)
		{
			m_isAudioPanelOpen = false;
		}
		return;
	}

	if (!m_isLanguagePanelOpen)
	{
		return;
	}

	const auto& definitions = Localization::GetLanguageDefinitions();
	for (size_t i = 0; i < definitions.size(); ++i)
	{
		if (!getLanguageOptionRect(i).leftClicked())
		{
			continue;
		}

		Localization::SetLanguage(definitions[i].language);
		GameSettings::SetPersistedLanguage(definitions[i].language);
		m_isLanguagePanelOpen = false;
		return;
	}

	if (MouseL.down() && !getLanguagePanelRect().mouseOver())
	{
		m_isLanguagePanelOpen = false;
	}
}

void WindowChromeAddon::openCloseDialog()
{
	m_isCloseDialogOpen = true;
	m_isLanguagePanelOpen = false;
	m_isAudioPanelOpen = false;
	m_dragStartWindow.reset();
}

void WindowChromeAddon::handleCloseDialog()
{
	if (KeyEnter.down() || getCloseDialogYesButtonRect().leftClicked())
	{
		s3d::System::Exit();
		return;
	}

	if (KeyEscape.down() || getCloseDialogNoButtonRect().leftClicked())
	{
		m_isCloseDialogOpen = false;
	}
}

void WindowChromeAddon::handleFullscreenButton()
{
	if (!getFullscreenButtonRect().leftClicked())
	{
		return;
	}

	const bool isFullscreen = Window::GetState().fullscreen;
	Window::SetFullscreen(!isFullscreen);
	m_settingsDirty = true;

	if (isFullscreen)
	{
		Window::SetStyle(s3d::WindowStyle::Frameless);
	}
}

void WindowChromeAddon::handleAudioButton()
{
	if (getAudioButtonRect().leftClicked())
	{
		m_isAudioPanelOpen = !m_isAudioPanelOpen;
		if (m_isAudioPanelOpen)
		{
			m_isLanguagePanelOpen = false;
		}
		return;
	}

	if (m_isAudioPanelOpen && MouseL.down() && !getVolumePanelRect().mouseOver())
	{
		m_isAudioPanelOpen = false;
	}
}

double WindowChromeAddon::getBusVolume(const s3d::MixBus bus)
{
	return Clamp(s3d::GlobalAudio::BusGetVolume(bus), 0.0, 1.0);
}

double WindowChromeAddon::getMasterVolume()
{
	return Clamp(s3d::GlobalAudio::GetVolume(), 0.0, 1.0);
}

void WindowChromeAddon::setBusVolume(const s3d::MixBus bus, const double volume)
{
	s3d::GlobalAudio::BusSetVolume(bus, Clamp(volume, 0.0, 1.0));
}

void WindowChromeAddon::setMasterVolume(const double volume)
{
	s3d::GlobalAudio::SetVolume(Clamp(volume, 0.0, 1.0));
}

void WindowChromeAddon::handleVolumeControls()
{
	if (!m_isAudioPanelOpen)
	{
		return;
	}

	handleMasterVolumeRowControls(0);
	handleVolumeRowControls(1, BgmBus);
	handleVolumeRowControls(2, SeBus);
}

void WindowChromeAddon::handleMasterVolumeRowControls(const int32 rowIndex)
{
	if (getVolumeDecreaseButtonRect(rowIndex).leftClicked())
	{
		setPersistedMasterVolume(getMasterVolume() - m_volumeStep);
	}

	if (getVolumeIncreaseButtonRect(rowIndex).leftClicked())
	{
		setPersistedMasterVolume(getMasterVolume() + m_volumeStep);
	}

	const RectF gaugeRect = getVolumeGaugeRect(rowIndex);
	if (MouseL.pressed() && gaugeRect.mouseOver())
	{
		setPersistedMasterVolume((Cursor::PosF().x - gaugeRect.x) / gaugeRect.w);
	}
}

void WindowChromeAddon::handleVolumeRowControls(const int32 rowIndex, const s3d::MixBus bus)
{
	if (getVolumeDecreaseButtonRect(rowIndex).leftClicked())
	{
		setPersistedBusVolume(bus, getBusVolume(bus) - m_volumeStep);
	}

	if (getVolumeIncreaseButtonRect(rowIndex).leftClicked())
	{
		setPersistedBusVolume(bus, getBusVolume(bus) + m_volumeStep);
	}

	const RectF gaugeRect = getVolumeGaugeRect(rowIndex);
	if (MouseL.pressed() && gaugeRect.mouseOver())
	{
		setPersistedBusVolume(bus, (Cursor::PosF().x - gaugeRect.x) / gaugeRect.w);
	}
}

void WindowChromeAddon::setPersistedMasterVolume(const double volume)
{
	const double nextVolume = Clamp(volume, 0.0, 1.0);
	if (getMasterVolume() == nextVolume)
	{
		return;
	}

	setMasterVolume(nextVolume);
	m_settingsDirty = true;
}

void WindowChromeAddon::setPersistedBusVolume(const s3d::MixBus bus, const double volume)
{
	const double nextVolume = Clamp(volume, 0.0, 1.0);
	if (getBusVolume(bus) == nextVolume)
	{
		return;
	}

	setBusVolume(bus, nextVolume);
	m_settingsDirty = true;
}

void WindowChromeAddon::flushPersistedSettings()
{
	if (!(m_settingsDirty && !MouseL.pressed()))
	{
		return;
	}

	if (GameSettings::SetPersistedWindowAndAudioSettings(Window::GetState().fullscreen, getMasterVolume(), getBusVolume(BgmBus), getBusVolume(SeBus)))
	{
		m_settingsDirty = false;
	}
}

void WindowChromeAddon::handleWindowDrag()
{
	const RectF dragRect = getDragRect();
	if (!m_dragStartWindow && MouseL.down() && dragRect.mouseOver() && !isPointerOnInteractiveChrome())
	{
		m_dragStartWindow = std::pair{ Cursor::ScreenPos(), Window::GetState().bounds.pos };
	}

	if (!m_dragStartWindow)
	{
		return;
	}

	if (MouseL.pressed())
	{
		Window::SetPos(m_dragStartWindow->second + (Cursor::ScreenPos() - m_dragStartWindow->first));
	}
	else
	{
		m_dragStartWindow.reset();
	}
}

bool WindowChromeAddon::isPointerOnInteractiveChrome() const
{
	if (getCloseButtonRect().mouseOver() || getFullscreenButtonRect().mouseOver() || getAudioButtonRect().mouseOver() || getLanguageButtonRect().mouseOver())
	{
		return true;
	}

	return ((m_isAudioPanelOpen && getVolumePanelRect().mouseOver())
		|| (m_isLanguagePanelOpen && getLanguagePanelRect().mouseOver()));
}
