# pragma once
# include "MainContext.hpp"

namespace SkyAppUiLayout
{
  namespace
	{
		inline constexpr int32 PanelMargin = 20;
		inline constexpr int32 PanelGap = 20;
		inline constexpr int32 RightColumnWidth = 220;
		inline constexpr int32 BottomControlYOffset = 100;
       inline constexpr int32 AccordionHeaderHeight = 36;
            inline constexpr int32 MiniMapExpandedHeight = 220;
         inline constexpr int32 SkySettingsExpandedHeight = 466;
			inline constexpr int32 CameraSettingsExpandedHeight = 416;
	}

 [[nodiscard]] inline int32 AccordionPanelHeight(const bool expanded, const int32 expandedHeight)
	{
		return expanded ? expandedHeight : AccordionHeaderHeight;
	}

  [[nodiscard]] inline Rect MiniMap(const int32 sceneWidth, const int32 sceneHeight, const bool expanded = true)
	{
		(void)sceneHeight;
        return Rect{ Max(PanelMargin, (sceneWidth - RightColumnWidth - PanelMargin)), 124, 220, AccordionPanelHeight(expanded, MiniMapExpandedHeight) };
	}

 [[nodiscard]] inline Rect SkySettings(const int32 sceneWidth, const int32 sceneHeight, const bool expanded = true)
	{
        (void)sceneWidth;
		(void)sceneHeight;
        return Rect{ 20, 20, 480, AccordionPanelHeight(expanded, SkySettingsExpandedHeight) };
	}

  [[nodiscard]] inline Rect CameraSettings(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool expanded = true)
	{
     (void)sceneWidth;
		(void)sceneHeight;
      return Rect{ (20 + 480 + PanelGap), (SkySettings(sceneWidth, sceneHeight, skySettingsExpanded).y), 360, AccordionPanelHeight(expanded, CameraSettingsExpandedHeight) };
	}

 [[nodiscard]] inline Rect MapEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
        (void)sceneHeight;
		return Rect{ (sceneWidth - 360), 20, 340, 580 };
	}

 [[nodiscard]] inline Rect BlacksmithMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
        return Rect{ (sceneWidth - 320), (sceneHeight - 224), 300, 184 };
	}

 [[nodiscard]] inline Rect SapperMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
        return Rect{ (sceneWidth - 340), (sceneHeight - 324), 320, 284 };
	}

 [[nodiscard]] inline Rect MillStatusEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
       return Rect{ (sceneWidth - 340), Max(20, (sceneHeight - 620)), 320, 580 };
	}

 [[nodiscard]] inline Rect ModelHeight(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool cameraSettingsExpanded = true)
	{
     (void)sceneWidth;
		(void)sceneHeight;
     const Rect cameraPanel = CameraSettings(sceneWidth, sceneHeight, skySettingsExpanded, cameraSettingsExpanded);
		return Rect{ cameraPanel.x, (cameraPanel.bottomY() + PanelGap), 400, 300 };
	}

   [[nodiscard]] inline Rect ResourcePanel(const int32 sceneWidth, const int32 sceneHeight)
	{
      (void)sceneHeight;
		return Rect{ Max(PanelMargin, (sceneWidth - RightColumnWidth - PanelMargin)), 20, 220, 84 };
	}

    [[nodiscard]] inline Rect UiToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
      (void)sceneWidth;
		return Rect{ 20, (sceneHeight - BottomControlYOffset), 140, 36 };
	}

   [[nodiscard]] inline Rect MapModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
     (void)sceneWidth;
      return Rect{ 180, (sceneHeight - BottomControlYOffset), 44, 36 };
	}

   [[nodiscard]] inline Rect ModelHeightModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
     (void)sceneWidth;
        return Rect{ 244, (sceneHeight - BottomControlYOffset), 44, 36 };
	}

 [[nodiscard]] inline Rect EscMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
      return Rect{ ((sceneWidth - 280) / 2), ((sceneHeight - 140) / 2), 280, 140 };
	}

  [[nodiscard]] inline Rect TimeSlider(const int32 sceneWidth, const int32 sceneHeight)
	{
        return Rect{ 20, (sceneHeight - 60), Max(0, (sceneWidth - 40)), 36 };
	}

	[[nodiscard]] inline Rect AccordionHeaderRect(const Rect& panelRect)
	{
		return Rect{ panelRect.x, panelRect.y, panelRect.w, AccordionHeaderHeight };
	}

	[[nodiscard]] inline Vec2 UiTogglePosition(const Rect& uiToggle)
	{
		return Vec2{ static_cast<double>(uiToggle.x), static_cast<double>(uiToggle.y) };
	}

	[[nodiscard]] inline Vec2 BottomMessagePosition(const Rect& uiToggle, const int32 messageIndex)
	{
		return Vec2{ static_cast<double>(uiToggle.x), static_cast<double>(uiToggle.y - 32 + messageIndex * 24) };
	}

	[[nodiscard]] inline Vec2 TimeSliderPosition(const Rect& timeSlider)
	{
		return Vec2{ static_cast<double>(timeSlider.x), static_cast<double>(timeSlider.y) };
	}

	[[nodiscard]] inline int32 TimeSliderLabelWidth()
	{
		return 120;
	}

	[[nodiscard]] inline int32 TimeSliderTrackWidth(const Rect& timeSlider)
	{
		return Max(0, (timeSlider.w - (TimeSliderLabelWidth() + 20)));
	}

	[[nodiscard]] inline Vec2 ResourcePanelTitlePosition(const Rect& resourcePanel)
	{
		return Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 6) };
	}

	[[nodiscard]] inline Vec2 ResourcePanelBudgetPosition(const Rect& resourcePanel)
	{
		return Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 28) };
	}

	[[nodiscard]] inline Vec2 ResourcePanelGunpowderPosition(const Rect& resourcePanel)
	{
		return Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 48) };
	}

	[[nodiscard]] inline Vec2 ResourcePanelManaPosition(const Rect& resourcePanel)
	{
		return Vec2{ static_cast<double>(resourcePanel.x + 112), static_cast<double>(resourcePanel.y + 48) };
	}

	[[nodiscard]] inline Vec2 MenuTextPosition(const Rect& panelRect, const int32 yOffset)
	{
		return Vec2{ static_cast<double>(panelRect.x + 16), static_cast<double>(panelRect.y + yOffset) };
	}

	[[nodiscard]] inline Rect MenuWideButton(const Rect& panelRect, const int32 yOffset)
	{
		return Rect{ (panelRect.x + 16), (panelRect.y + yOffset), (panelRect.w - 32), 28 };
	}

	[[nodiscard]] inline Vec2 MenuMessagePosition(const Rect& panelRect)
	{
		return Vec2{ static_cast<double>(panelRect.x + 16), static_cast<double>(panelRect.y - 28) };
	}

	[[nodiscard]] inline Vec2 CameraPanelTextPosition(const Rect& panelRect, const int32 xOffset, const int32 yOffset)
	{
		return Vec2{ static_cast<double>(panelRect.x + xOffset), static_cast<double>(panelRect.y + yOffset) };
	}

	[[nodiscard]] inline Vec2 CameraPanelSliderPosition(const Rect& panelRect, const int32 yOffset)
	{
		return Vec2{ static_cast<double>(panelRect.x + 20), static_cast<double>(panelRect.y + yOffset) };
	}

	[[nodiscard]] inline Rect CameraPanelButtonRect(const Rect& panelRect, const int32 xOffset, const int32 yOffset, const int32 width = 150, const int32 height = 30)
	{
		return Rect{ (panelRect.x + xOffset), (panelRect.y + yOffset), width, height };
	}

	[[nodiscard]] inline int32 CameraPanelSliderLabelWidth()
	{
		return 140;
	}

	[[nodiscard]] inline int32 CameraPanelSliderWidth()
	{
		return 180;
	}

	[[nodiscard]] inline int32 CameraPanelClipSelectorWidth()
	{
		return 150;
	}
}
