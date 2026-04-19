# include "MainUiModelHeightInternal.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUiInternal.hpp"
# include "SkyAppUiPanelFrameInternal.hpp"

namespace MainSupport
{
    using SkyAppText::TextId;
    using SkyAppText::Tr;

    void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
        size_t& activePreviewModelIndex,
        bool& textureMode,
        TireTrackTextureSegment& activeTextureSegment,
        UnitModelAnimationRole& previewAnimationRole,
        UnitModel& activeModel,
        const Array<FilePath>& previewModelPaths,
        const Array<String>& previewModelLabels,
        String& modelHeightMessage,
        double& modelHeightMessageUntil,
        const Rect& panelRect,
        const Array<Vec3>& previewRenderPositions)
    {
        using namespace ModelHeightEditorDetail;

        if (previewModelPaths.isEmpty())
        {
            return;
        }

        activePreviewModelIndex = Min(activePreviewModelIndex, (previewModelPaths.size() - 1));
        const FilePath& activePreviewModelPath = previewModelPaths[activePreviewModelIndex];
        const int32 modelListWidth = ((not textureMode) && ModelListCollapsed()) ? TextureColumnCollapsedWidth : 156;
        const int32 modelDetailOffset = ((not textureMode) ? (modelListWidth + 8) : 164);
        const Rect listPanel{ panelRect.x, panelRect.y, modelListWidth, panelRect.h };
        const Rect detailPanel{ (panelRect.x + modelDetailOffset), panelRect.y, (panelRect.w - modelDetailOffset), panelRect.h };
        double& activeOffset = GetModelHeightOffset(modelHeightSettings, activePreviewModelPath);
        double& activeScale = GetModelScale(modelHeightSettings, activePreviewModelPath);
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

        DrawModelHeightEditorContext context{
            .modelHeightSettings = modelHeightSettings,
            .activePreviewModelIndex = activePreviewModelIndex,
            .textureMode = textureMode,
            .activeTextureSegment = activeTextureSegment,
            .previewAnimationRole = previewAnimationRole,
            .activeModel = activeModel,
            .previewModelPaths = previewModelPaths,
            .previewModelLabels = previewModelLabels,
            .modelHeightMessage = modelHeightMessage,
            .modelHeightMessageUntil = modelHeightMessageUntil,
            .panelRect = panelRect,
            .previewRenderPositions = previewRenderPositions,
        };

        ModelHeightEditorBindings bindings{
            .activePreviewModelPath = activePreviewModelPath,
            .activeOffset = activeOffset,
            .activeScale = activeScale,
            .activeTextureYOffset = activeTextureYOffset,
            .activeTextureOpacity = activeTextureOpacity,
            .activeTextureSoftness = activeTextureSoftness,
            .activeTextureWarmth = activeTextureWarmth,
        };

        if (textureMode)
        {
            DrawTextureModeEditor(context, bindings);
            return;
        }

        DrawModelModeEditor(context, bindings, listPanel, detailPanel);
    }
}
