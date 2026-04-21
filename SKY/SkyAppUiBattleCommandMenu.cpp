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
        constexpr StringView BattleCommandArcaneInfantryIconTexturePath = U"C:/Users/aband/source/repos/fluffySheepInX/LoganToga/SKY/App/texture/t12.png";

        struct BattleCommandSlotDefinition
        {
            Optional<SapperUnitType> unitType;
            ColorF accentColor;
            StringView portraitLabel;
            StringView shortCode;
            StringView tooltipTitle;
            StringView tooltipSummary;
            StringView tooltipDescription;
            StringView tooltipActionHint;
        };

        [[nodiscard]] const BattleCommandSlotDefinition& GetBattleCommandSlotDefinition(const size_t slotIndex)
        {
            static const BattleCommandSlotDefinition slotDefinitions[]
            {
                {
                    SapperUnitType::Infantry, ColorF{ 0.74, 0.84, 0.98, 1.0 }, U"Infantry", U"INF",
                    U"歩兵 (INF)",
                    U"汎用歩兵。資源確保と前線維持に有効",
                    U"標準的な歩兵ユニット。射程は短いが量産しやすく、Mill 建設スキルで前線の生産拠点を構築できます。",
                    U"左クリックで配置を開始",
                },
                {
                    SapperUnitType::ArcaneInfantry, ColorF{ 0.76, 0.68, 0.98, 1.0 }, U"Arcane Infantry", U"ARC",
                    U"魔導兵 (ARC)",
                    U"後衛から味方を回復しつつ攻撃する支援兵",
                    U"魔力を扱う上位歩兵。射程と火力が高く、回復スキルで前線部隊を支援します。コストはやや高め。",
                    U"左クリックで配置を開始",
                },
                {
                    SapperUnitType::SugoiCar, ColorF{ 0.96, 0.78, 0.42, 1.0 }, U"Sugoi Car", U"CAR",
                    U"すごい車 (CAR)",
                    U"装甲と機動力に優れる車両ユニット",
                    U"高耐久・高火力の車両。偵察スキルで一時的に視野を拡張できます。生産コストが高いので運用に注意。",
                    U"左クリックで配置を開始",
                },
                {
                    none, ColorF{ 0.70, 0.74, 0.82, 1.0 }, U"Reserve", U"RSV",
                    U"予備枠",
                    U"未割り当てのスロット",
                    U"将来のユニット用に予約された枠です。現時点では生産できません。",
                    U"",
                },
            };

            return slotDefinitions[Min(slotIndex, (std::size(slotDefinitions) - 1))];
        }

        struct BattleCommandTooltipContent
        {
            Rect anchorRect;
            ColorF accentColor;
            String title;
            String costLine;
            String summary;
            String description;
            String actionHint;
        };

        [[nodiscard]] String BuildBattleCommandCostLine(const BattleCommandSlotDefinition& slotDefinition)
        {
            if (not slotDefinition.unitType)
            {
                return U"";
            }

            const UnitParameters& params = GetDefaultUnitParameters(UnitTeam::Player, *slotDefinition.unitType);
            return U"コスト {:.0f}  HP {:.0f}"_fmt(params.manaCost, params.maxHitPoints);
        }

        [[nodiscard]] Array<String> WrapJapaneseText(const Font& font, const String& text, const double maxWidth)
        {
            Array<String> lines;
            if (text.isEmpty())
            {
                return lines;
            }

            String current;
            for (const char32 ch : text)
            {
                if (ch == U'\n')
                {
                    lines << current;
                    current.clear();
                    continue;
                }

                String candidate = current + String{ ch };
                if (font(candidate).region().w > maxWidth && not current.isEmpty())
                {
                    lines << current;
                    current = String{ ch };
                }
                else
                {
                    current = candidate;
                }
            }

            if (not current.isEmpty())
            {
                lines << current;
            }

            return lines;
        }

        void DrawBattleCommandTooltip(const BattleCommandTooltipContent& tooltip)
        {
            static const Font titleFont{ 16, Typeface::Bold };
            static const Font bodyFont{ 13 };

            constexpr int32 TooltipWidth = 300;
            constexpr int32 PaddingX = 12;
            constexpr int32 PaddingY = 10;
            constexpr int32 LineGap = 4;

            const double bodyMaxWidth = (TooltipWidth - (PaddingX * 2));
            const Array<String> summaryLines = WrapJapaneseText(bodyFont, tooltip.summary, bodyMaxWidth);
            const Array<String> descriptionLines = WrapJapaneseText(bodyFont, tooltip.description, bodyMaxWidth);
            const Array<String> actionLines = WrapJapaneseText(bodyFont, tooltip.actionHint, bodyMaxWidth);

            const int32 titleHeight = static_cast<int32>(titleFont(tooltip.title).region().h);
            const int32 costHeight = tooltip.costLine.isEmpty() ? 0 : (static_cast<int32>(bodyFont(tooltip.costLine).region().h) + LineGap);
            const int32 lineHeight = static_cast<int32>(bodyFont(U"あ").region().h);
            const int32 summaryHeight = (summaryLines.isEmpty() ? 0 : (static_cast<int32>(summaryLines.size()) * (lineHeight + LineGap)));
            const int32 descriptionHeight = (descriptionLines.isEmpty() ? 0 : (static_cast<int32>(descriptionLines.size()) * (lineHeight + LineGap) + 4));
            const int32 actionHeight = (actionLines.isEmpty() ? 0 : (static_cast<int32>(actionLines.size()) * (lineHeight + LineGap) + 6));

            const int32 tooltipHeight = (PaddingY * 2) + titleHeight + costHeight + summaryHeight + descriptionHeight + actionHeight;

            const int32 sceneWidth = Scene::Width();
            int32 tooltipX = (tooltip.anchorRect.x + (tooltip.anchorRect.w - TooltipWidth) / 2);
            tooltipX = Clamp(tooltipX, 8, (sceneWidth - TooltipWidth - 8));
            int32 tooltipY = (tooltip.anchorRect.y - tooltipHeight - 12);
            if (tooltipY < 8)
            {
                tooltipY = (tooltip.anchorRect.y + tooltip.anchorRect.h + 12);
            }

            const Rect tooltipRect{ tooltipX, tooltipY, TooltipWidth, tooltipHeight };
            tooltipRect.rounded(8).draw(ColorF{ 0.04, 0.06, 0.10, 0.96 });
            tooltipRect.rounded(8).drawFrame(2, 0, ColorF{ tooltip.accentColor.r, tooltip.accentColor.g, tooltip.accentColor.b, 0.95 });

            int32 cursorY = (tooltipRect.y + PaddingY);
            titleFont(tooltip.title).draw((tooltipRect.x + PaddingX), cursorY, ColorF{ 0.98, 0.98, 1.0 });
            cursorY += titleHeight;

            if (not tooltip.costLine.isEmpty())
            {
                cursorY += LineGap;
                bodyFont(tooltip.costLine).draw((tooltipRect.x + PaddingX), cursorY, ColorF{ 1.0, 0.86, 0.42 });
                cursorY += static_cast<int32>(bodyFont(tooltip.costLine).region().h);
            }

            for (const auto& line : summaryLines)
            {
                cursorY += LineGap;
                bodyFont(line).draw((tooltipRect.x + PaddingX), cursorY, ColorF{ 0.86, 0.92, 1.0 });
                cursorY += lineHeight;
            }

            if (not descriptionLines.isEmpty())
            {
                cursorY += 4;
                for (const auto& line : descriptionLines)
                {
                    cursorY += LineGap;
                    bodyFont(line).draw((tooltipRect.x + PaddingX), cursorY, ColorF{ 0.94, 0.96, 1.0 });
                    cursorY += lineHeight;
                }
            }

            if (not actionLines.isEmpty())
            {
                cursorY += 6;
                for (const auto& line : actionLines)
                {
                    cursorY += LineGap;
                    bodyFont(line).draw((tooltipRect.x + PaddingX), cursorY, ColorF{ 0.72, 0.88, 0.72 });
                    cursorY += lineHeight;
                }
            }
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

        [[nodiscard]] const Texture* GetBattleCommandUnitIconTexture(const SapperUnitType unitType)
        {
            switch (unitType)
            {
            case SapperUnitType::ArcaneInfantry:
            {
                static const Texture texture{ FilePath{ BattleCommandArcaneInfantryIconTexturePath } };
                return &texture;
            }

            default:
                return nullptr;
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
            if (slotDefinition.unitType)
            {
                if (const Texture* texture = GetBattleCommandUnitIconTexture(*slotDefinition.unitType))
                {
                    texture->resized(imageRect.w, imageRect.h).draw(imageRect.pos, ColorF{ 1.0, 1.0, 1.0, unlocked ? 1.0 : 0.42 });
                }
            }

            if (not unlocked)
            {
                Rect{ rect.x + 6, rect.y + 6, rect.w - 12, rect.h - 12 }.rounded(6).draw(ColorF{ 0.02, 0.03, 0.05, 0.42 });
            }

            return hovered && MouseL.down();
        }

        void DrawBattleCommandMessage(const Rect& panelRect, const TimedMessage& battleCommandMessage)
        {
            if (not battleCommandMessage.isVisible())
            {
                return;
            }

            const Rect baseMessageRect = SkyAppUiLayout::BattleCommandMessageRect(panelRect);
            const Array<String> lines = battleCommandMessage.text.split(U'\n');
            const bool multiline = (1 < lines.size());
            const Rect messageRect = multiline
                ? Rect{ (baseMessageRect.x - 2), (baseMessageRect.y - Max(60, static_cast<int32>(lines.size()) * 16 - 24)), (baseMessageRect.w + 4), Max(84, 10 + static_cast<int32>(lines.size()) * 16) }
                : baseMessageRect;
            messageRect.rounded(6).draw(ColorF{ 0.96, 0.97, 0.99, 0.84 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
            static double copiedUntil = -1.0;
            const Rect copyButtonRect{ (messageRect.rightX() - 58), (messageRect.y + 4), 52, 18 };
            if (DrawTextButton(copyButtonRect, ((Scene::Time() < copiedUntil) ? U"Done" : U"Copy")))
            {
                Clipboard::SetText(battleCommandMessage.text);
                copiedUntil = (Scene::Time() + 1.5);
            }

            if (multiline)
            {
                static const Font diagnosticsFont{ 11 };
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    diagnosticsFont(lines[i]).draw((messageRect.x + 8), (messageRect.y + 3 + static_cast<int32>(i) * 16), ColorF{ 0.12 });
                }
            }
            else
            {
                SimpleGUI::GetFont()(battleCommandMessage.text).draw((messageRect.x + 8), (messageRect.y + 1), ColorF{ 0.12 });
            }
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

        Optional<BattleCommandTooltipContent> hoveredTooltip;

        for (int32 index = 0; index < BattleCommandSlotCount; ++index)
        {
            const bool unlocked = (index < unlockedSlotCount);
            const Rect tabRect = SkyAppUiLayout::BattleCommandSlotButton(panelRect, index);
            if (DrawBattleCommandTabButton(tabRect, index, unlocked, (selectedSlotIndex == static_cast<size_t>(index))))
            {
                selectedSlotIndex = static_cast<size_t>(index);
            }
            if (tabRect.mouseOver())
            {
                BattleCommandTooltipContent tooltip;
                tooltip.anchorRect = tabRect;
                tooltip.accentColor = ColorF{ 0.96, 0.84, 0.48, 1.0 };
                tooltip.title = U"T{} スロット"_fmt(index + 1);
                tooltip.summary = unlocked ? U"このティアの編成を表示します" : U"未解放のティアです";
                tooltip.description = unlocked
                    ? U"クリックでこのティアのユニット一覧に切り替えます。"
                    : U"右下の TierUp ボタンから解放できます。解放には予算が必要です。";
                tooltip.actionHint = unlocked ? U"左クリックで切り替え" : U"";
                hoveredTooltip = tooltip;
            }
        }

        innerRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.82 }).drawFrame(2, 0, ColorF{ 0.44, 0.48, 0.56, 0.90 });

        const Array<size_t> tierUnitIndices = GetBattleCommandTierUnitIndices(selectedSlotIndex);
        const bool selectedTierUnlocked = (selectedSlotIndex < static_cast<size_t>(unlockedSlotCount));

        for (size_t displayedUnitIndex = 0; displayedUnitIndex < tierUnitIndices.size(); ++displayedUnitIndex)
        {
            const BattleCommandSlotDefinition& slotDefinition = GetBattleCommandSlotDefinition(tierUnitIndices[displayedUnitIndex]);
            const Rect unitRect = SkyAppUiLayout::BattleCommandUnitButton(panelRect, static_cast<int32>(displayedUnitIndex));
            if (DrawBattleCommandUnitButton(unitRect,
                slotDefinition,
                unitEditorSettings,
                selectedTierUnlocked,
                false))
            {
                TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, modelHeightSettings, *slotDefinition.unitType, battleCommandMessage);
            }
            if (unitRect.mouseOver())
            {
                BattleCommandTooltipContent tooltip;
                tooltip.anchorRect = unitRect;
                tooltip.accentColor = slotDefinition.accentColor;
                tooltip.title = String{ slotDefinition.tooltipTitle };
                tooltip.costLine = BuildBattleCommandCostLine(slotDefinition);
                tooltip.summary = String{ slotDefinition.tooltipSummary };
                tooltip.description = String{ slotDefinition.tooltipDescription };
                tooltip.actionHint = selectedTierUnlocked
                    ? String{ slotDefinition.tooltipActionHint }
                    : U"このティアは未解放です";
                hoveredTooltip = tooltip;
            }
        }

        const bool canTierUp = (unlockedSlotCount < BattleCommandSlotCount);
        const Rect tierUpRect = SkyAppUiLayout::BattleCommandTierUpButton(panelRect);
        if (DrawBattleCommandTierUpImageButton(tierUpRect, canTierUp))
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
        if (tierUpRect.mouseOver())
        {
            BattleCommandTooltipContent tooltip;
            tooltip.anchorRect = tierUpRect;
            tooltip.accentColor = ColorF{ 0.98, 0.88, 0.54, 1.0 };
            tooltip.title = U"ティアアップ";
            tooltip.costLine = canTierUp
                ? U"必要予算 {:.0f}"_fmt(GetTierUpgradeCost(unlockedSlotCount))
                : U"";
            tooltip.summary = canTierUp
                ? U"次のティアスロットを解放します"
                : U"全てのティアが解放済みです";
            tooltip.description = canTierUp
                ? U"上位ティアの編成を利用できるようになります。予算を消費してロックされた T スロットを開放します。"
                : U"これ以上ティアを解放することはできません。";
            tooltip.actionHint = canTierUp ? U"左クリックで解放" : U"";
            hoveredTooltip = tooltip;
        }

        DrawBattleCommandMessage(panelRect, battleCommandMessage);

        if (hoveredTooltip)
        {
            DrawBattleCommandTooltip(*hoveredTooltip);
        }
    }
}
