# pragma once
# include "SkyAppUiLayoutCommon.hpp"

namespace SkyAppUiLayout
{
	[[nodiscard]] inline Rect MiniMap(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const bool expanded = true)
	{
		const auto& layout = Detail::LayoutProfile();
		const int32 panelHeight = AccordionPanelHeight(expanded, layout.miniMap.expandedHeight);
		const Point clampedPosition = Detail::ClampPanelPosition(position, layout.rightColumn.width, panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, layout.rightColumn.width, panelHeight };
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
		return Rect{ 20, 20, 480, Detail::LayoutProfile().skySettings.expandedHeight };
	}

	[[nodiscard]] inline Rect CameraSettings(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool expanded = true)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		(void)skySettingsExpanded;
		(void)expanded;
		const auto& layout = Detail::LayoutProfile();
		return Rect{ (20 + 480 + layout.shared.panelGap), 20, 360, layout.cameraSettings.expandedHeight };
	}

	[[nodiscard]] inline Rect TerrainVisualSettings(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
		const auto& layout = Detail::LayoutProfile();
		const Point clampedPosition = Detail::ClampPanelPosition(position, layout.terrainVisualSettings.panelWidth, layout.terrainVisualSettings.expandedHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, layout.terrainVisualSettings.panelWidth, layout.terrainVisualSettings.expandedHeight };
	}

	[[nodiscard]] inline Rect TerrainVisualSettings(const int32 sceneWidth, const int32 sceneHeight, const bool cameraSettingsExpanded = true, const bool expanded = true)
	{
		(void)sceneWidth;
		(void)sceneHeight;
		(void)cameraSettingsExpanded;
		(void)expanded;
		return TerrainVisualSettings(sceneWidth, sceneHeight, DefaultTerrainVisualSettingsPosition(sceneWidth, sceneHeight));
	}

	[[nodiscard]] inline Rect FogSettings(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
		const auto& layout = Detail::LayoutProfile();
		const Point clampedPosition = Detail::ClampPanelPosition(position, layout.fogSettings.panelWidth, layout.fogSettings.panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, layout.fogSettings.panelWidth, layout.fogSettings.panelHeight };
	}

	[[nodiscard]] inline Rect FogSettings(const int32 sceneWidth, const int32 sceneHeight)
	{
		return FogSettings(sceneWidth, sceneHeight, DefaultFogSettingsPosition(sceneWidth, sceneHeight));
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
		const auto& layout = Detail::LayoutProfile();
		const Point clampedPosition = Detail::ClampPanelPosition(position, layout.modelHeight.panelWidth, layout.modelHeight.panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, layout.modelHeight.panelWidth, layout.modelHeight.panelHeight };
	}

	[[nodiscard]] inline Rect ModelHeight(const int32 sceneWidth, const int32 sceneHeight, const bool skySettingsExpanded = true, const bool cameraSettingsExpanded = true)
	{
		(void)skySettingsExpanded;
		(void)cameraSettingsExpanded;
		return ModelHeight(sceneWidth, sceneHeight, DefaultModelHeightPosition(sceneWidth, sceneHeight));
	}

	[[nodiscard]] inline Rect UnitEditor(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
		const auto& layout = Detail::LayoutProfile();
		const int32 panelHeight = Clamp((sceneHeight - layout.shared.panelMargin * 2), 420, 748);
		const Point clampedPosition = Detail::ClampPanelPosition(position, 340, panelHeight, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, 340, panelHeight };
	}

	[[nodiscard]] inline Rect UnitEditor(const int32 sceneWidth, const int32 sceneHeight)
	{
		return UnitEditor(sceneWidth, sceneHeight, DefaultUnitEditorPosition(sceneWidth));
	}

	[[nodiscard]] inline Rect UnitEditorList(const int32 sceneWidth, const int32 sceneHeight, const Point& position)
	{
		const Point clampedPosition = Detail::ClampPanelPosition(position, 240, 440, sceneWidth, sceneHeight);
		return Rect{ clampedPosition.x, clampedPosition.y, 240, 440 };
	}

	[[nodiscard]] inline Rect UnitEditorList(const int32 sceneWidth, const int32 sceneHeight)
	{
		return UnitEditorList(sceneWidth, sceneHeight, DefaultUnitEditorListPosition());
	}

	[[nodiscard]] inline Point ClampResourcePanelSize(const Point& size, const Point& position, const int32 sceneWidth, const int32 sceneHeight, const bool expanded)
	{
		const auto& layout = Detail::LayoutProfile();
		const int32 minHeight = (expanded ? layout.resourcePanel.expandedHeight : layout.resourcePanel.collapsedHeight);
		return Point{
			Clamp(size.x, layout.resourcePanel.minWidth, Max(layout.resourcePanel.minWidth, (sceneWidth - position.x))),
			Clamp(size.y, minHeight, Max(minHeight, (sceneHeight - position.y)))
		};
	}

	[[nodiscard]] inline Rect ResourcePanel(const int32 sceneWidth, const int32 sceneHeight, const Point& position, const Point& size, const bool expanded = false, const bool showStoredHeight = false)
	{
		const Point clampedSize = ClampResourcePanelSize(size, position, sceneWidth, sceneHeight, expanded);
		const bool usesStoredHeight = (expanded || showStoredHeight);
		const int32 panelHeight = (usesStoredHeight ? clampedSize.y : Detail::LayoutProfile().resourcePanel.collapsedHeight);
		const Point clampedPosition = Detail::ClampPanelPosition(position, clampedSize.x, panelHeight, sceneWidth, sceneHeight);
		const Point reclampedSize = ClampResourcePanelSize(clampedSize, clampedPosition, sceneWidth, sceneHeight, expanded);
		return Rect{ clampedPosition.x, clampedPosition.y, reclampedSize.x, (usesStoredHeight ? reclampedSize.y : Detail::LayoutProfile().resourcePanel.collapsedHeight) };
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
		const auto& layout = Detail::LayoutProfile();
		return Rect{ (resourcePanel.rightX() - layout.resourcePanel.resizeHandleSize), (resourcePanel.bottomY() - layout.resourcePanel.resizeHandleSize), layout.resourcePanel.resizeHandleSize, layout.resourcePanel.resizeHandleSize };
	}

	[[nodiscard]] inline Rect ResourcePanelCameraHomeButton(const Rect& resourcePanel)
	{
		const auto& layout = Detail::LayoutProfile();
		return Rect{
			Max(0, (resourcePanel.x - layout.resourcePanel.iconButtonGap - layout.resourcePanel.iconButtonSize)),
			resourcePanel.y,
			layout.resourcePanel.iconButtonSize,
			layout.resourcePanel.iconButtonSize
		};
	}

	[[nodiscard]] inline Rect UiToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		const auto& layout = Detail::LayoutProfile();
		return Rect{ 20, (sceneHeight - layout.bottomControl.yOffset), layout.bottomControl.panelWidth, layout.bottomControl.panelHeight };
	}

	[[nodiscard]] inline Rect BottomControlRevealHotZone(const int32 sceneWidth, const int32 sceneHeight)
	{
		(void)sceneWidth;
		return Rect{ 0, Max(0, (sceneHeight - 160)), 28, 160 };
	}

	[[nodiscard]] inline Rect UiToggleCheckBox(const Rect& uiToggle)
	{
		return Rect{ (uiToggle.x + 10), (uiToggle.y + 8), Detail::LayoutProfile().bottomControl.checkBoxWidth, 36 };
	}

	[[nodiscard]] inline Rect MapModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect uiToggle = UiToggle(sceneWidth, sceneHeight);
		return Rect{ (UiToggleCheckBox(uiToggle).rightX() + Detail::LayoutProfile().bottomControl.buttonGap), (uiToggle.y + 8), 44, 36 };
	}

	[[nodiscard]] inline Rect ModelHeightModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = MapModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect UnitEditorModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = ModelHeightModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 60, 36 };
	}

	[[nodiscard]] inline Rect SkySettingsToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = UnitEditorModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect CameraSettingsToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = SkySettingsToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect TerrainVisualToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = CameraSettingsToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 44, 36 };
	}

	[[nodiscard]] inline Rect FogSettingsToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = TerrainVisualToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 52, 36 };
	}

	[[nodiscard]] inline Rect UiEditModeToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = FogSettingsToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 84, 36 };
	}

	[[nodiscard]] inline Rect ResourceAdjustToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = UiEditModeToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 88, 36 };
	}

	[[nodiscard]] inline Rect EnemyPlanToggle(const int32 sceneWidth, const int32 sceneHeight)
	{
		const Rect anchorRect = ResourceAdjustToggle(sceneWidth, sceneHeight);
		return Rect{ (anchorRect.rightX() + Detail::LayoutProfile().bottomControl.buttonGap), anchorRect.y, 110, 36 };
	}

	[[nodiscard]] inline Rect BottomEditorTextColorsButton(const Rect& anchorToggle)
	{
		const auto& layout = Detail::LayoutProfile();
		return Rect{ (anchorToggle.rightX() + layout.bottomControl.editorIconButtonGap), (anchorToggle.y + (anchorToggle.h - layout.bottomControl.editorIconButtonSize) / 2), layout.bottomControl.editorIconButtonSize, layout.bottomControl.editorIconButtonSize };
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
		return Rect{ panelRect.x, panelRect.y, panelRect.w, Detail::LayoutProfile().shared.accordionHeaderHeight };
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

	[[nodiscard]] inline Rect FogSettingsPanelDragHandle(const Rect& panelRect)
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
