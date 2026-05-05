# include "../stdafx.h"
# include "SceneAssets.hpp"

namespace app
{
    SceneAssets::SceneAssets(const FilePath& configPath)
        : m_settings{ LoadSettings(configPath) }
        , m_groundPlane{ MeshData::OneSidedPlane(m_settings.groundSize, m_settings.groundResolution) }
        , m_groundTexture{ m_settings.groundTexturePath, TextureDesc::MippedSRGB }
    {
        for (const auto& modelSettings : m_settings.models)
        {
            LoadedModelAsset loaded{ .settings = modelSettings, .model = Model{ modelSettings.path } };
            if (modelSettings.registerDiffuseTextures)
            {
                Model::RegisterDiffuseTextures(loaded.model, TextureDesc::MippedSRGB);
            }
            m_models << std::move(loaded);
        }
    }

    void SceneAssets::drawStaticScene() const
    {
        Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

        for (const auto& asset : m_models)
        {
            if (asset.settings.drawDoubleSided)
            {
                const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
                if (asset.settings.rotateYDegrees)
                {
                    asset.model.draw(asset.settings.position, Quaternion::RotateY(*asset.settings.rotateYDegrees * Math::Pi / 180.0));
                }
                else
                {
                    asset.model.draw(asset.settings.position);
                }
                continue;
            }

            if (asset.settings.rotateYDegrees)
            {
                asset.model.draw(asset.settings.position, Quaternion::RotateY(*asset.settings.rotateYDegrees * Math::Pi / 180.0));
            }
            else
            {
                asset.model.draw(asset.settings.position);
            }
        }
    }

    const Mesh& SceneAssets::groundPlane() const noexcept
    {
        return m_groundPlane;
    }

    const Texture& SceneAssets::groundTexture() const noexcept
    {
        return m_groundTexture;
    }

    const SceneAssetSettings& SceneAssets::settings() const noexcept
    {
        return m_settings;
    }

    SceneAssetSettings SceneAssets::LoadSettings(const FilePath& configPath)
    {
        SceneAssetSettings settings;
        settings.models = DefaultModels();

        if (not FileSystem::Exists(configPath))
        {
            return settings;
        }

        const TOMLReader toml{ configPath };
        if (not toml)
        {
            return settings;
        }

        const auto ground = toml[U"ground"];
        settings.groundTexturePath = ground[U"texturePath"].getOr<String>(settings.groundTexturePath);
        settings.groundSize = ground[U"size"].getOr<double>(settings.groundSize);
        settings.groundResolution.x = ground[U"resolutionX"].getOr<int32>(settings.groundResolution.x);
        settings.groundResolution.y = ground[U"resolutionY"].getOr<int32>(settings.groundResolution.y);

        Array<SceneModelAsset> models;
        for (const auto& v : toml[U"models"].tableArrayView())
        {
            SceneModelAsset model;
            model.id = v[U"id"].getOr<String>(U"");
            model.path = v[U"path"].getOr<String>(U"");
            model.position = Vec3{
                v[U"positionX"].getOr<double>(0.0),
                v[U"positionY"].getOr<double>(0.0),
                v[U"positionZ"].getOr<double>(0.0)
            };
            if (const auto rotation = v[U"rotateYDegrees"].getOpt<double>())
            {
                model.rotateYDegrees = *rotation;
            }
            model.registerDiffuseTextures = v[U"registerDiffuseTextures"].getOr<bool>(false);
            model.drawDoubleSided = v[U"drawDoubleSided"].getOr<bool>(false);

            if (not model.path.isEmpty())
            {
                models << std::move(model);
            }
        }

        if (not models.isEmpty())
        {
            settings.models = std::move(models);
        }

        return settings;
    }

    Array<SceneModelAsset> SceneAssets::DefaultModels()
    {
        return {
            { U"blacksmith", U"example/obj/blacksmith.obj", Vec3{ 8, 0, 4 }, none, false, false },
            { U"mill", U"example/obj/mill.obj", Vec3{ -8, 0, 4 }, none, false, false },
            { U"tree", U"example/obj/tree.obj", Vec3{ 16, 0, 4 }, none, true, true },
            { U"pine", U"example/obj/pine.obj", Vec3{ 16, 0, 0 }, none, true, true },
            { U"siv3d-kun", U"example/obj/siv3d-kun.obj", Vec3{ 2, 0, -2 }, 180.0, true, false },
        };
    }
}
