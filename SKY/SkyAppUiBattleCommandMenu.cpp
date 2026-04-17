# include "SkyAppUiMenusInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        using namespace UiMenusInternal;

        constexpr int32 BattleCommandSlotCount = 4;
        constexpr StringView BattleCommandTierUpTexturePath = U"texture/TierUp.png";
        constexpr StringView BattleCommandTabTexturePath1 = U"C:/Users/aband/source/repos/fluffySheepInX/LoganToga/SKY/App/texture/T1.png";
        constexpr StringView BattleCommandTabTexturePath2 = U"C:/Users/aband/source/repos/fluffySheepInX/LoganToga/SKY/App/texture/T2.png";
        constexpr StringView BattleCommandTabTexturePath3 = U"C:/Users/aband/source/repos/fluffySheepInX/LoganToga/SKY/App/texture/T3.png";
        constexpr StringView BattleCommandTabTexturePath4 = U"C:/Users/aband/source/repos/fluffySheepInX/LoganToga/SKY/App/texture/T4.png";

        struct BattleCommandSlotDefinition
        {
            Optional<SapperUnitType> unitType;
            ColorF accentColor;
            StringView portraitLabel;
            StringView shortCode;
        };

        [[nodiscard]] const BattleCommandSlotDefinition& GetBattleCommandSlotDefinition(const size_t slotIndex)
        {
            static const BattleCommandSlotDefinition slotDefinitions[]
            {
                { SapperUnitType::Infantry, ColorF{ 0.74, 0.84, 0.98, 1.0 }, U"Infantry", U"INF" },
                { SapperUnitType::ArcaneInfantry, ColorF{ 0.76, 0.68, 0.98, 1.0 }, U"Arcane Infantry", U"ARC" },
                { SapperUnitType::SugoiCar, ColorF{ 0.96, 0.78, 0.42, 1.0 }, U"Sugoi Car", U"CAR" },
                { none, ColorF{ 0.70, 0.74, 0.82, 1.0 }, U"Reserve", U"RSV" },
            };

            return slotDefinitions[Min(slotIndex, (std::size(slotDefinitions) - 1))];
        }

        [[nodiscard]] Array<size_t> GetBattleCommandTierUnitIndices(const size_t tierIndex)
        {
            switch (tierIndex)
            {
            case 0:
                return{ 0, 1, 2 };

            case 1:
            case 2:
            case 3:
            default:
                return{};
            }
        }

        void DrawBattleCommandHoverGlow(const Rect& rect, const ColorF& glowColor)
        {
            rect.stretched(6).rounded(12).drawFrame(4, 0, ColorF{ glowColor.r, glowColor.g, glowColor.b, 0.18 });
            rect.stretched(3).rounded(10).drawFrame(3, 0, ColorF{ glowColor.r, glowColor.g, glowColor.b, 0.32 });
        }

        [[nodiscard]] bool DrawBattleCommandTabButton(const Rect& rect,
            const int32 tabIndex,
            const bool unlocked,
            const bool selected)
        {
            static const std::array<Texture, BattleCommandSlotCount> tabTextures = {
                Texture{ FilePath{ BattleCommandTabTexturePath1 } },
                Texture{ FilePath{ BattleCommandTabTexturePath2 } },
                Texture{ FilePath{ BattleCommandTabTexturePath3 } },
                Texture{ FilePath{ BattleCommandTabTexturePath4 } },
            };

            const bool hovered = unlocked && rect.mouseOver();
            const Texture& texture = tabTextures[Clamp(tabIndex, 0, (BattleCommandSlotCount - 1))];
            const ColorF fillColor = unlocked
                ? (selected ? ColorF{ 0.16, 0.14, 0.10, 0.26 } : (hovered ? ColorF{ 0.18, 0.22, 0.30, 0.18 } : ColorF{ 0.08, 0.10, 0.14, 0.12 }))
                : ColorF{ 0.04, 0.05, 0.08, 0.48 };
            const ColorF frameColor = selected ? ColorF{ 0.96, 0.84, 0.48, 1.0 } : ColorF{ 0.42, 0.48, 0.58, 0.84 };
            rect.rounded(6).draw(fillColor);
            texture.resized(rect.w, rect.h).draw(rect.pos, ColorF{ 1.0, 1.0, 1.0, unlocked ? 1.0 : 0.42 });
            if (hovered)
            {
                DrawBattleCommandHoverGlow(rect, ColorF{ 0.98, 0.88, 0.54, 1.0 });
                rect.rounded(6).draw(ColorF{ 1.0, 1.0, 1.0, 0.06 });
            }
            rect.rounded(6).drawFrame(2, 0, frameColor);
            if (not unlocked)
            {
                rect.rounded(6).draw(ColorF{ 0.02, 0.03, 0.05, 0.34 });
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

        [[nodiscard]] bool DrawBattleCommandTierUpImageButton(const Rect& rect,
            const bool enabled)
        {
            static const Texture tierUpTexture{ FilePath{ BattleCommandTierUpTexturePath } };

            const bool hovered = enabled && rect.mouseOver();
            const ColorF frameColor = enabled
                ? (hovered ? ColorF{ 0.96, 0.84, 0.48, 1.0 } : ColorF{ 0.72, 0.76, 0.84, 0.92 })
                : ColorF{ 0.34, 0.38, 0.46, 0.84 };
            const double textureAlpha = enabled ? (hovered ? 1.0 : 0.94) : 0.45;

            if (hovered)
            {
                DrawBattleCommandHoverGlow(rect, ColorF{ 0.98, 0.88, 0.54, 1.0 });
            }

            rect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.14, 0.92 });
            tierUpTexture.resized(rect.w, rect.h).draw(rect.pos, ColorF{ 1.0, 1.0, 1.0, textureAlpha });
            rect.rounded(8).drawFrame(2, 0, frameColor);

            if (not enabled)
            {
                rect.rounded(8).draw(ColorF{ 0.02, 0.03, 0.05, 0.38 });
            }

            return hovered && MouseL.down();
        }

        [[nodiscard]] bool DrawBattleCommandUnitButton(const Rect& rect,
            const BattleCommandSlotDefinition& slotDefinition,
            const UnitEditorSettings& unitEditorSettings,
            const bool unlocked,
            const bool selected)
        {
            (void)unitEditorSettings;
            const bool hovered = unlocked && rect.mouseOver();
            const ColorF fillColor = unlocked
                ? (selected ? ColorF{ 0.20, 0.26, 0.36, 0.96 } : (hovered ? ColorF{ 0.18, 0.22, 0.30, 0.96 } : ColorF{ 0.12, 0.15, 0.22, 0.94 }))
                : ColorF{ 0.12, 0.12, 0.14, 0.92 };
            const ColorF frameColor = selected ? slotDefinition.accentColor : ColorF{ 0.34, 0.38, 0.46, 0.92 };
            if (hovered)
            {
                DrawBattleCommandHoverGlow(rect, slotDefinition.accentColor);
            }
            rect.rounded(8).draw(fillColor).drawFrame(2, 0, frameColor);
            const Rect imageRect{ (rect.x + 8), (rect.y + 8), (rect.w - 16), (rect.h - 28) };
            imageRect.rounded(6).draw(slotDefinition.accentColor.lerp(ColorF{ 0.08, 0.10, 0.14, 1.0 }, 0.62));

            if (not unlocked)
            {
                Rect{ rect.x + 6, rect.y + 6, rect.w - 12, rect.h - 12 }.rounded(6).draw(ColorF{ 0.02, 0.03, 0.05, 0.42 });
            }

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

        unlockedSlotCount = Clamp(unlockedSlotCount, 1, BattleCommandSlotCount);
        selectedSlotIndex = Min(selectedSlotIndex, static_cast<size_t>(unlockedSlotCount - 1));

        const Rect& panelRect = panels.blacksmithMenu;
        const Rect innerRect = SkyAppUiLayout::BattleCommandInnerFrame(panelRect);

        UiInternal::DrawPanelFrame(panelRect, U"", ColorF{ 0.10, 0.12, 0.18, 0.94 }, ColorF{ 0.44, 0.48, 0.56, 0.90 }, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::Hud);

        for (int32 index = 0; index < BattleCommandSlotCount; ++index)
        {
            const bool unlocked = (index < unlockedSlotCount);
            if (DrawBattleCommandTabButton(SkyAppUiLayout::BattleCommandSlotButton(panelRect, index), index, unlocked, (selectedSlotIndex == static_cast<size_t>(index))))
            {
                selectedSlotIndex = static_cast<size_t>(index);
            }
        }

        innerRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.82 }).drawFrame(2, 0, ColorF{ 0.44, 0.48, 0.56, 0.90 });

        const Array<size_t> tierUnitIndices = GetBattleCommandTierUnitIndices(selectedSlotIndex);
        const bool selectedTierUnlocked = (selectedSlotIndex < static_cast<size_t>(unlockedSlotCount));

        for (size_t displayedUnitIndex = 0; displayedUnitIndex < tierUnitIndices.size(); ++displayedUnitIndex)
        {
            const BattleCommandSlotDefinition& slotDefinition = GetBattleCommandSlotDefinition(tierUnitIndices[displayedUnitIndex]);
            if (DrawBattleCommandUnitButton(SkyAppUiLayout::BattleCommandUnitButton(panelRect, static_cast<int32>(displayedUnitIndex)),
                slotDefinition,
                unitEditorSettings,
                selectedTierUnlocked,
                false))
            {
                TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, modelHeightSettings, *slotDefinition.unitType, battleCommandMessage);
            }
        }

        const bool canTierUp = (unlockedSlotCount < BattleCommandSlotCount);
        if (DrawBattleCommandTierUpImageButton(SkyAppUiLayout::BattleCommandTierUpButton(panelRect), canTierUp))
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
    }
}
