# include "MainUi.hpp"
# include "MainSettings.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUiInternal.hpp"
# include "SkyAppUiPanelFrameInternal.hpp"

namespace MainSupport
{
    using SkyAppText::TextId;
    using SkyAppText::Tr;
    using SkyAppText::TrFormat;

    namespace
    {
        inline constexpr double ScaleButtonStepLarge = 0.5;
        inline constexpr double ScaleButtonStepMedium = 0.1;
        inline constexpr double ScaleButtonStepSmall = 0.01;
        inline constexpr double ModelHeightDragRoundStep = 0.001;
        inline constexpr int32 TextureColumnWidth = 140;
        inline constexpr int32 TextureColumnCollapsedWidth = 22;
        inline constexpr int32 TextureColumnGap = 12;
        inline constexpr int32 TextureColumnHandleSize = 18;

        [[nodiscard]] StringView ToModelHeightTargetLabel(const UnitRenderModel renderModel)
        {
            return GetUnitRenderModelLabel(renderModel);
        }

        [[nodiscard]] StringView ToAnimationRoleLabel(const UnitModelAnimationRole role)
        {
            switch (role)
            {
            case UnitModelAnimationRole::Move:
                return U"Move";

            case UnitModelAnimationRole::Attack:
                return U"Attack";

            case UnitModelAnimationRole::Idle:
            default:
                return U"Idle";
            }
        }

        [[nodiscard]] double GetActiveModelScale(const ModelHeightSettings& modelHeightSettings, const UnitRenderModel renderModel)
        {
            return GetModelScale(modelHeightSettings, renderModel);
        }

        [[nodiscard]] double GetModelHeightWorldY(const UnitRenderModel renderModel,
            const std::array<Vec3, UnitRenderModelCount>& previewRenderPositions)
        {
            return previewRenderPositions[GetUnitRenderModelIndex(renderModel)].y;
        }

        [[nodiscard]] StringView ToTextureTargetLabel(const TireTrackTextureSegment segment)
        {
            return GetTireTrackTextureSegmentLabel(segment);
        }

        [[nodiscard]] double RoundModelHeightEditorValue(const double value, const double roundStep)
        {
            if (roundStep <= 0.0)
            {
                return value;
            }

            return (Math::Round(value / roundStep) * roundStep);
        }

        [[nodiscard]] String ToTexturePrimarySummary(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
        {
            return U"Y {:.3f} / A {:.2f}"_fmt(GetTireTrackYOffset(settings, segment), GetTireTrackOpacity(settings, segment));
        }

        [[nodiscard]] String ToTextureSecondarySummary(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
        {
            return U"S {:.2f} / W {:.2f}"_fmt(GetTireTrackSoftness(settings, segment), GetTireTrackWarmth(settings, segment));
        }

        [[nodiscard]] String ToTextureStatusSummary(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
        {
            return U"Y {:.3f} / Opacity {:.2f} / Softness {:.2f} / Warmth {:.2f}"_fmt(
                GetTireTrackYOffset(settings, segment),
                GetTireTrackOpacity(settings, segment),
                GetTireTrackSoftness(settings, segment),
                GetTireTrackWarmth(settings, segment));
        }

        [[nodiscard]] String ToCompactTextureName(const StringView label)
        {
            constexpr size_t MaxVisibleCharacters = 7;

            if (label.size() <= MaxVisibleCharacters)
            {
                return String{ label };
            }

            return (label.substr(0, MaxVisibleCharacters) + U"...");
        }

        [[nodiscard]] String ToCompactLabel(const StringView label, const size_t maxVisibleCharacters)
        {
            if (label.size() <= maxVisibleCharacters)
            {
                return String{ label };
            }

            return (label.substr(0, maxVisibleCharacters) + U"...");
        }

        void DrawEditorSectionLabel(const Rect& panelRect, StringView label)
        {
            static const Font sectionFont{ 16, Typeface::Bold };
            sectionFont(label).draw((panelRect.x + 4), panelRect.y, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
        }

        void DrawSelectionCard(const Rect& rect, StringView title, StringView primarySummary, StringView secondarySummary, const bool selected)
        {
            const bool hovered = rect.mouseOver();
            rect.rounded(8).draw(selected
                ? ColorF{ 0.33, 0.53, 0.82, 0.96 }
                : (hovered ? ColorF{ 0.96, 0.97, 0.99, 0.90 } : ColorF{ 0.98, 0.97, 0.95, 0.82 }))
                .drawFrame(1.0, 0.0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52, 0.84 });

            SimpleGUI::GetFont()(title).draw((rect.x + 10), (rect.y + 8), selected ? ColorF{ 0.98 } : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            SimpleGUI::GetFont()(primarySummary).draw((rect.x + 10), (rect.y + 28), selected ? ColorF{ 0.96 } : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            SimpleGUI::GetFont()(secondarySummary).draw((rect.x + 10), (rect.y + 48), selected ? ColorF{ 0.93 } : SkyAppSupport::UiInternal::EditorTextOnCardSecondaryColor());
        }

        bool DrawMiniHandleButton(const Rect& rect, const StringView label)
        {
            const bool hovered = rect.mouseOver();
            rect.rounded(4).draw(hovered ? ColorF{ 0.96, 0.97, 0.99, 0.92 } : ColorF{ 0.88, 0.90, 0.94, 0.86 })
                .drawFrame(1.0, 0.0, ColorF{ 0.42, 0.48, 0.58, 0.86 });
            SimpleGUI::GetFont()(label).drawAt(rect.center(), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            return hovered && MouseL.down();
        }

        void DrawCompactSelectionCard(const Rect& rect, const StringView title, const bool selected)
        {
            const bool hovered = rect.mouseOver();
            rect.rounded(8).draw(selected
                ? ColorF{ 0.33, 0.53, 0.82, 0.96 }
                : (hovered ? ColorF{ 0.96, 0.97, 0.99, 0.90 } : ColorF{ 0.98, 0.97, 0.95, 0.82 }))
                .drawFrame(1.0, 0.0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52, 0.84 });
            SimpleGUI::GetFont()(title).draw((rect.x + 12), (rect.y + 16), selected ? ColorF{ 0.98 } : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
        }

        void CopyTextureSegmentSettings(ModelHeightSettings& destination,
            const ModelHeightSettings& source,
            const TireTrackTextureSegment segment)
        {
            GetTireTrackYOffset(destination, segment) = GetTireTrackYOffset(source, segment);
            GetTireTrackOpacity(destination, segment) = GetTireTrackOpacity(source, segment);
            GetTireTrackSoftness(destination, segment) = GetTireTrackSoftness(source, segment);
            GetTireTrackWarmth(destination, segment) = GetTireTrackWarmth(source, segment);
        }

        void DrawDragValueRect(const Rect& rect,
            const int32 controlId,
            const StringView label,
            double& value,
            const double minValue,
            const double maxValue,
            const double roundStep)
        {
            static Optional<int32> activeControlId;

            const bool hovered = rect.mouseOver();
            if (MouseL.down() && hovered)
            {
                activeControlId = controlId;
            }

            const bool active = (activeControlId && (*activeControlId == controlId));
            const RectF sliderTrackRect{ (rect.x + 12.0), (rect.bottomY() - 12.0), (rect.w - 24.0), 8.0 };

            if (active)
            {
                if (MouseL.pressed())
                {
                    const double cursorRatio = Math::Saturate((Cursor::PosF().x - sliderTrackRect.x) / Max(1.0, sliderTrackRect.w));
                    value = Clamp(RoundModelHeightEditorValue((minValue + (maxValue - minValue) * cursorRatio), roundStep), minValue, maxValue);
                }
                else
                {
                    activeControlId.reset();
                }
            }
            else
            {
                value = Clamp(RoundModelHeightEditorValue(value, roundStep), minValue, maxValue);
            }

            const double ratio = Math::Saturate((value - minValue) / Max(0.0001, (maxValue - minValue)));
            rect.rounded(8).draw(active
                ? ColorF{ 0.90, 0.94, 1.0, 0.92 }
                : (hovered ? ColorF{ 0.98, 0.99, 1.0, 0.84 } : ColorF{ 0.96, 0.97, 0.99, 0.78 }))
                .drawFrame(1.0, 0.0, active ? ColorF{ 0.28, 0.46, 0.74, 0.96 } : ColorF{ 0.58, 0.64, 0.72, 0.84 });

            SimpleGUI::GetFont()(label).draw((rect.x + 12), (rect.y + 6), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            SimpleGUI::GetFont()(U"{:.3f}"_fmt(value)).draw((rect.rightX() - 74), (rect.y + 6), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

            if (active)
            {
                sliderTrackRect.rounded(4).draw(ColorF{ 0.12, 0.14, 0.18, 0.92 });
                RectF{ sliderTrackRect.pos, (sliderTrackRect.w * ratio), sliderTrackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, 0.95 });
                sliderTrackRect.rounded(4).drawFrame(1.0, ColorF{ 0.78, 0.86, 0.96, 0.72 });
                RectF knobRect{ Arg::center = Vec2{ (sliderTrackRect.x + sliderTrackRect.w * ratio), sliderTrackRect.centerY() }, 14, 22 };
                knobRect.rounded(4).draw(ColorF{ 0.94, 0.97, 1.0 }).drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
            }
            else
            {
                SimpleGUI::GetFont()(TrFormat(TextId::CommonDragToAdjustRange, U"{:.2f}, {:.2f}"_fmt(minValue, maxValue))).draw((rect.x + 12), (rect.y + 28), SkyAppSupport::UiInternal::EditorTextOnCardSecondaryColor());
            }
        }

        void DrawCompactTextureParameterRow(const Rect& rect,
            const int32 controlId,
            const StringView label,
            double& value,
            const double minValue,
            const double maxValue,
            const double roundStep,
            const int32 decimals)
        {
            static Optional<int32> activeControlId;

            const RectF sliderTrackRect{ (rect.x + 12.0), (rect.bottomY() - 10.0), (rect.w - 24.0), 6.0 };
            const bool hovered = rect.mouseOver() || sliderTrackRect.stretched(0.0, 8.0).mouseOver();

            if (MouseL.down() && hovered)
            {
                activeControlId = controlId;
            }

            const bool active = (activeControlId && (*activeControlId == controlId));
            if (active)
            {
                if (MouseL.pressed())
                {
                    const double cursorRatio = Math::Saturate((Cursor::PosF().x - sliderTrackRect.x) / Max(1.0, sliderTrackRect.w));
                    value = Clamp(RoundModelHeightEditorValue((minValue + (maxValue - minValue) * cursorRatio), roundStep), minValue, maxValue);
                }
                else
                {
                    activeControlId.reset();
                }
            }
            else
            {
                value = Clamp(RoundModelHeightEditorValue(value, roundStep), minValue, maxValue);
            }

            const double ratio = Math::Saturate((value - minValue) / Max(0.0001, (maxValue - minValue)));
            rect.rounded(8).draw(active
                ? ColorF{ 0.90, 0.94, 1.0, 0.92 }
                : (hovered ? ColorF{ 0.98, 0.99, 1.0, 0.84 } : ColorF{ 0.96, 0.97, 0.99, 0.78 }))
                .drawFrame(1.0, 0.0, active ? ColorF{ 0.28, 0.46, 0.74, 0.96 } : ColorF{ 0.58, 0.64, 0.72, 0.84 });

            SimpleGUI::GetFont()(label).draw((rect.x + 12), (rect.y + 8), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

            String valueText;
            switch (decimals)
            {
            case 1:
                valueText = U"{:.1f}"_fmt(value);
                break;

            case 2:
                valueText = U"{:.2f}"_fmt(value);
                break;

            case 3:
            default:
                valueText = U"{:.3f}"_fmt(value);
                break;
            }

            SimpleGUI::GetFont()(valueText).draw((rect.rightX() - 82), (rect.y + 8), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            sliderTrackRect.rounded(4).draw(ColorF{ 0.14, 0.16, 0.20, active ? 0.94 : 0.72 });
            RectF{ sliderTrackRect.pos, (sliderTrackRect.w * ratio), sliderTrackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, active ? 0.98 : 0.88 });

            if (active || hovered)
            {
                RectF knobRect{ Arg::center = Vec2{ (sliderTrackRect.x + sliderTrackRect.w * ratio), sliderTrackRect.centerY() }, 10, 16 };
                knobRect.rounded(4).draw(ColorF{ 0.94, 0.97, 1.0 }).drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
            }
        }
    }

    void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
        UnitRenderModel& activeRenderModel,
        bool& textureMode,
        TireTrackTextureSegment& activeTextureSegment,
        UnitModelAnimationRole& previewAnimationRole,
        UnitModel& activeModel,
        String& modelHeightMessage,
        double& modelHeightMessageUntil,
        const Rect& panelRect,
        const std::array<Vec3, UnitRenderModelCount>& previewRenderPositions)
    {
        static bool modelListCollapsed = false;
        static bool textureInfoCollapsed = false;
        static bool textureSegmentCollapsed = false;

        const int32 modelListWidth = ((not textureMode) && modelListCollapsed) ? TextureColumnCollapsedWidth : 156;
        const int32 modelDetailOffset = ((not textureMode) ? (modelListWidth + 8) : 164);
        const Rect listPanel{ panelRect.x, panelRect.y, modelListWidth, panelRect.h };
        const Rect detailPanel{ (panelRect.x + modelDetailOffset), panelRect.y, (panelRect.w - modelDetailOffset), panelRect.h };
        double& activeOffset = GetModelHeightOffset(modelHeightSettings, activeRenderModel);
        double& activeScale = GetModelScale(modelHeightSettings, activeRenderModel);
        double& activeTextureYOffset = GetTireTrackYOffset(modelHeightSettings, activeTextureSegment);
        double& activeTextureOpacity = GetTireTrackOpacity(modelHeightSettings, activeTextureSegment);
        double& activeTextureSoftness = GetTireTrackSoftness(modelHeightSettings, activeTextureSegment);
        double& activeTextureWarmth = GetTireTrackWarmth(modelHeightSettings, activeTextureSegment);
        activeOffset = Clamp(activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax);
        activeScale = Clamp(activeScale, ModelScaleMin, ModelScaleMax);
        activeTextureYOffset = Clamp(activeTextureYOffset, TireTrackYOffsetMin, TireTrackYOffsetMax);
        activeTextureOpacity = Clamp(activeTextureOpacity, TireTrackOpacityMin, TireTrackOpacityMax);
        activeTextureSoftness = Clamp(activeTextureSoftness, TireTrackSoftnessMin, TireTrackSoftnessMax);
        activeTextureWarmth = Clamp(activeTextureWarmth, TireTrackWarmthMin, TireTrackWarmthMax);

        SkyAppSupport::UiInternal::DrawNinePatchPanelFrame(panelRect, Tr(TextId::ModelHeightPanelTitle), ColorF{ 1.0, 0.92 });
        const Rect modelToggleRect{ (listPanel.x + 12), (listPanel.y + 36), 62, 26 };
        const Rect textureToggleRect{ (listPanel.x + 82), (listPanel.y + 36), 62, 26 };
        modelToggleRect.rounded(6).draw(textureMode ? ColorF{ 0.96, 0.97, 0.99, 0.82 } : ColorF{ 0.33, 0.53, 0.82 })
            .drawFrame(1.0, 0.0, textureMode ? ColorF{ 0.58, 0.64, 0.72, 0.84 } : ColorF{ 0.20, 0.32, 0.52 });
        textureToggleRect.rounded(6).draw(textureMode ? ColorF{ 0.33, 0.53, 0.82 } : ColorF{ 0.96, 0.97, 0.99, 0.82 })
            .drawFrame(1.0, 0.0, textureMode ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.64, 0.72, 0.84 });
        SimpleGUI::GetFont()(U"model").drawAt(modelToggleRect.center(), textureMode ? ColorF{ 0.14 } : ColorF{ 0.98 });
        SimpleGUI::GetFont()(U"texture").drawAt(textureToggleRect.center(), textureMode ? ColorF{ 0.98 } : ColorF{ 0.14 });

        if (modelToggleRect.mouseOver() && MouseL.down())
        {
            textureMode = false;
        }

        if (textureToggleRect.mouseOver() && MouseL.down())
        {
            textureMode = true;
        }

        SimpleGUI::GetFont()(textureMode ? U"Textures" : Tr(TextId::ModelHeightTargets)).draw((listPanel.x + 16), (listPanel.y + 70), ColorF{ 0.18 });

        int32 targetIndex = 0;
        if (textureMode)
        {
            const int32 contentTop = (panelRect.y + 70);
            const int32 contentHeight = (panelRect.h - 82);
            const int32 textureInfoWidth = (textureInfoCollapsed ? TextureColumnCollapsedWidth : TextureColumnWidth);
            const int32 textureSegmentWidth = (textureSegmentCollapsed ? TextureColumnCollapsedWidth : TextureColumnWidth);
            const Rect textureInfoPanel{ (panelRect.x + 12), contentTop, textureInfoWidth, contentHeight };
            const Rect textureSegmentPanel{ (textureInfoPanel.rightX() + TextureColumnGap), contentTop, textureSegmentWidth, contentHeight };
            const Rect textureDetailPanel{ (textureSegmentPanel.rightX() + TextureColumnGap), contentTop, Max(180, (panelRect.rightX() - textureSegmentPanel.rightX() - 20)), contentHeight };
            const int32 detailX = (textureDetailPanel.x + 8);
            const int32 detailWidth = Max(180, (textureDetailPanel.w - 16));
            const int32 detailButtonWidth = Max(88, ((detailWidth - 8) / 2));
            const int32 detailRowHeight = 48;
            const int32 detailRowGap = 12;
            const int32 detailRowsTop = (textureDetailPanel.y + 52);
            const int32 actionRowsTop = (textureDetailPanel.bottomY() - 78);

            if (not textureInfoCollapsed)
            {
                Rect{ (textureInfoPanel.rightX() + 5), (panelRect.y + 8), 1, (panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
                DrawEditorSectionLabel(textureInfoPanel, U"Texture");
                DrawCompactSelectionCard(Rect{ (textureInfoPanel.x + 4), (textureInfoPanel.y + 28), (textureInfoPanel.w - 8), 76 }, ToCompactTextureName(U"tireTrackTextureSet"), true);

                if (DrawMiniHandleButton(Rect{ (textureInfoPanel.rightX() - TextureColumnHandleSize - 6), (textureInfoPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
                {
                    textureInfoCollapsed = true;
                }
            }
            else if (DrawMiniHandleButton(Rect{ textureInfoPanel.x, (textureInfoPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
            {
                textureInfoCollapsed = false;
            }

            if (not textureSegmentCollapsed)
            {
                Rect{ (textureSegmentPanel.rightX() + 5), (panelRect.y + 8), 1, (panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
                DrawEditorSectionLabel(textureSegmentPanel, U"Segment");
                for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
                {
                    const Rect buttonRect{ (textureSegmentPanel.x + 4), (textureSegmentPanel.y + 28 + targetIndex * 64), (textureSegmentPanel.w - 8), 52 };
                    const bool selected = (activeTextureSegment == segment);
                    DrawCompactSelectionCard(buttonRect, ToTextureTargetLabel(segment), selected);

                    if (buttonRect.mouseOver() && MouseL.down())
                    {
                        activeTextureSegment = segment;
                    }

                    ++targetIndex;
                }

                if (DrawMiniHandleButton(Rect{ (textureSegmentPanel.rightX() - TextureColumnHandleSize - 6), (textureSegmentPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
                {
                    textureSegmentCollapsed = true;
                }
            }
            else if (DrawMiniHandleButton(Rect{ textureSegmentPanel.x, (textureSegmentPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
            {
                textureSegmentCollapsed = false;
            }

            DrawEditorSectionLabel(textureDetailPanel, U"Parameters");
            SimpleGUI::GetFont()(ToTextureTargetLabel(activeTextureSegment)).draw((detailX + 2), (textureDetailPanel.y + 28), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
            DrawCompactTextureParameterRow(Rect{ detailX, detailRowsTop, detailWidth, detailRowHeight }, 10, U"offset", activeTextureYOffset, TireTrackYOffsetMin, TireTrackYOffsetMax, ModelHeightDragRoundStep, 3);
            DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap)), detailWidth, detailRowHeight }, 11, U"opacity", activeTextureOpacity, TireTrackOpacityMin, TireTrackOpacityMax, 0.01, 3);
            DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap) * 2), detailWidth, detailRowHeight }, 12, U"softness", activeTextureSoftness, TireTrackSoftnessMin, TireTrackSoftnessMax, 0.01, 3);
            DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap) * 3), detailWidth, detailRowHeight }, 13, U"warmth", activeTextureWarmth, TireTrackWarmthMin, TireTrackWarmthMax, 0.01, 3);

            if (DrawTextButton(Rect{ detailX, actionRowsTop, detailButtonWidth, 32 }, U"Reset Segment"))
            {
                ModelHeightSettings defaultSettings;
                CopyTextureSegmentSettings(modelHeightSettings, defaultSettings, activeTextureSegment);
                modelHeightMessage = U"Segment reset";
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (DrawTextButton(Rect{ (detailX + detailButtonWidth + 8), actionRowsTop, detailButtonWidth, 32 }, U"Reload Segment"))
            {
                const ModelHeightSettings loadedSettings = LoadModelHeightSettings();
                CopyTextureSegmentSettings(modelHeightSettings, loadedSettings, activeTextureSegment);
                modelHeightMessage = Tr(TextId::ModelHeightReloaded);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (DrawTextButton(Rect{ detailX, (actionRowsTop + 40), detailButtonWidth, 32 }, U"Reset All"))
            {
                modelHeightSettings = {};
                modelHeightMessage = Tr(TextId::ModelHeightOffsetsScalesReset);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (DrawTextButton(Rect{ (detailX + detailButtonWidth + 8), (actionRowsTop + 40), detailButtonWidth, 32 }, U"Save All"))
            {
                modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
                    ? TrFormat(TextId::ModelHeightSavedWithPath, ModelHeightSettingsPath)
                    : Tr(TextId::ModelHeightSaveFailed);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (Scene::Time() < modelHeightMessageUntil)
            {
                const Rect statusRect{ detailX, (actionRowsTop - 40), detailWidth, 28 };
                statusRect.rounded(6).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
                SimpleGUI::GetFont()(modelHeightMessage).draw((statusRect.x + 10), (statusRect.y + 2), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
            }
        }
        else
        {
            const int32 detailX = (detailPanel.x + 10);
            const int32 detailWidth = Max(180, (detailPanel.w - 20));
            const int32 compactRowHeight = 48;
            const int32 compactRowGap = 12;
            const int32 sectionTop = (detailPanel.y + 52);

            if (not modelListCollapsed)
            {
                Rect{ (listPanel.rightX() + 3), (panelRect.y + 8), 1, (panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
                for (const UnitRenderModel renderModel : GetUnitRenderModels())
                {
                    const Rect buttonRect{ (listPanel.x + 12), (listPanel.y + 96 + targetIndex * 58), 132, 48 };
                    const bool selected = (activeRenderModel == renderModel);
                    DrawCompactSelectionCard(buttonRect, ToCompactLabel(ToModelHeightTargetLabel(renderModel), 8), selected);

                    if (buttonRect.mouseOver() && MouseL.down())
                    {
                        activeRenderModel = renderModel;
                    }

                    ++targetIndex;
                }

                if (DrawMiniHandleButton(Rect{ (listPanel.rightX() - TextureColumnHandleSize - 6), (listPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
                {
                    modelListCollapsed = true;
                }
            }
            else if (DrawMiniHandleButton(Rect{ listPanel.x, (listPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
            {
                modelListCollapsed = false;
            }

            DrawEditorSectionLabel(detailPanel, U"Parameters");
            SimpleGUI::GetFont()(ToModelHeightTargetLabel(activeRenderModel)).draw((detailX + 2), (detailPanel.y + 28), ColorF{ 0.14 });
            DrawCompactTextureParameterRow(Rect{ detailX, sectionTop, detailWidth, compactRowHeight }, 0, U"offset", activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax, ModelHeightDragRoundStep, 3);
            DrawCompactTextureParameterRow(Rect{ detailX, (sectionTop + compactRowHeight + compactRowGap), detailWidth, compactRowHeight }, 1, U"scale", activeScale, ModelScaleMin, ModelScaleMax, ModelHeightDragRoundStep, 3);

            int32 summaryY = (sectionTop + (compactRowHeight + compactRowGap) * 2 + 10);

            if (activeModel.hasAnimations())
            {
                const auto& clips = activeModel.animations();
                for (const UnitModelAnimationRole role : { UnitModelAnimationRole::Idle, UnitModelAnimationRole::Move, UnitModelAnimationRole::Attack })
                {
                    int32& clipIndex = GetModelAnimationClipIndex(modelHeightSettings, activeRenderModel, role);
                    clipIndex = Clamp(clipIndex, -1, static_cast<int32>(clips.size() - 1));

                    const Rect cardRect{ detailX, summaryY, detailWidth, 44 };
                    const int32 buttonGap = 6;
                    const int32 buttonWidth = Max(44, ((detailWidth - 24 - buttonGap * 3) / 4));
                    const int32 buttonY = (cardRect.y + 20);
                    const Rect prevClipButton{ (cardRect.x + 12), buttonY, buttonWidth, 18 };
                    const Rect nextClipButton{ (prevClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };
                    const Rect clearClipButton{ (nextClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };
                    const Rect previewButton{ (clearClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };

                    const String clipSummary = (0 <= clipIndex)
                        ? U"{}/{}"_fmt((clipIndex + 1), clips.size())
                        : U"None";
                    cardRect.rounded(8).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
                    SimpleGUI::GetFont()(ToAnimationRoleLabel(role)).draw((cardRect.x + 12), (cardRect.y + 4), ColorF{ 0.12 });
                    SimpleGUI::GetFont()(ToCompactLabel((0 <= clipIndex) ? clips[clipIndex].name : U"None", 12)).draw((cardRect.x + 76), (cardRect.y + 4), ColorF{ 0.12 });
                    SimpleGUI::GetFont()(clipSummary).draw((cardRect.rightX() - 48), (cardRect.y + 4), ColorF{ 0.12 });

                    if (DrawTextButton(prevClipButton, U"Prev"))
                    {
                        clipIndex = (clipIndex < 0)
                            ? (static_cast<int32>(clips.size()) - 1)
                            : (clipIndex - 1);
                        previewAnimationRole = role;
                    }

                    if (DrawTextButton(nextClipButton, U"Next"))
                    {
                        clipIndex = (clipIndex + 1);
                        if (static_cast<int32>(clips.size()) <= clipIndex)
                        {
                            clipIndex = -1;
                        }
                        previewAnimationRole = role;
                    }

                    if (DrawTextButton(clearClipButton, U"Clear"))
                    {
                        clipIndex = -1;
                        previewAnimationRole = role;
                    }

                    if (DrawTextButton(previewButton, ((previewAnimationRole == role) ? U"Preview*" : U"Preview")))
                    {
                        previewAnimationRole = role;
                    }

                    summaryY += 50;
                }
            }

            const int32 actionButtonWidth = Max(88, ((detailWidth - 8) / 2));
            if (DrawTextButton(Rect{ detailX, summaryY, actionButtonWidth, 30 }, Tr(TextId::ModelHeightResetTarget)))
            {
                activeOffset = 0.0;
                activeScale = 1.0;
                GetModelAnimationClipIndex(modelHeightSettings, activeRenderModel, UnitModelAnimationRole::Idle) = -1;
                GetModelAnimationClipIndex(modelHeightSettings, activeRenderModel, UnitModelAnimationRole::Move) = -1;
                GetModelAnimationClipIndex(modelHeightSettings, activeRenderModel, UnitModelAnimationRole::Attack) = -1;
                previewAnimationRole = UnitModelAnimationRole::Idle;
            }

            if (DrawTextButton(Rect{ detailX, (summaryY + 38), actionButtonWidth, 30 }, Tr(TextId::CommonSave)))
            {
                modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
                    ? TrFormat(TextId::ModelHeightSavedWithPath, ModelHeightSettingsPath)
                    : Tr(TextId::ModelHeightSaveFailed);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (DrawTextButton(Rect{ (detailX + actionButtonWidth + 8), (summaryY + 38), actionButtonWidth, 30 }, Tr(TextId::CommonReload)))
            {
                modelHeightSettings = LoadModelHeightSettings();
                modelHeightMessage = Tr(TextId::ModelHeightReloaded);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            if (DrawTextButton(Rect{ (detailX + actionButtonWidth + 8), summaryY, actionButtonWidth, 30 }, Tr(TextId::CommonResetAll)))
            {
                modelHeightSettings = {};
                modelHeightMessage = Tr(TextId::ModelHeightOffsetsScalesReset);
                modelHeightMessageUntil = (Scene::Time() + 2.0);
            }

            const Rect statusRect{ detailX, Min((detailPanel.bottomY() - 28), (summaryY + 76)), detailWidth, 24 };
            statusRect.rounded(6).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
            const String statusText = (Scene::Time() < modelHeightMessageUntil)
                ? ToCompactLabel(modelHeightMessage, 26)
                : ToCompactLabel(U"worldY {:.3f} / scale {:.3f}"_fmt(GetModelHeightWorldY(activeRenderModel, previewRenderPositions), GetActiveModelScale(modelHeightSettings, activeRenderModel)), 26);
            SimpleGUI::GetFont()(statusText).draw((statusRect.x + 10), statusRect.y, ColorF{ 0.12 });
        }
    }
}
