# pragma once
# include "SkyAppUiLayoutCommon.hpp"

namespace SkyAppUiLayout
{
    [[nodiscard]] inline int32 ClampBattleCommandIconSize(const int32 iconSize)
    {
        return (iconSize <= 112) ? 96 : 128;
    }

    [[nodiscard]] inline int32 BattleCommandIconSize(const Rect& panelRect)
    {
        return ClampBattleCommandIconSize(((panelRect.w - 72) / 3));
    }

    [[nodiscard]] inline Rect BlacksmithMenu(const int32 sceneWidth, const int32 sceneHeight, const int32 iconSize = 128)
    {
        const int32 resolvedIconSize = ClampBattleCommandIconSize(iconSize);
        const int32 panelWidth = (72 + resolvedIconSize * 3);
        const int32 panelHeight = (172 + resolvedIconSize * 2);
        return Rect{ (sceneWidth - (panelWidth + 20)), Max(20, (sceneHeight - (panelHeight + 20))), panelWidth, panelHeight };
    }

    [[nodiscard]] inline Rect BattleCommandSlotButton(const Rect& panelRect, const int32 index)
    {
        constexpr int32 SlotWidth = 80;
        constexpr int32 SlotHeight = 64;
        constexpr int32 SlotGap = 6;
        return Rect{ (panelRect.x + 16 + index * (SlotWidth + SlotGap)), (panelRect.y + 38), SlotWidth, SlotHeight };
    }

    [[nodiscard]] inline Rect BattleCommandInnerFrame(const Rect& panelRect)
    {
        return Rect{ (panelRect.x + 12), (panelRect.y + 114), (panelRect.w - 24), (panelRect.h - 130) };
    }

    [[nodiscard]] inline Rect BattleCommandUnitButton(const Rect& panelRect, const int32 index)
    {
        const int32 cardSize = BattleCommandIconSize(panelRect);
        constexpr int32 CardGap = 6;
        const Rect innerRect = BattleCommandInnerFrame(panelRect);
        return Rect{ (innerRect.x + 16 + index * (cardSize + CardGap)), (innerRect.y + 18), cardSize, cardSize };
    }

    [[nodiscard]] inline Rect BattleCommandTierUpButton(const Rect& panelRect)
    {
        const Rect innerRect = BattleCommandInnerFrame(panelRect);
        const int32 cardSize = BattleCommandIconSize(panelRect);
        return Rect{ (innerRect.rightX() - (cardSize + 12)), (innerRect.bottomY() - (cardSize + 12)), cardSize, cardSize };
    }

    [[nodiscard]] inline Rect BattleCommandMessageRect(const Rect& panelRect)
    {
        const Rect innerRect = BattleCommandInnerFrame(panelRect);
        const int32 cardSize = BattleCommandIconSize(panelRect);
        return Rect{ (innerRect.x + 12), (innerRect.bottomY() - 24), Max(0, (innerRect.w - (cardSize + 36))), 20 };
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
