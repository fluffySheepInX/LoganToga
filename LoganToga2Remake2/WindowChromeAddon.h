#pragma once

#include "Remake2Common.h"
#include <utility>

class WindowChromeAddon : public s3d::IAddon
{
public:
	static constexpr s3d::StringView AddonName = U"WindowChromeAddon";
	static constexpr s3d::MixBus BgmBus = s3d::MixBus::Index1;
	static constexpr s3d::MixBus SeBus = s3d::MixBus::Index2;

	static void Configure(const String& title, const ColorF& frameColor = ColorF{ 0.32, 0.58, 0.92 }, const ColorF& glowColor = ColorF{ 0.92, 0.56, 0.78 })
	{
		if (auto* addon = s3d::Addon::GetAddon<WindowChromeAddon>(AddonName))
		{
			addon->m_title = title;
			addon->m_frameColor = frameColor;
			addon->m_glowColor = glowColor;
			Window::SetStyle(s3d::WindowStyle::Frameless);
			addon->ensureTextures(Scene::Size());
		}
	}

private:
	bool update() override
	{
		if (!Window::GetState().fullscreen && (Window::GetStyle() != s3d::WindowStyle::Frameless))
		{
			Window::SetStyle(s3d::WindowStyle::Frameless);
		}

		ensureTextures(Scene::Size());

		if (m_isCloseDialogOpen)
		{
			handleCloseDialog();
			return true;
		}

		handleAudioButton();
		handleFullscreenButton();
		handleVolumeControls();
		handleCloseButton();
		handleWindowDrag();
		return true;
	}

	void draw() const override
	{
		drawGlow();
		drawChrome();
	}

	void ensureTextures(const s3d::Size& sceneSize)
	{
		if ((m_sceneSize == sceneSize) && m_gaussianA1 && m_gaussianA4 && m_gaussianA8)
		{
			return;
		}

		m_sceneSize = sceneSize;
		m_gaussianA1 = s3d::RenderTexture{ m_sceneSize };
		m_gaussianB1 = s3d::RenderTexture{ m_sceneSize };
		m_gaussianA4 = s3d::RenderTexture{ m_sceneSize / 4 };
		m_gaussianB4 = s3d::RenderTexture{ m_sceneSize / 4 };
		m_gaussianA8 = s3d::RenderTexture{ m_sceneSize / 8 };
		m_gaussianB8 = s3d::RenderTexture{ m_sceneSize / 8 };
	}

	[[nodiscard]] RectF getBottomBarRect() const
	{
		return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, static_cast<double>(m_sceneSize.x), m_bottomBarHeight };
	}

	[[nodiscard]] RectF getLeftBorderRect() const
	{
		return RectF{ 0, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
	}

	[[nodiscard]] RectF getRightBorderRect() const
	{
		return RectF{ static_cast<double>(m_sceneSize.x) - m_borderThickness, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
	}

	[[nodiscard]] RectF getCloseButtonRect() const
	{
		return RectF{ static_cast<double>(m_sceneSize.x) - m_closeButtonSize - 6.0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight + 3.0, m_closeButtonSize, m_bottomBarHeight - 6.0 };
	}

	[[nodiscard]] RectF getFullscreenButtonRect() const
	{
		const RectF closeButtonRect = getCloseButtonRect();
		return RectF{ (closeButtonRect.x - m_buttonSpacing - m_fullscreenButtonWidth), closeButtonRect.y, m_fullscreenButtonWidth, closeButtonRect.h };
	}

	[[nodiscard]] RectF getAudioButtonRect() const
	{
		const RectF fullscreenButtonRect = getFullscreenButtonRect();
		return RectF{ (fullscreenButtonRect.x - m_buttonSpacing - m_audioButtonWidth), fullscreenButtonRect.y, m_audioButtonWidth, fullscreenButtonRect.h };
	}

	[[nodiscard]] RectF getVolumePanelRect() const
	{
		const RectF audioButtonRect = getAudioButtonRect();
		const double panelX = Clamp((audioButtonRect.rightX() - m_volumePanelWidth), 6.0, Max(6.0, static_cast<double>(m_sceneSize.x) - m_volumePanelWidth - 6.0));
		return RectF{ panelX, (audioButtonRect.y - m_volumePanelHeight - 6.0), m_volumePanelWidth, m_volumePanelHeight };
	}

	[[nodiscard]] RectF getVolumeRowRect(const int32 rowIndex) const
	{
		const RectF panelRect = getVolumePanelRect();
		return RectF{ (panelRect.x + 10.0), (panelRect.y + 8.0 + (rowIndex * 31.0)), (panelRect.w - 20.0), 25.0 };
	}

	[[nodiscard]] RectF getVolumeDecreaseButtonRect(const int32 rowIndex) const
	{
		const RectF rowRect = getVolumeRowRect(rowIndex);
		return RectF{ (rowRect.x + 42.0), (rowRect.y + 2.0), 20.0, 20.0 };
	}

	[[nodiscard]] RectF getVolumeIncreaseButtonRect(const int32 rowIndex) const
	{
		const RectF rowRect = getVolumeRowRect(rowIndex);
		return RectF{ (rowRect.rightX() - 20.0), (rowRect.y + 2.0), 20.0, 20.0 };
	}

	[[nodiscard]] RectF getVolumeGaugeRect(const int32 rowIndex) const
	{
		const RectF decreaseButtonRect = getVolumeDecreaseButtonRect(rowIndex);
		const RectF increaseButtonRect = getVolumeIncreaseButtonRect(rowIndex);
		return RectF{ (decreaseButtonRect.rightX() + 8.0), (decreaseButtonRect.y + 6.0), Max(0.0, (increaseButtonRect.x - decreaseButtonRect.rightX() - 16.0)), 8.0 };
	}

	[[nodiscard]] RectF getCloseDialogRect() const
	{
		return RectF{ s3d::Arg::center = Scene::CenterF(), 420, 180 };
	}

	[[nodiscard]] RectF getCloseDialogYesButtonRect() const
	{
		const RectF dialogRect = getCloseDialogRect();
		return RectF{ s3d::Arg::center(dialogRect.center().movedBy(-90, 44)), 140, 40 };
	}

	[[nodiscard]] RectF getCloseDialogNoButtonRect() const
	{
		const RectF dialogRect = getCloseDialogRect();
		return RectF{ s3d::Arg::center(dialogRect.center().movedBy(90, 44)), 140, 40 };
	}

	[[nodiscard]] RectF getDragRect() const
	{
		const double dragWidth = Max(0.0, getAudioButtonRect().x - 6.0);
		return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, dragWidth, m_bottomBarHeight };
	}

	void handleCloseButton()
	{
		if (getCloseButtonRect().leftClicked())
		{
			openCloseDialog();
		}
	}

	void openCloseDialog()
	{
		m_isCloseDialogOpen = true;
		m_isAudioPanelOpen = false;
		m_dragStartWindow.reset();
	}

	void handleCloseDialog()
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

	void handleFullscreenButton()
	{
		if (!getFullscreenButtonRect().leftClicked())
		{
			return;
		}

		const bool isFullscreen = Window::GetState().fullscreen;
		Window::SetFullscreen(!isFullscreen);

		if (isFullscreen)
		{
			Window::SetStyle(s3d::WindowStyle::Frameless);
		}
	}

	void handleAudioButton()
	{
		if (getAudioButtonRect().leftClicked())
		{
			m_isAudioPanelOpen = !m_isAudioPanelOpen;
			return;
		}

		if (m_isAudioPanelOpen && MouseL.down() && !getVolumePanelRect().mouseOver())
		{
			m_isAudioPanelOpen = false;
		}
	}

	static double getBusVolume(const s3d::MixBus bus)
	{
		return Clamp(s3d::GlobalAudio::BusGetVolume(bus), 0.0, 1.0);
	}

	static double getMasterVolume()
	{
		return Clamp(s3d::GlobalAudio::GetVolume(), 0.0, 1.0);
	}

	static void setBusVolume(const s3d::MixBus bus, const double volume)
	{
		s3d::GlobalAudio::BusSetVolume(bus, Clamp(volume, 0.0, 1.0));
	}

	static void setMasterVolume(const double volume)
	{
		s3d::GlobalAudio::SetVolume(Clamp(volume, 0.0, 1.0));
	}

	void handleVolumeControls()
	{
		if (!m_isAudioPanelOpen)
		{
			return;
		}

		handleMasterVolumeRowControls(0);
		handleVolumeRowControls(1, BgmBus);
		handleVolumeRowControls(2, SeBus);
	}

	void handleMasterVolumeRowControls(const int32 rowIndex)
	{
		if (getVolumeDecreaseButtonRect(rowIndex).leftClicked())
		{
			setMasterVolume(getMasterVolume() - m_volumeStep);
		}

		if (getVolumeIncreaseButtonRect(rowIndex).leftClicked())
		{
			setMasterVolume(getMasterVolume() + m_volumeStep);
		}

		const RectF gaugeRect = getVolumeGaugeRect(rowIndex);
		if (MouseL.pressed() && gaugeRect.mouseOver())
		{
			setMasterVolume((Cursor::PosF().x - gaugeRect.x) / gaugeRect.w);
		}
	}

	void handleVolumeRowControls(const int32 rowIndex, const s3d::MixBus bus)
	{
		if (getVolumeDecreaseButtonRect(rowIndex).leftClicked())
		{
			setBusVolume(bus, getBusVolume(bus) - m_volumeStep);
		}

		if (getVolumeIncreaseButtonRect(rowIndex).leftClicked())
		{
			setBusVolume(bus, getBusVolume(bus) + m_volumeStep);
		}

		const RectF gaugeRect = getVolumeGaugeRect(rowIndex);
		if (MouseL.pressed() && gaugeRect.mouseOver())
		{
			setBusVolume(bus, ((Cursor::PosF().x - gaugeRect.x) / gaugeRect.w));
		}
	}

	void handleWindowDrag()
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

	[[nodiscard]] bool isPointerOnInteractiveChrome() const
	{
		if (getCloseButtonRect().mouseOver() || getFullscreenButtonRect().mouseOver() || getAudioButtonRect().mouseOver())
		{
			return true;
		}

		return (m_isAudioPanelOpen && getVolumePanelRect().mouseOver());
	}

	void drawGlow() const
	{
		if (!m_gaussianA1)
		{
			return;
		}

		drawGlowSource();

		{
			const s3d::ScopedRenderTarget2D target{ m_gaussianA1.clear(ColorF{ 0.0 }) };
			const s3d::ScopedRenderStates2D blend{ s3d::BlendState::Additive };
			drawGlowSource();
		}

		s3d::Shader::GaussianBlur(m_gaussianA1, m_gaussianB1, m_gaussianA1);
		s3d::Shader::Downsample(m_gaussianA1, m_gaussianA4);
		s3d::Shader::GaussianBlur(m_gaussianA4, m_gaussianB4, m_gaussianA4);
		s3d::Shader::Downsample(m_gaussianA4, m_gaussianA8);
		s3d::Shader::GaussianBlur(m_gaussianA8, m_gaussianB8, m_gaussianA8);

		{
			const s3d::ScopedRenderStates2D blend{ s3d::BlendState::Additive };
			if (m_a1) m_gaussianA1.resized(m_sceneSize).draw(ColorF{ m_a1 });
			if (m_a4) m_gaussianA4.resized(m_sceneSize).draw(ColorF{ m_a4 });
			if (m_a8) m_gaussianA8.resized(m_sceneSize).draw(ColorF{ m_a8 });
		}
	}

	void drawGlowSource() const
	{
		//right area
		Rect{ m_sceneSize.x - 1, 0, 30, m_sceneSize.y }.draw(m_frameColor);
		//top area
		Rect{ 0, -29, m_sceneSize.x, 30 }.draw(m_frameColor);
		//left area
		Rect{ -29, 0, 30, m_sceneSize.y }.draw(m_frameColor);
		//bottom area
		Rect{ 0, m_sceneSize.y - 30, m_sceneSize.x, 30 }.draw(m_frameColor);
	}

	void drawChrome() const
	{
		const RectF bottomBarRect = getBottomBarRect();
		const RectF audioButtonRect = getAudioButtonRect();
		const RectF fullscreenButtonRect = getFullscreenButtonRect();
		const RectF closeButtonRect = getCloseButtonRect();
		const bool audioHovered = audioButtonRect.mouseOver();
		const bool fullscreenHovered = fullscreenButtonRect.mouseOver();
		const bool closeHovered = closeButtonRect.mouseOver();

		bottomBarRect.draw(ColorF{ 0.08, 0.10, 0.12, 0.82 });
		bottomBarRect.drawFrame(1.0, m_frameColor);
		m_titleFont(m_title).draw(10.0, (bottomBarRect.y + 4.0), Palette::White);

		drawControlButton(audioButtonRect, U"音量", audioHovered, m_isAudioPanelOpen);
		drawControlButton(fullscreenButtonRect, Window::GetState().fullscreen ? U"窓" : U"全画面", fullscreenHovered, false);
		closeButtonRect.draw(closeHovered ? ColorF{ 0.60, 0.16, 0.20, 0.95 } : ColorF{ 0.20, 0.11, 0.16, 0.90 });
		closeButtonRect.drawFrame(1.0, m_frameColor);
		Line{ closeButtonRect.tl().movedBy(5, 4), closeButtonRect.br().movedBy(-5, -4) }.draw(1.5, Palette::White);
		Line{ closeButtonRect.tr().movedBy(-5, 4), closeButtonRect.bl().movedBy(5, -4) }.draw(1.5, Palette::White);

		if (m_isAudioPanelOpen)
		{
			drawVolumePanel();
		}

		if (m_isCloseDialogOpen)
		{
			drawCloseDialog();
		}
	}

	void drawControlButton(const RectF& rect, const String& label, const bool hovered, const bool selected) const
	{
		const ColorF fillColor = selected
			? ColorF{ 0.18, 0.33, 0.24, 0.96 }
			: (hovered ? ColorF{ 0.16, 0.18, 0.22, 0.96 } : ColorF{ 0.10, 0.12, 0.16, 0.90 });

		rect.draw(fillColor);
		rect.drawFrame(1.0, m_frameColor);
		m_titleFont(label).drawAt(rect.center(), Palette::White);
	}

	void drawVolumePanel() const
	{
		const RectF panelRect = getVolumePanelRect();
		panelRect.draw(ColorF{ 0.05, 0.07, 0.09, 0.96 });
		panelRect.drawFrame(1.0, m_frameColor);
		drawMasterVolumeRow(U"Master", 0);
		drawVolumeRow(U"BGM", 1, BgmBus);
		drawVolumeRow(U"SE", 2, SeBus);
	}

	void drawMasterVolumeRow(const String& label, const int32 rowIndex) const
	{
		drawVolumeRowInternal(label, rowIndex, getMasterVolume());
	}

	void drawVolumeRow(const String& label, const int32 rowIndex, const s3d::MixBus bus) const
	{
		drawVolumeRowInternal(label, rowIndex, getBusVolume(bus));
	}

	void drawVolumeRowInternal(const String& label, const int32 rowIndex, const double volume) const
	{
		const RectF rowRect = getVolumeRowRect(rowIndex);
		const RectF decreaseButtonRect = getVolumeDecreaseButtonRect(rowIndex);
		const RectF increaseButtonRect = getVolumeIncreaseButtonRect(rowIndex);
		const RectF gaugeRect = getVolumeGaugeRect(rowIndex);

		m_titleFont(label).draw(rowRect.x, rowRect.y, Palette::White);
		decreaseButtonRect.draw(decreaseButtonRect.mouseOver() ? ColorF{ 0.18, 0.18, 0.22, 0.96 } : ColorF{ 0.10, 0.12, 0.16, 0.90 });
		decreaseButtonRect.drawFrame(1.0, m_frameColor);
		m_titleFont(U"-").drawAt(decreaseButtonRect.center(), Palette::White);

		gaugeRect.draw(ColorF{ 0.10, 0.12, 0.16, 0.90 });
		RectF{ gaugeRect.x, gaugeRect.y, (gaugeRect.w * volume), gaugeRect.h }.draw(ColorF{ 0.28, 0.74, 0.42, 0.96 });
		gaugeRect.drawFrame(1.0, ColorF{ 0.65, 0.75, 0.80, 0.85 });

		increaseButtonRect.draw(increaseButtonRect.mouseOver() ? ColorF{ 0.18, 0.18, 0.22, 0.96 } : ColorF{ 0.10, 0.12, 0.16, 0.90 });
		increaseButtonRect.drawFrame(1.0, m_frameColor);
		m_titleFont(U"+").drawAt(increaseButtonRect.center(), Palette::White);
	}

	void drawCloseDialog() const
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

		const RectF dialogRect = getCloseDialogRect();
		dialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		dialogRect.drawFrame(2.0, ColorF{ 0.40, 0.58, 0.90, 0.98 });

		m_dialogFont(U"ゲームを終了しますか？").drawAt(dialogRect.center().movedBy(0, -34), Palette::White);
		drawDialogButton(getCloseDialogYesButtonRect(), U"はい", true);
		drawDialogButton(getCloseDialogNoButtonRect(), U"いいえ", false);
		m_dialogSmallFont(U"Enter: はい / Esc: いいえ").drawAt(dialogRect.center().movedBy(0, 70), ColorF{ 0.80, 0.87, 0.95 });
	}

	void drawDialogButton(const RectF& rect, const String& label, const bool selected) const
	{
		const bool hovered = rect.mouseOver();
		const ColorF fillColor = selected
			? (hovered ? ColorF{ 0.22, 0.34, 0.60, 0.98 } : ColorF{ 0.18, 0.28, 0.52, 0.98 })
			: (hovered ? ColorF{ 0.14, 0.18, 0.26, 0.98 } : ColorF{ 0.10, 0.14, 0.20, 0.98 });

		rect.draw(fillColor);
		rect.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });
		m_dialogFont(label).drawAt(rect.center(), Palette::White);
	}

	String m_title = U"LoganToga2Remake2";
	ColorF m_frameColor{ 0.32, 0.58, 0.92 };
	ColorF m_glowColor{ 0.92, 0.56, 0.78, 0.85 };
	double m_bottomBarHeight = 24.0;
	double m_borderThickness = 2.0;
	double m_innerGlowThickness = 8.0;
	double m_audioButtonWidth = 50.0;
	double m_fullscreenButtonWidth = 66.0;
	double m_buttonSpacing = 6.0;
	double m_closeButtonSize = 30.0;
	double m_volumePanelWidth = 190.0;
	double m_volumePanelHeight = 103.0;
	double m_volumeStep = 0.1;
	s3d::Size m_sceneSize{ 0, 0 };
	s3d::RenderTexture m_gaussianA1;
	s3d::RenderTexture m_gaussianB1;
	s3d::RenderTexture m_gaussianA4;
	s3d::RenderTexture m_gaussianB4;
	s3d::RenderTexture m_gaussianA8;
	s3d::RenderTexture m_gaussianB8;
	double m_a1 = 0.0;
	double m_a4 = 0.0;
	double m_a8 = 0.5;
	Font m_titleFont{ FontMethod::MSDF, 12, Typeface::Medium };
	Font m_dialogFont{ FontMethod::MSDF, 24, Typeface::Bold };
	Font m_dialogSmallFont{ 16, Typeface::Medium };
	bool m_isAudioPanelOpen = false;
	bool m_isCloseDialogOpen = false;
	Optional<std::pair<s3d::Point, s3d::Point>> m_dragStartWindow;
};
