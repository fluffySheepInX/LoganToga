#include "WindowChromeAddon.h"

void WindowChromeAddon::Configure(const String& title, const ColorF& frameColor, const ColorF& glowColor)
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

bool WindowChromeAddon::update()
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
	flushPersistedSettings();
	handleCloseButton();
	handleWindowDrag();
	return true;
}

void WindowChromeAddon::ensureTextures(const s3d::Size& sceneSize)
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

RectF WindowChromeAddon::getBottomBarRect() const
{
	return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, static_cast<double>(m_sceneSize.x), m_bottomBarHeight };
}

RectF WindowChromeAddon::getLeftBorderRect() const
{
	return RectF{ 0, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
}

RectF WindowChromeAddon::getRightBorderRect() const
{
	return RectF{ static_cast<double>(m_sceneSize.x) - m_borderThickness, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
}

RectF WindowChromeAddon::getCloseButtonRect() const
{
	return RectF{ static_cast<double>(m_sceneSize.x) - m_closeButtonSize - 6.0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight + 3.0, m_closeButtonSize, m_bottomBarHeight - 6.0 };
}

RectF WindowChromeAddon::getFullscreenButtonRect() const
{
	const RectF closeButtonRect = getCloseButtonRect();
	return RectF{ (closeButtonRect.x - m_buttonSpacing - m_fullscreenButtonWidth), closeButtonRect.y, m_fullscreenButtonWidth, closeButtonRect.h };
}

RectF WindowChromeAddon::getAudioButtonRect() const
{
	const RectF fullscreenButtonRect = getFullscreenButtonRect();
	return RectF{ (fullscreenButtonRect.x - m_buttonSpacing - m_audioButtonWidth), fullscreenButtonRect.y, m_audioButtonWidth, fullscreenButtonRect.h };
}

RectF WindowChromeAddon::getVolumePanelRect() const
{
	const RectF audioButtonRect = getAudioButtonRect();
	const double panelX = Clamp((audioButtonRect.rightX() - m_volumePanelWidth), 6.0, Max(6.0, static_cast<double>(m_sceneSize.x) - m_volumePanelWidth - 6.0));
	return RectF{ panelX, (audioButtonRect.y - m_volumePanelHeight - 6.0), m_volumePanelWidth, m_volumePanelHeight };
}

RectF WindowChromeAddon::getVolumeRowRect(const int32 rowIndex) const
{
	const RectF panelRect = getVolumePanelRect();
	return RectF{ (panelRect.x + 10.0), (panelRect.y + 8.0 + (rowIndex * 31.0)), (panelRect.w - 20.0), 25.0 };
}

RectF WindowChromeAddon::getVolumeDecreaseButtonRect(const int32 rowIndex) const
{
	const RectF rowRect = getVolumeRowRect(rowIndex);
	return RectF{ (rowRect.x + 42.0), (rowRect.y + 2.0), 20.0, 20.0 };
}

RectF WindowChromeAddon::getVolumeIncreaseButtonRect(const int32 rowIndex) const
{
	const RectF rowRect = getVolumeRowRect(rowIndex);
	return RectF{ (rowRect.rightX() - 20.0), (rowRect.y + 2.0), 20.0, 20.0 };
}

RectF WindowChromeAddon::getVolumeGaugeRect(const int32 rowIndex) const
{
	const RectF decreaseButtonRect = getVolumeDecreaseButtonRect(rowIndex);
	const RectF increaseButtonRect = getVolumeIncreaseButtonRect(rowIndex);
	return RectF{ (decreaseButtonRect.rightX() + 8.0), (decreaseButtonRect.y + 6.0), Max(0.0, (increaseButtonRect.x - decreaseButtonRect.rightX() - 16.0)), 8.0 };
}

RectF WindowChromeAddon::getCloseDialogRect() const
{
	return RectF{ s3d::Arg::center = Scene::CenterF(), 420, 180 };
}

RectF WindowChromeAddon::getCloseDialogYesButtonRect() const
{
	const RectF dialogRect = getCloseDialogRect();
	return RectF{ s3d::Arg::center(dialogRect.center().movedBy(-90, 44)), 140, 40 };
}

RectF WindowChromeAddon::getCloseDialogNoButtonRect() const
{
	const RectF dialogRect = getCloseDialogRect();
	return RectF{ s3d::Arg::center(dialogRect.center().movedBy(90, 44)), 140, 40 };
}

RectF WindowChromeAddon::getDragRect() const
{
	const double dragWidth = Max(0.0, getAudioButtonRect().x - 6.0);
	return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, dragWidth, m_bottomBarHeight };
}
