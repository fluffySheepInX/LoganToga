# pragma once
# include "MainContext.hpp"

namespace SkyAppUiLayout
{
  namespace
	{
		inline constexpr int32 PanelMargin = 20;
		inline constexpr int32 PanelGap = 20;
		inline constexpr int32 RightColumnWidth = 220;
           inline constexpr int32 ResourcePanelMinWidth = RightColumnWidth;
			inline constexpr int32 ResourcePanelResizeHandleSize = 18;
      inline constexpr int32 ModelHeightPanelWidth = 460;
        inline constexpr int32 ModelHeightPanelHeight = 420;
      inline constexpr int32 ResourcePanelIconButtonSize = 36;
		inline constexpr int32 ResourcePanelIconButtonGap = 8;
      inline constexpr int32 ResourcePanelCollapsedHeight = 96;
       inline constexpr int32 ResourcePanelExpandedHeight = 248;
		inline constexpr int32 BottomControlYOffset = 100;
     inline constexpr int32 BottomControlPanelWidth = 804;
		inline constexpr int32 BottomControlPanelHeight = 52;
		inline constexpr int32 BottomControlCheckBoxWidth = 96;
		inline constexpr int32 BottomControlButtonGap = 8;
       inline constexpr int32 BottomEditorIconButtonSize = 24;
		inline constexpr int32 BottomEditorIconButtonGap = 8;
       inline constexpr int32 AccordionHeaderHeight = 36;
            inline constexpr int32 MiniMapExpandedHeight = 220;
         inline constexpr int32 SkySettingsExpandedHeight = 466;
			inline constexpr int32 CameraSettingsExpandedHeight = 416;
           inline constexpr int32 TerrainVisualSettingsPanelWidth = 420;
           inline constexpr int32 TerrainVisualSettingsExpandedHeight = 560;

		[[nodiscard]] inline int32 RightColumnX(const int32 sceneWidth)
		{
			return Max(PanelMargin, (sceneWidth - RightColumnWidth - PanelMargin));
		}

		[[nodiscard]] inline Point ClampPanelPosition(const Point& position, const int32 panelWidth, const int32 panelHeight, const int32 sceneWidth, const int32 sceneHeight)
		{
			return Point{
				Clamp(position.x, 0, Max(0, (sceneWidth - panelWidth))),
				Clamp(position.y, 0, Max(0, (sceneHeight - panelHeight)))
			};
		}
	}

	inline constexpr int32 UiEditGridCellSize = 8;
	inline constexpr int32 UiEditGridMajorLineSpan = 4;

	[[nodiscard]] inline int32 SnapToUiEditGrid(const int32 value)
	{
		return static_cast<int32>(Math::Round(static_cast<double>(value) / UiEditGridCellSize) * UiEditGridCellSize);
	}

 [[nodiscard]] inline int32 AccordionPanelHeight(const bool expanded, const int32 expandedHeight)
	{
		return expanded ? expandedHeight : AccordionHeaderHeight;
	}

    [[nodiscard]] inline Point DefaultMiniMapPosition(const int32 sceneWidth)
	{
       return Point{ RightColumnX(sceneWidth), (PanelMargin + ResourcePanelExpandedHeight + PanelGap) };
	}

	[[nodiscard]] inline Point DefaultResourcePanelPosition(const int32 sceneWidth)
	{
		return Point{ RightColumnX(sceneWidth), PanelMargin };
	}

		[[nodiscard]] inline Point DefaultResourcePanelSize()
		{
			return Point{ ResourcePanelMinWidth, ResourcePanelExpandedHeight };
		}

	[[nodiscard]] inline Point DefaultUnitEditorPosition(const int32 sceneWidth)
	{
		return Point{ (sceneWidth - 340), 124 };
	}

	[[nodiscard]] inline Point DefaultUnitEditorListPosition()
	{
		return Point{ 20, 124 };
	}

	[[nodiscard]] inline Point DefaultModelHeightPosition(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		return Point{ (20 + 480 + PanelGap), (20 + CameraSettingsExpandedHeight + PanelGap) };
	}

		[[nodiscard]] inline Point DefaultTerrainVisualSettingsPosition(const int32 sceneWidth, const int32 sceneHeight)
		{
			(void)sceneWidth;
			(void)sceneHeight;
			return Point{ (20 + 480 + PanelGap), (20 + CameraSettingsExpandedHeight + 16) };
		}

	[[nodiscard]] inline Point SnapToUiEditGrid(const Point& position)
	{
		return Point{ SnapToUiEditGrid(position.x), SnapToUiEditGrid(position.y) };
	}

	[[nodiscard]] inline Rect MiniMap(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const bool expanded = true)
	{
      const int32 panelHeight = AccordionPanelHeight(expanded, MiniMapExpandedHeight);
		const Point clampedPosition = ClampPanelPosition(position, RightColumnWidth, panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, RightColumnWidth, panelHeight };
	}

	[[nodiscard]] inline Rect MiniMap(const int32 sceneWidth, const int32 sceneHeight, const bool expanded = true)
	{
		return MiniMap(sceneWidth, sceneHeight, DefaultMiniMapPosition(sceneWidth), expanded);
	}

 [[nodiscard]] inline Rect SkySettings(const int32 sceneWidth, const int32 sceneHeight, const bool expanded = true)
	{
        (void)sceneWidth;
		(void)sceneHeight;
      (void)expanded;
		return Rect{ 20, 20, 480, SkySettingsExpandedHeight };
	}

  [[nodiscard]] inline Rect CameraSettings(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool expanded = true)
	{
     (void)sceneWidth;
		(void)sceneHeight;
       (void)skySettingsExpanded;
		(void)expanded;
		return Rect{ (20 + 480 + PanelGap), 20, 360, CameraSettingsExpandedHeight };
	}

  [[nodiscard]] inline Rect TerrainVisualSettings(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
      const Point clampedPosition = ClampPanelPosition(position, TerrainVisualSettingsPanelWidth, TerrainVisualSettingsExpandedHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, TerrainVisualSettingsPanelWidth, TerrainVisualSettingsExpandedHeight };
	}

	[[nodiscard]] inline Rect TerrainVisualSettings(const int32 sceneWidth, const int32 sceneHeight, const bool cameraSettingsExpanded = true, const bool expanded = true)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		(void)cameraSettingsExpanded;
		(void)expanded;
       return TerrainVisualSettings(sceneWidth, sceneHeight, DefaultTerrainVisualSettingsPosition(sceneWidth, sceneHeight));
	}

 [[nodiscard]] inline Rect MapEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
        (void)sceneHeight;
        return Rect{ (sceneWidth - 360), 20, 340, 660 };
	}

 [[nodiscard]] inline Rect BlacksmithMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
        return Rect{ (sceneWidth - 424), Max(20, (sceneHeight - 332)), 404, 312 };
	}

	[[nodiscard]] inline Rect BattleCommandSlotButton(const Rect& panelRect, const int32 index)
	{
		constexpr int32 SlotWidth = 82;
		constexpr int32 SlotHeight = 44;
		constexpr int32 SlotGap = 8;
		return Rect{ (panelRect.x + 16 + index * (SlotWidth + SlotGap)), (panelRect.y + 48), SlotWidth, SlotHeight };
	}

	[[nodiscard]] inline Rect BattleCommandPortrait(const Rect& panelRect)
	{
		return Rect{ (panelRect.x + 16), (panelRect.y + 108), 148, 112 };
	}

	[[nodiscard]] inline Rect BattleCommandDetail(const Rect& panelRect)
	{
		return Rect{ (panelRect.x + 178), (panelRect.y + 108), 210, 112 };
	}

	[[nodiscard]] inline Rect BattleCommandPrimaryActionButton(const Rect& panelRect)
	{
		return Rect{ (panelRect.x + 194), (panelRect.y + 182), 178, 28 };
	}

	[[nodiscard]] inline Rect BattleCommandTierUpButton(const Rect& panelRect)
	{
		return Rect{ (panelRect.x + 16), (panelRect.y + 252), 148, 40 };
	}

	[[nodiscard]] inline Rect BattleCommandMessageRect(const Rect& panelRect)
	{
		return Rect{ (panelRect.x + 178), (panelRect.y + 228), 210, 56 };
	}

 [[nodiscard]] inline Rect SapperMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
           return Rect{ (sceneWidth - 340), (sceneHeight - 396), 320, 356 };
	}

 [[nodiscard]] inline Rect MillStatusEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
       return Rect{ (sceneWidth - 340), Max(20, (sceneHeight - 620)), 320, 580 };
	}

    [[nodiscard]] inline Rect ModelHeight(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
      const Point clampedPosition = ClampPanelPosition(position, ModelHeightPanelWidth, ModelHeightPanelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, ModelHeightPanelWidth, ModelHeightPanelHeight };
	}

	[[nodiscard]] inline Rect ModelHeight(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool cameraSettingsExpanded = true)
	{
		(void)skySettingsExpanded;
		(void)cameraSettingsExpanded;
		return ModelHeight(sceneWidth, sceneHeight, DefaultModelHeightPosition(sceneWidth, sceneHeight));
	}

	[[nodiscard]] inline Rect UnitEditor(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
         const int32 panelHeight = Clamp((sceneHeight - PanelMargin * 2), 420, 748);
		const Point clampedPosition = ClampPanelPosition(position, 340, panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, 340, panelHeight };
	}

	[[nodiscard]] inline Rect UnitEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
		return UnitEditor(sceneWidth, sceneHeight, DefaultUnitEditorPosition(sceneWidth));
	}

	[[nodiscard]] inline Rect UnitEditorList(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
      const Point clampedPosition = ClampPanelPosition(position, 240, 440, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, 240, 440 };
	}

	[[nodiscard]] inline Rect UnitEditorList(const int32 sceneWidth, const int32 sceneHeight)
	{
		return UnitEditorList(sceneWidth, sceneHeight, DefaultUnitEditorListPosition());
	}

    [[nodiscard]] inline Point ClampResourcePanelSize(const Point& size, const Point& position, const int32 sceneWidth, const int32 sceneHeight, const bool expanded)
	{
		const int32 minHeight = (expanded ? ResourcePanelExpandedHeight : ResourcePanelCollapsedHeight);
		return Point{
			Clamp(size.x, ResourcePanelMinWidth, Max(ResourcePanelMinWidth, (sceneWidth - position.x))),
			Clamp(size.y, minHeight, Max(minHeight, (sceneHeight - position.y)))
		};
	}

 [[nodiscard]] inline Rect ResourcePanel(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const Point& size, const bool expanded = false, const bool showStoredHeight = false)
	{
     const Point clampedSize = ClampResourcePanelSize(size, position, sceneWidth, sceneHeight, expanded);
        const bool usesStoredHeight = (expanded || showStoredHeight);
		const int32 panelHeight = (usesStoredHeight ? clampedSize.y : ResourcePanelCollapsedHeight);
		const Point clampedPosition = ClampPanelPosition(position, clampedSize.x, panelHeight, sceneWidth, sceneHeight);
		const Point reclampedSize = ClampResourcePanelSize(clampedSize, clampedPosition, sceneWidth, sceneHeight, expanded);
      return Rect{ clampedPosition.x, clampedPosition.y, reclampedSize.x, (usesStoredHeight ? reclampedSize.y : ResourcePanelCollapsedHeight) };
	}

    [[nodiscard]] inline Rect ResourcePanel(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const bool expanded = false, const bool showStoredHeight = false)
	{
      return ResourcePanel(sceneWidth, sceneHeight, position, DefaultResourcePanelSize(), expanded, showStoredHeight);
	}

   [[nodiscard]] inline Rect ResourcePanel(const int32 sceneWidth, const int32 sceneHeight, const bool expanded = false, const bool showStoredHeight = false)
	{
          return ResourcePanel(sceneWidth, sceneHeight, DefaultResourcePanelPosition(sceneWidth), DefaultResourcePanelSize(), expanded, showStoredHeight);
	}

	[[nodiscard]] inline Rect ResourcePanelResizeHandle(const Rect& resourcePanel)
	{
		return Rect{ (resourcePanel.rightX() - ResourcePanelResizeHandleSize), (resourcePanel.bottomY() - ResourcePanelResizeHandleSize), ResourcePanelResizeHandleSize, ResourcePanelResizeHandleSize };
	}

	[[nodiscard]] inline Rect ResourcePanelCameraHomeButton(const Rect& resourcePanel)
	{
		return Rect{
			Max(0, (resourcePanel.x - ResourcePanelIconButtonGap - ResourcePanelIconButtonSize)),
			resourcePanel.y,
			ResourcePanelIconButtonSize,
			ResourcePanelIconButtonSize
		};
	}

 [[nodiscard]] inline Rect UiToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
     (void)sceneWidth;
		return Rect{ 20, (sceneHeight - BottomControlYOffset), BottomControlPanelWidth, BottomControlPanelHeight };
	}

 [[nodiscard]] inline Rect BottomControlRevealHotZone(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		return Rect{ 0, Max(0, (sceneHeight - 160)), 28, 160 };
	}

	[[nodiscard]] inline Rect UiToggleCheckBox(const Rect& uiToggle)
	{
		return Rect{ (uiToggle.x + 10), (uiToggle.y + 8), BottomControlCheckBoxWidth, 36 };
	}

	[[nodiscard]] inline Rect MapModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
      const Rect uiToggle = UiToggle(sceneWidth, sceneHeight);
		return Rect{ (UiToggleCheckBox(uiToggle).rightX() + BottomControlButtonGap), (uiToggle.y + 8), 44, 36 };
	}

 [[nodiscard]] inline Rect ModelHeightModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
      const Rect anchorRect = MapModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect UnitEditorModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = ModelHeightModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 60, 36 };
	}

	[[nodiscard]] inline Rect SkySettingsToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = UnitEditorModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect CameraSettingsToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = SkySettingsToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect TerrainVisualToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
	const Rect anchorRect = CameraSettingsToggle(sceneWidth, sceneHeight);
	return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect UiEditModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = TerrainVisualToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 84, 36 };
	}

	[[nodiscard]] inline Rect ResourceAdjustToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = UiEditModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 88, 36 };
	}

	[[nodiscard]] inline Rect EnemyPlanToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
       const Rect anchorRect = ResourceAdjustToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + BottomControlButtonGap), anchorRect.y, 110, 36 };
	}

	[[nodiscard]] inline Rect BottomEditorTextColorsButton(const Rect& anchorToggle)
	{
		return Rect{ (anchorToggle.rightX() + BottomEditorIconButtonGap), (anchorToggle.y + (anchorToggle.h - BottomEditorIconButtonSize) / 2), BottomEditorIconButtonSize, BottomEditorIconButtonSize };
	}

 [[nodiscard]] inline Rect EscMenu(const int32 sceneWidth, const int32 sceneHeight)
	{
      return Rect{ ((sceneWidth - 280) / 2), ((sceneHeight - 300) / 2), 280, 300 };
	}

  [[nodiscard]] inline Rect TimeSlider(const int32 sceneWidth, const int32 sceneHeight)
	{
        return Rect{ 20, (sceneHeight - 60), Max(0, (sceneWidth - 40)), 36 };
	}

	[[nodiscard]] inline Rect AccordionHeaderRect(const Rect& panelRect)
	{
		return Rect{ panelRect.x, panelRect.y, panelRect.w, AccordionHeaderHeight };
	}

	[[nodiscard]] inline Vec2 BottomMessagePosition(const Rect& uiToggle, const int32 messageIndex)
	{
       return Vec2{ static_cast<double>(uiToggle.x + 12), static_cast<double>(uiToggle.y - 32 + messageIndex * 24) };
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

	[[nodiscard]] inline Rect SkySettingsTimePanel(const Rect& skySettingsPanel)
	{
		return Rect{ skySettingsPanel.x, (skySettingsPanel.bottomY() + 8), skySettingsPanel.w, 92 };
	}

	[[nodiscard]] inline Vec2 SkySettingsTimeSliderPosition(const Rect& timePanel)
	{
		return Vec2{ static_cast<double>(timePanel.x + 12), static_cast<double>(timePanel.y + 14) };
	}

	[[nodiscard]] inline int32 SkySettingsTimeSliderLabelWidth()
	{
		return 120;
	}

	[[nodiscard]] inline int32 SkySettingsTimeSliderTrackWidth(const Rect& timePanel)
	{
		return Max(0, (timePanel.w - 24 - (SkySettingsTimeSliderLabelWidth() + 20)));
	}

	[[nodiscard]] inline Rect SkySettingsTimeStepButton(const Rect& timePanel, const int32 index)
	{
		return Rect{ (timePanel.x + 12 + index * 70), (timePanel.y + 52), 62, 28 };
	}

	[[nodiscard]] inline Vec2 TerrainVisualPanelTextPosition(const Rect& panelRect, const int32 xOffset, const int32 yOffset)
	{
		return Vec2{ static_cast<double>(panelRect.x + xOffset), static_cast<double>(panelRect.y + yOffset) };
	}

	[[nodiscard]] inline Vec2 TerrainVisualPanelSliderPosition(const Rect& panelRect, const int32 yOffset)
	{
		return Vec2{ static_cast<double>(panelRect.x + 12), static_cast<double>(panelRect.y + yOffset) };
	}

	[[nodiscard]] inline int32 TerrainVisualPanelSliderLabelWidth()
	{
     return 148;
	}

	[[nodiscard]] inline int32 TerrainVisualPanelSliderWidth()
	{
     return 236;
	}

	[[nodiscard]] inline Rect TerrainVisualPanelButtonRect(const Rect& panelRect, const int32 xOffset, const int32 yOffset)
	{
		return Rect{ (panelRect.x + xOffset), (panelRect.y + yOffset), 92, 28 };
	}

	[[nodiscard]] inline Rect TerrainVisualPanelDragHandle(const Rect& panelRect)
	{
		return Rect{ (panelRect.rightX() - 88), (panelRect.y + 8), 72, 24 };
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
