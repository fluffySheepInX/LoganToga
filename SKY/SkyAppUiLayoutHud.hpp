# pragma once
# include "SkyAppUiLayoutCommon.hpp"

namespace SkyAppUiLayout
{
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

    [[nodiscard]] inline Rect BottomPanelSkinButton(const Rect& editorTextColorsButton)
    {
        const auto& layout = Detail::LayoutProfile();
        return Rect{ (editorTextColorsButton.rightX() + layout.bottomControl.editorIconButtonGap), editorTextColorsButton.y, layout.bottomControl.editorIconButtonSize, layout.bottomControl.editorIconButtonSize };
    }

    [[nodiscard]] inline Rect TimeSlider(const int32 sceneWidth, const int32 sceneHeight)
    {
        return Rect{ 20, (sceneHeight - 60), Max(0, (sceneWidth - 40)), 36 };
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
}
