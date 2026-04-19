# pragma once
# include "MainUi.hpp"

namespace MainSupport::ModelHeightEditorDetail
{
    inline constexpr double ModelHeightDragRoundStep = 0.001;
    inline constexpr int32 TextureColumnWidth = 140;
    inline constexpr int32 TextureColumnCollapsedWidth = 22;
    inline constexpr int32 TextureColumnGap = 12;
    inline constexpr int32 TextureColumnHandleSize = 18;

    struct DrawModelHeightEditorContext
    {
        ModelHeightSettings& modelHeightSettings;
        size_t& activePreviewModelIndex;
        bool& textureMode;
        TireTrackTextureSegment& activeTextureSegment;
        UnitModelAnimationRole& previewAnimationRole;
        UnitModel& activeModel;
        const Array<FilePath>& previewModelPaths;
        const Array<String>& previewModelLabels;
        String& modelHeightMessage;
        double& modelHeightMessageUntil;
        const Rect& panelRect;
        const Array<Vec3>& previewRenderPositions;
    };

    struct ModelHeightEditorBindings
    {
        const FilePath& activePreviewModelPath;
        double& activeOffset;
        double& activeScale;
        double& activeTextureYOffset;
        double& activeTextureOpacity;
        double& activeTextureSoftness;
        double& activeTextureWarmth;
    };

    [[nodiscard]] bool& ModelListCollapsed();
    [[nodiscard]] bool& TextureInfoCollapsed();
    [[nodiscard]] bool& TextureSegmentCollapsed();
    [[nodiscard]] StringView ToModelHeightTargetLabel(size_t previewModelIndex, const Array<String>& previewModelLabels);
    [[nodiscard]] StringView ToAnimationRoleLabel(UnitModelAnimationRole role);
    [[nodiscard]] double GetActiveModelScale(const ModelHeightSettings& modelHeightSettings, FilePathView modelPath);
    [[nodiscard]] double GetModelHeightWorldY(size_t previewModelIndex, const Array<Vec3>& previewRenderPositions);
    [[nodiscard]] StringView ToTextureTargetLabel(TireTrackTextureSegment segment);
    [[nodiscard]] double RoundModelHeightEditorValue(double value, double roundStep);
    [[nodiscard]] String ToCompactTextureName(StringView label);
    [[nodiscard]] String ToCompactLabel(StringView label, size_t maxVisibleCharacters);
    void DrawEditorSectionLabel(const Rect& panelRect, StringView label);
    bool DrawMiniHandleButton(const Rect& rect, StringView label);
    void DrawCompactSelectionCard(const Rect& rect, StringView title, bool selected);
    void CopyTextureSegmentSettings(ModelHeightSettings& destination, const ModelHeightSettings& source, TireTrackTextureSegment segment);
    void DrawCompactTextureParameterRow(const Rect& rect,
        int32 controlId,
        StringView label,
        double& value,
        double minValue,
        double maxValue,
        double roundStep,
        int32 decimals);
    void DrawTextureModeEditor(DrawModelHeightEditorContext& context, ModelHeightEditorBindings bindings);
    void DrawModelModeEditor(DrawModelHeightEditorContext& context, ModelHeightEditorBindings bindings, const Rect& listPanel, const Rect& detailPanel);
}
