# include "MainUiModelHeightInternal.hpp"
# include "MainSettings.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUiInternal.hpp"

namespace MainSupport::ModelHeightEditorDetail
{
    using SkyAppText::Tr;
    using SkyAppText::TrFormat;

    void DrawTextureModeEditor(DrawModelHeightEditorContext& context, ModelHeightEditorBindings bindings)
    {
        const int32 contentTop = (context.panelRect.y + 70);
        const int32 contentHeight = (context.panelRect.h - 82);
        const int32 textureInfoWidth = (TextureInfoCollapsed() ? TextureColumnCollapsedWidth : TextureColumnWidth);
        const int32 textureSegmentWidth = (TextureSegmentCollapsed() ? TextureColumnCollapsedWidth : TextureColumnWidth);
        const Rect textureInfoPanel{ (context.panelRect.x + 12), contentTop, textureInfoWidth, contentHeight };
        const Rect textureSegmentPanel{ (textureInfoPanel.rightX() + TextureColumnGap), contentTop, textureSegmentWidth, contentHeight };
        const Rect textureDetailPanel{ (textureSegmentPanel.rightX() + TextureColumnGap), contentTop, Max(180, (context.panelRect.rightX() - textureSegmentPanel.rightX() - 20)), contentHeight };
        const int32 detailX = (textureDetailPanel.x + 8);
        const int32 detailWidth = Max(180, (textureDetailPanel.w - 16));
        const int32 detailButtonWidth = Max(88, ((detailWidth - 8) / 2));
        const int32 detailRowHeight = 48;
        const int32 detailRowGap = 12;
        const int32 detailRowsTop = (textureDetailPanel.y + 52);
        const int32 actionRowsTop = (textureDetailPanel.bottomY() - 78);

        if (not TextureInfoCollapsed())
        {
            Rect{ (textureInfoPanel.rightX() + 5), (context.panelRect.y + 8), 1, (context.panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
            DrawEditorSectionLabel(textureInfoPanel, U"Texture");
            DrawCompactSelectionCard(Rect{ (textureInfoPanel.x + 4), (textureInfoPanel.y + 28), (textureInfoPanel.w - 8), 76 }, ToCompactTextureName(U"tireTrackTextureSet"), true);

            if (DrawMiniHandleButton(Rect{ (textureInfoPanel.rightX() - TextureColumnHandleSize - 6), (textureInfoPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
            {
                TextureInfoCollapsed() = true;
            }
        }
        else if (DrawMiniHandleButton(Rect{ textureInfoPanel.x, (textureInfoPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
        {
            TextureInfoCollapsed() = false;
        }

        if (not TextureSegmentCollapsed())
        {
            Rect{ (textureSegmentPanel.rightX() + 5), (context.panelRect.y + 8), 1, (context.panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
            DrawEditorSectionLabel(textureSegmentPanel, U"Segment");

            int32 targetIndex = 0;
            for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
            {
                const Rect buttonRect{ (textureSegmentPanel.x + 4), (textureSegmentPanel.y + 28 + targetIndex * 64), (textureSegmentPanel.w - 8), 52 };
                const bool selected = (context.activeTextureSegment == segment);
                DrawCompactSelectionCard(buttonRect, ToTextureTargetLabel(segment), selected);

                if (buttonRect.mouseOver() && MouseL.down())
                {
                    context.activeTextureSegment = segment;
                }

                ++targetIndex;
            }

            if (DrawMiniHandleButton(Rect{ (textureSegmentPanel.rightX() - TextureColumnHandleSize - 6), (textureSegmentPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
            {
                TextureSegmentCollapsed() = true;
            }
        }
        else if (DrawMiniHandleButton(Rect{ textureSegmentPanel.x, (textureSegmentPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
        {
            TextureSegmentCollapsed() = false;
        }

        DrawEditorSectionLabel(textureDetailPanel, U"Parameters");
        SimpleGUI::GetFont()(ToTextureTargetLabel(context.activeTextureSegment)).draw((detailX + 2), (textureDetailPanel.y + 28), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
        DrawCompactTextureParameterRow(Rect{ detailX, detailRowsTop, detailWidth, detailRowHeight }, 10, U"offset", bindings.activeTextureYOffset, TireTrackYOffsetMin, TireTrackYOffsetMax, ModelHeightDragRoundStep, 3);
        DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap)), detailWidth, detailRowHeight }, 11, U"opacity", bindings.activeTextureOpacity, TireTrackOpacityMin, TireTrackOpacityMax, 0.01, 3);
        DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap) * 2), detailWidth, detailRowHeight }, 12, U"softness", bindings.activeTextureSoftness, TireTrackSoftnessMin, TireTrackSoftnessMax, 0.01, 3);
        DrawCompactTextureParameterRow(Rect{ detailX, (detailRowsTop + (detailRowHeight + detailRowGap) * 3), detailWidth, detailRowHeight }, 13, U"warmth", bindings.activeTextureWarmth, TireTrackWarmthMin, TireTrackWarmthMax, 0.01, 3);

        if (DrawTextButton(Rect{ detailX, actionRowsTop, detailButtonWidth, 32 }, U"Reset Segment"))
        {
            ModelHeightSettings defaultSettings;
            CopyTextureSegmentSettings(context.modelHeightSettings, defaultSettings, context.activeTextureSegment);
            context.modelHeightMessage = U"Segment reset";
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (DrawTextButton(Rect{ (detailX + detailButtonWidth + 8), actionRowsTop, detailButtonWidth, 32 }, U"Reload Segment"))
        {
            const ModelHeightSettings loadedSettings = LoadModelHeightSettings();
            CopyTextureSegmentSettings(context.modelHeightSettings, loadedSettings, context.activeTextureSegment);
            context.modelHeightMessage = Tr(U"ModelHeightReloaded");
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (DrawTextButton(Rect{ detailX, (actionRowsTop + 40), detailButtonWidth, 32 }, U"Reset All"))
        {
            context.modelHeightSettings = {};
            context.modelHeightMessage = Tr(U"ModelHeightOffsetsScalesReset");
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (DrawTextButton(Rect{ (detailX + detailButtonWidth + 8), (actionRowsTop + 40), detailButtonWidth, 32 }, U"Save All"))
        {
            context.modelHeightMessage = SaveModelHeightSettings(context.modelHeightSettings)
                ? TrFormat(U"ModelHeightSavedWithPath", ModelHeightSettingsPath)
                : Tr(U"ModelHeightSaveFailed");
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (Scene::Time() < context.modelHeightMessageUntil)
        {
            const Rect statusRect{ detailX, (actionRowsTop - 40), detailWidth, 28 };
            statusRect.rounded(6).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
            SimpleGUI::GetFont()(context.modelHeightMessage).draw((statusRect.x + 10), (statusRect.y + 2), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
        }
    }
}
