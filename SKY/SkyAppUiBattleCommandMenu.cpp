# include "SkyAppUiMenusInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        using namespace UiMenusInternal;

        constexpr int32 BattleCommandSlotCount = 4;

        struct BattleCommandSlotDefinition
        {
            Optional<SapperUnitType> unitType;
            ColorF accentColor;
            StringView portraitLabel;
        };

        [[nodiscard]] const BattleCommandSlotDefinition& GetBattleCommandSlotDefinition(const size_t slotIndex)
        {
            static const BattleCommandSlotDefinition slotDefinitions[]
            {
                { SapperUnitType::Infantry, ColorF{ 0.74, 0.84, 0.98, 1.0 }, U"Infantry" },
                { SapperUnitType::ArcaneInfantry, ColorF{ 0.76, 0.68, 0.98, 1.0 }, U"Arcane Infantry" },
                { SapperUnitType::SugoiCar, ColorF{ 0.96, 0.78, 0.42, 1.0 }, U"Sugoi Car" },
                { none, ColorF{ 0.70, 0.74, 0.82, 1.0 }, U"Reserve" },
            };

            return slotDefinitions[Min(slotIndex, (std::size(slotDefinitions) - 1))];
        }

        [[nodiscard]] String GetBattleCommandTierUpLabel(const int32 unlockedSlotCount)
        {
            if (unlockedSlotCount >= BattleCommandSlotCount)
            {
                return U"Tier up [MAX]";
            }

            return U"Tier up ({:.0f})"_fmt(GetTierUpgradeCost(unlockedSlotCount));
        }

        [[nodiscard]] String GetBattleCommandPortraitLabel(const BattleCommandSlotDefinition& slotDefinition)
        {
            if (not slotDefinition.unitType)
            {
                return String{ slotDefinition.portraitLabel };
            }

            return String{ GetUnitDisplayName(*slotDefinition.unitType) };
        }

        [[nodiscard]] bool DrawBattleCommandTabButton(const Rect& rect,
            const String& label,
            const bool unlocked,
            const bool selected)
        {
            static const Font tabFont{ 18, Typeface::Bold };
            const bool hovered = unlocked && rect.mouseOver();
            const ColorF fillColor = unlocked
                ? (selected ? ColorF{ 0.40, 0.54, 0.82, 0.96 } : (hovered ? ColorF{ 0.86, 0.90, 0.98, 0.98 } : ColorF{ 0.95, 0.96, 0.98, 0.94 }))
                : ColorF{ 0.32, 0.36, 0.44, 0.90 };
            const ColorF frameColor = selected ? ColorF{ 0.16, 0.26, 0.48, 1.0 } : ColorF{ 0.26, 0.30, 0.36, 0.90 };
            rect.rounded(6).draw(fillColor).drawFrame(2, 0, frameColor);
            tabFont(label).drawAt(rect.center().movedBy(0, unlocked ? 0 : -8), unlocked ? ColorF{ 0.08, 0.12, 0.18 } : ColorF{ 0.78, 0.82, 0.90 });

            if (not unlocked)
            {
                SimpleGUI::GetFont()(U"LOCK").drawAt(rect.center().movedBy(0, 12), ColorF{ 0.82, 0.86, 0.94, 0.88 });
            }

            return hovered && MouseL.down();
        }

        [[nodiscard]] bool DrawBattleCommandActionButton(const Rect& rect,
            const String& label,
            const bool enabled,
            const ColorF& accentColor = ColorF{ 0.34, 0.46, 0.72, 1.0 })
        {
            static const Font buttonFont{ 18, Typeface::Bold };
            const bool hovered = enabled && rect.mouseOver();
            const ColorF fillColor = enabled
                ? (hovered ? accentColor.lerp(ColorF{ 1.0, 1.0, 1.0, 1.0 }, 0.18) : accentColor)
                : ColorF{ 0.42, 0.46, 0.50, 0.78 };
            rect.rounded(6).draw(fillColor).drawFrame(2, 0, enabled ? ColorF{ 0.12, 0.18, 0.28, 0.94 } : ColorF{ 0.24, 0.26, 0.30, 0.84 });
            buttonFont(label).drawAt(rect.center(), enabled ? ColorF{ 0.98 } : ColorF{ 0.86, 0.88, 0.90 });
            return hovered && MouseL.down();
        }
    }

    void DrawBattleCommandMenu(const SkyAppPanels& panels,
        Array<SpawnedSapper>& spawnedSappers,
        const MapData& mapData,
        const Vec3& playerBasePosition,
        const Vec3& rallyPoint,
        ResourceStock& playerResources,
        size_t& selectedSlotIndex,
        int32& unlockedSlotCount,
        const UnitEditorSettings& unitEditorSettings,
        const ModelHeightSettings& modelHeightSettings,
        TimedMessage& battleCommandMessage)
    {
        using namespace UiMenusInternal;

        const Font& font = SimpleGUI::GetFont();

        unlockedSlotCount = Clamp(unlockedSlotCount, 1, BattleCommandSlotCount);
        selectedSlotIndex = Min(selectedSlotIndex, static_cast<size_t>(unlockedSlotCount - 1));

        const Rect& panelRect = panels.blacksmithMenu;
        const Rect portraitRect = SkyAppUiLayout::BattleCommandPortrait(panelRect);
        const Rect detailRect = SkyAppUiLayout::BattleCommandDetail(panelRect);

        UiInternal::DrawPanelFrame(panelRect, U"Battle Command", ColorF{ 0.97, 0.97, 0.98, 0.96 }, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::Hud);
        font(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panelRect.x + 16), (panelRect.y + 18), ColorF{ 0.12 });
        font(U"魔力: {:.0f} / 解放 {}/{}"_fmt(playerResources.mana, unlockedSlotCount, BattleCommandSlotCount)).draw((panelRect.x + 162), (panelRect.y + 18), ColorF{ 0.12 });

        for (int32 index = 0; index < BattleCommandSlotCount; ++index)
        {
            const bool unlocked = (index < unlockedSlotCount);
            if (DrawBattleCommandTabButton(SkyAppUiLayout::BattleCommandSlotButton(panelRect, index), U"T{}"_fmt(index + 1), unlocked, (selectedSlotIndex == static_cast<size_t>(index))))
            {
                selectedSlotIndex = static_cast<size_t>(index);
            }
        }

        const BattleCommandSlotDefinition& selectedSlot = GetBattleCommandSlotDefinition(selectedSlotIndex);
        const Optional<SapperUnitType>& selectedUnitType = selectedSlot.unitType;
        const ColorF accentColor = selectedSlot.accentColor;
        portraitRect.rounded(8).draw(accentColor.lerp(ColorF{ 0.08, 0.10, 0.14, 1.0 }, 0.68)).drawFrame(2, 0, accentColor);
        font(GetBattleCommandPortraitLabel(selectedSlot)).drawAt(portraitRect.center().movedBy(0, -10), Palette::White);
        font(selectedUnitType ? U"Unit image / icon fallback" : U"Slot reserved").drawAt(portraitRect.center().movedBy(0, 28), ColorF{ 0.90, 0.93, 0.98 });

        detailRect.rounded(8).draw(ColorF{ 0.95, 0.96, 0.98, 0.98 }).drawFrame(2, 0, accentColor);
        if (selectedUnitType)
        {
            const UnitParameters& unitParameters = GetUnitParameters(unitEditorSettings, UnitTeam::Player, *selectedUnitType);
            font(GetUnitDisplayName(*selectedUnitType)).draw((detailRect.x + 12), (detailRect.y + 10), ColorF{ 0.12 });
            font(U"消費魔力: {:.0f}"_fmt(unitParameters.manaCost)).draw((detailRect.x + 12), (detailRect.y + 34), ColorF{ 0.18 });
            font(U"HP {:.0f} / 攻撃 {:.0f}"_fmt(unitParameters.maxHitPoints, unitParameters.attackDamage)).draw((detailRect.x + 12), (detailRect.y + 56), ColorF{ 0.18 });
            font(U"射程 {:.1f} / 移動 {:.1f}"_fmt(unitParameters.attackRange, unitParameters.moveSpeed)).draw((detailRect.x + 12), (detailRect.y + 78), ColorF{ 0.18 });

            if (DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandPrimaryActionButton(panelRect), U"出撃", true, accentColor))
            {
                TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, modelHeightSettings, *selectedUnitType, battleCommandMessage);
            }
        }
        else
        {
            font(U"T4 Reserve").draw((detailRect.x + 12), (detailRect.y + 10), ColorF{ 0.12 });
            font(U"将来の拡張スロットです").draw((detailRect.x + 12), (detailRect.y + 38), ColorF{ 0.20 });
            font(U"Tier up で順番にアンロック").draw((detailRect.x + 12), (detailRect.y + 62), ColorF{ 0.20 });
            DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandPrimaryActionButton(panelRect), U"準備中", false, accentColor);
        }

        if (DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandTierUpButton(panelRect), GetBattleCommandTierUpLabel(unlockedSlotCount), true, ColorF{ 0.34, 0.38, 0.46, 1.0 }))
        {
            switch (TryPurchaseTierUpgrade(playerResources.budget, unlockedSlotCount, 1, BattleCommandSlotCount))
            {
            case TierUpgradePurchaseResult::Succeeded:
                selectedSlotIndex = static_cast<size_t>(unlockedSlotCount - 1);
                battleCommandMessage.show(U"T{} を解放"_fmt(unlockedSlotCount));
                break;

            case TierUpgradePurchaseResult::AlreadyMax:
                battleCommandMessage.show(U"スロットは最大です");
                break;

            case TierUpgradePurchaseResult::InsufficientBudget:
                battleCommandMessage.show(U"予算不足");
                break;
            }
        }

        const Rect messageRect = SkyAppUiLayout::BattleCommandMessageRect(panelRect);
        messageRect.rounded(8).draw(ColorF{ 0.92, 0.94, 0.98, 0.70 }).drawFrame(1, 0, ColorF{ 0.68, 0.74, 0.82, 0.72 });
        font(U"押した Tier が上から順に解放されます").draw((messageRect.x + 10), (messageRect.y + 8), ColorF{ 0.18 });
        if (battleCommandMessage.isVisible())
        {
            font(battleCommandMessage.text).draw((messageRect.x + 10), (messageRect.y + 30), ColorF{ 0.12 });
        }
    }
}
