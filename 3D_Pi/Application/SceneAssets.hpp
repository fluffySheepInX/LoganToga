# pragma once
# include <Siv3D.hpp>
# include "../UI/EditorIconLayout.hpp"

namespace app
{
    struct SceneModelAsset
    {
        String id;
        FilePath path;
        Vec3 position{ 0, 0, 0 };
        Optional<double> rotateYDegrees;
        bool registerDiffuseTextures = false;
        bool drawDoubleSided = false;
        Optional<Vec2> shadowSizeXZ;
        Optional<Vec2> shadowOffsetXZ;
        Optional<double> shadowOpacity;
        Optional<bool> projectedShadow;
    };

    struct SceneAssetSettings
    {
        FilePath groundTexturePath = U"example/texture/ground.jpg";
        double groundSize = 2000.0;
        Size groundResolution{ 400, 400 };
        Array<SceneModelAsset> models;
    };

    class SceneAssets
    {
    public:
        explicit SceneAssets(const FilePath& configPath = U"Application/scene_assets.toml");

        void updateEditor();
        void drawEditorUI();
        [[nodiscard]] bool wantsMouseCapture() const;

        void drawStaticScene(const Vec3& sunDirection) const;
        [[nodiscard]] const Mesh& groundPlane() const noexcept;
        [[nodiscard]] const Texture& groundTexture() const noexcept;
        [[nodiscard]] const SceneAssetSettings& settings() const noexcept;

    private:
     static constexpr double EditorPanelWidth = 420.0;
        static constexpr double CollapsedIconSize = ui::editor_icon::CollapsedIconSize;

        struct LoadedModelAsset
        {
            SceneModelAsset settings;
            Model model;
            Texture shadowTexture;
            Vec2 shadowSize{ 1.8, 1.4 };
            Vec2 shadowOffsetXZ{ 0.14, 0.10 };
            double shadowOpacity = 0.7;
            bool useProjectedShadow = false;
        };

        [[nodiscard]] RectF getEditorPanelRect() const;
      [[nodiscard]] RectF getCollapsedIconRect() const;
        void syncCollapsedIconRegistry() const;
        void updateCollapsedIconDrag(const RectF& dragRect);
        void saveSettingsToConfig() const;

        static SceneAssetSettings LoadSettings(const FilePath& configPath);
        static Array<SceneModelAsset> DefaultModels();

        FilePath m_configPath;
        SceneAssetSettings m_settings;
        Mesh m_groundPlane;
        Texture m_groundTexture;
        Array<LoadedModelAsset> m_models;
        PixelShader m_projectedShadowPS;
        mutable ConstantBuffer<Float4> m_projectedShadowParams{ Float4{ 0.7f, 0.0f, 0.0f, 0.0f } };

        Font m_editorFont{ 18 };
        Font m_editorSmallFont{ 12 };
        bool m_editorEnabled = false;
     bool m_editorCollapsed = true;
        bool m_editorDragging = false;
        Vec2 m_editorPanelPos{ ui::editor_icon::GetDockedStackPosition(2) };
        Vec2 m_editorDragOffset{ 0, 0 };
        Vec2 m_togglePressCursor{ 0, 0 };
        Texture m_toggleIcon{ U"texture/shadowEditor.png" };
        size_t m_selectedModelIndex = 0;
        String m_editorStatus = U"Shadow Editor: Ready";
      bool m_ignoreCollapsedClickUntilRelease = false;
    };
}
