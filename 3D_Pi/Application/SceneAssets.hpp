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

        void drawStaticScene() const;
        [[nodiscard]] const Mesh& groundPlane() const noexcept;
        [[nodiscard]] const Texture& groundTexture() const noexcept;
        [[nodiscard]] const SceneAssetSettings& settings() const noexcept;

    private:
        struct LoadedModelAsset
        {
            SceneModelAsset settings;
            Model model;
        };

        static SceneAssetSettings LoadSettings(const FilePath& configPath);
        static Array<SceneModelAsset> DefaultModels();

        SceneAssetSettings m_settings;
        Mesh m_groundPlane;
        Texture m_groundTexture;
        Array<LoadedModelAsset> m_models;
    };
}
