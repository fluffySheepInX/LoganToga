# pragma once
# include <Siv3D.hpp>

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

    struct SceneAssetEditorModel
    {
        String id;
        Vec2 shadowSize{ 1.8, 1.4 };
        Vec2 shadowOffsetXZ{ 0.14, 0.10 };
        double shadowOpacity = 0.7;
        bool useProjectedShadow = false;
    };

    struct KickerLightDrawParams
    {
        bool enabled = false;
        Vec3 direction{ 0.0, 0.0, 1.0 };
        ColorF colorLinear{ 1.0, 1.0, 1.0, 1.0 };
        double intensity = 0.0;
    };

    struct KickerLightShaderConstants
    {
        Float4 directionIntensity{ 0.0f, 0.0f, 1.0f, 0.0f };
        Float4 colorEnable{ 0.0f, 0.0f, 0.0f, 0.0f };
    };

    class SceneAssets
    {
    public:
        explicit SceneAssets(const FilePath& configPath = U"Application/scene_assets.toml");

        [[nodiscard]] size_t editableModelCount() const noexcept;
        [[nodiscard]] Array<String> editableModelNames() const;
        [[nodiscard]] Optional<SceneAssetEditorModel> getEditableModel(size_t index) const;
        bool applyEditableModel(size_t index, const SceneAssetEditorModel& model);
        bool saveSettingsToConfig() const;

        void drawStaticScene(const Vec3& sunDirection, const KickerLightDrawParams& kickerLight) const;
        [[nodiscard]] const Mesh& groundPlane() const noexcept;
        [[nodiscard]] const Texture& groundTexture() const noexcept;
        [[nodiscard]] const SceneAssetSettings& settings() const noexcept;

    private:
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

        static SceneAssetSettings LoadSettings(const FilePath& configPath);
        static Array<SceneModelAsset> DefaultModels();

        FilePath m_configPath;
        SceneAssetSettings m_settings;
        Mesh m_groundPlane;
        Texture m_groundTexture;
        Array<LoadedModelAsset> m_models;
        const Array<ConstantBufferBinding> m_defaultForwardVSBindings = {
            { U"VSPerView", 1 },
            { U"VSPerObject", 2 },
            { U"VSPerMaterial", 3 },
        };
        const Array<ConstantBufferBinding> m_defaultForwardPSBindings = {
            { U"PSPerFrame", 0 },
            { U"PSPerView", 1 },
            { U"PSKicker", 2 },
            { U"PSPerMaterial", 3 },
        };
        VertexShader m_defaultForwardVS;
        PixelShader m_defaultForwardPS;
        mutable ConstantBuffer<KickerLightShaderConstants> m_kickerBuffer;
        PixelShader m_projectedShadowPS;
        mutable ConstantBuffer<Float4> m_projectedShadowParams{ Float4{ 0.7f, 0.0f, 0.0f, 0.0f } };
    };
}
