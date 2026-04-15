# pragma once
# include "SkyAppUiLayoutCommon.hpp"

namespace SkyAppUiLayout
{
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

    [[nodiscard]] inline Rect EscMenu(const int32 sceneWidth, const int32 sceneHeight)
    {
        return Rect{ ((sceneWidth - 280) / 2), ((sceneHeight - 300) / 2), 280, 300 };
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
}
