#include "WindowChromeAddon.h"

void WindowChromeAddon::draw() const
{
	drawGlow();
	drawChrome();
}

void WindowChromeAddon::drawGlow() const
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

void WindowChromeAddon::drawGlowSource() const
{
	Rect{ m_sceneSize.x - 1, 0, 30, m_sceneSize.y }.draw(m_frameColor);
	Rect{ 0, -29, m_sceneSize.x, 30 }.draw(m_frameColor);
	Rect{ -29, 0, 30, m_sceneSize.y }.draw(m_frameColor);
	Rect{ 0, m_sceneSize.y - 30, m_sceneSize.x, 30 }.draw(m_frameColor);
}

void WindowChromeAddon::drawChrome() const
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

void WindowChromeAddon::drawControlButton(const RectF& rect, const String& label, const bool hovered, const bool selected) const
{
	const ColorF fillColor = selected
		? ColorF{ 0.18, 0.33, 0.24, 0.96 }
		: (hovered ? ColorF{ 0.16, 0.18, 0.22, 0.96 } : ColorF{ 0.10, 0.12, 0.16, 0.90 });

	rect.draw(fillColor);
	rect.drawFrame(1.0, m_frameColor);
	m_titleFont(label).drawAt(rect.center(), Palette::White);
}

void WindowChromeAddon::drawVolumePanel() const
{
	const RectF panelRect = getVolumePanelRect();
	panelRect.draw(ColorF{ 0.05, 0.07, 0.09, 0.96 });
	panelRect.drawFrame(1.0, m_frameColor);
	drawMasterVolumeRow(U"Master", 0);
	drawVolumeRow(U"BGM", 1, BgmBus);
	drawVolumeRow(U"SE", 2, SeBus);
}

void WindowChromeAddon::drawMasterVolumeRow(const String& label, const int32 rowIndex) const
{
	drawVolumeRowInternal(label, rowIndex, getMasterVolume());
}

void WindowChromeAddon::drawVolumeRow(const String& label, const int32 rowIndex, const s3d::MixBus bus) const
{
	drawVolumeRowInternal(label, rowIndex, getBusVolume(bus));
}

void WindowChromeAddon::drawVolumeRowInternal(const String& label, const int32 rowIndex, const double volume) const
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

void WindowChromeAddon::drawCloseDialog() const
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

void WindowChromeAddon::drawDialogButton(const RectF& rect, const String& label, const bool selected) const
{
	const bool hovered = rect.mouseOver();
	const ColorF fillColor = selected
		? (hovered ? ColorF{ 0.22, 0.34, 0.60, 0.98 } : ColorF{ 0.18, 0.28, 0.52, 0.98 })
		: (hovered ? ColorF{ 0.14, 0.18, 0.26, 0.98 } : ColorF{ 0.10, 0.14, 0.20, 0.98 });

	rect.draw(fillColor);
	rect.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });
	m_dialogFont(label).drawAt(rect.center(), Palette::White);
}
