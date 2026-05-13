# include "../stdafx.h"
# include "SceneAssets.hpp"
# include "../Addons/Pi3D/Shader/PiShaderLoader.hpp"

namespace app
{
    namespace
    {
        [[nodiscard]] bool IsProjectedShadowTarget(const SceneModelAsset& asset)
        {
            if (asset.projectedShadow)
            {
                return *asset.projectedShadow;
            }

            return (asset.id == U"mill")
                || (asset.id == U"blacksmith")
                || (asset.id == U"siv3d-kun");
        }

        [[nodiscard]] double ClampShadowOpacity(const double value)
        {
            return Clamp(value, 0.0, 0.9);
        }

        [[nodiscard]] const BlendState& GroundShadowBlendState()
        {
            static const BlendState blendState = []()
                {
                    BlendState state = BlendState::Default2D;
                    state.src = Blend::SrcAlpha;
                    state.dst = Blend::InvSrcAlpha;
                    state.op = BlendOp::Add;
                    state.srcAlpha = Blend::One;
                    state.dstAlpha = Blend::InvSrcAlpha;
                    state.opAlpha = BlendOp::Add;
                    return state;
                }();
            return blendState;
        }

        Texture MakeSoftShadowTexture(const Size size, const double edgeSoftness, const double centerBoost = 1.0)
        {
            Image image{ size, Color{ 0, 0, 0, 0 } };
            const Vec2 center = (Vec2{ size } * 0.5);
            const Vec2 half = (Vec2{ size } * 0.5);

            for (int32 y = 0; y < size.y; ++y)
            {
                for (int32 x = 0; x < size.x; ++x)
                {
                    const double nx = (x - center.x) / Max(1.0, half.x);
                    const double ny = (y - center.y) / Max(1.0, half.y);
                    const double ellipseDist = Math::Sqrt((nx * nx) + (ny * ny));
                    const double inner = Clamp(1.0 - ellipseDist, 0.0, 1.0);
                    const double alpha = Math::Pow(inner, edgeSoftness) * centerBoost;
                    const uint8 a = static_cast<uint8>(Clamp(alpha, 0.0, 1.0) * 255.0);
                    image[y][x] = Color{ 0, 0, 0, a };
                }
            }

            return Texture{ image, TextureDesc::Unmipped };
        }

        Texture MakeTreeShadowTexture()
        {
            Image image{ 192, 192, Color{ 0, 0, 0, 0 } };
            const Vec2 center{ 96, 96 };

            for (int32 y = 0; y < 192; ++y)
            {
                for (int32 x = 0; x < 192; ++x)
                {
                    const Vec2 p{ x, y };
                    const double dx = (p.x - center.x) / 92.0;
                    const double dy = (p.y - center.y) / 70.0;
                    const double canopy = Clamp(1.0 - Math::Sqrt((dx * dx) + (dy * dy)), 0.0, 1.0);

                    const double trunkDx = (p.x - center.x) / 20.0;
                    const double trunkDy = (p.y - (center.y + 18.0)) / 54.0;
                    const double trunk = Clamp(1.0 - Math::Sqrt((trunkDx * trunkDx) + (trunkDy * trunkDy)), 0.0, 1.0);

                    const double alpha = Clamp(Math::Pow(canopy, 1.85) * 0.78 + Math::Pow(trunk, 1.35) * 0.42, 0.0, 1.0);
                    image[y][x] = Color{ 0, 0, 0, static_cast<uint8>(alpha * 255.0) };
                }
            }

            return Texture{ image, TextureDesc::Unmipped };
        }

        Texture MakeBuildingShadowTexture()
        {
            Image image{ 256, 256, Color{ 0, 0, 0, 0 } };
            const Rect buildingRect{ 42, 56, 170, 128 };
            const Rect sideWing{ 28, 92, 54, 82 };

            for (int32 y = 0; y < 256; ++y)
            {
                for (int32 x = 0; x < 256; ++x)
                {
                    const bool inMain = buildingRect.intersects(Point{ x, y });
                    const bool inWing = sideWing.intersects(Point{ x, y });

                    double alpha = 0.0;
                    if (inMain || inWing)
                    {
                        const double fx = Min((x - 20.0) / 236.0, (236.0 - x) / 236.0);
                        const double fy = Min((y - 36.0) / 220.0, (236.0 - y) / 220.0);
                        const double edge = Clamp(Min(fx, fy) * 2.6, 0.0, 1.0);
                        alpha = 0.92 * edge;
                    }

                    image[y][x] = Color{ 0, 0, 0, static_cast<uint8>(Clamp(alpha, 0.0, 1.0) * 255.0) };
                }
            }

            return Texture{ image, TextureDesc::Unmipped };
        }

        Vec2 GetDefaultShadowSize(const SceneModelAsset& asset)
        {
            if (asset.id == U"tree")
            {
                return Vec2{ 2.7, 2.1 };
            }

            if (asset.id == U"pine")
            {
                return Vec2{ 2.4, 1.9 };
            }

            if (asset.id == U"mill")
            {
                return Vec2{ 3.2, 2.4 };
            }

            if (asset.id == U"blacksmith")
            {
                return Vec2{ 2.9, 2.2 };
            }

            if (asset.id == U"siv3d-kun")
            {
                return Vec2{ 1.05, 0.92 };
            }

            return Vec2{ 1.8, 1.4 };
        }

        Vec2 GetDefaultShadowOffsetXZ(const SceneModelAsset& asset)
        {
            if (asset.id == U"mill")
            {
                return Vec2{ 0.34, 0.24 };
            }

            if (asset.id == U"blacksmith")
            {
                return Vec2{ 0.22, 0.18 };
            }

            if (asset.id == U"tree" || asset.id == U"pine")
            {
                return Vec2{ 0.18, 0.16 };
            }

            return Vec2{ 0.14, 0.10 };
        }

        double GetDefaultShadowOpacity(const SceneModelAsset& asset)
        {
            if (asset.id == U"tree" || asset.id == U"pine")
            {
                return 0.58;
            }

            if (asset.id == U"siv3d-kun")
            {
                return 0.66;
            }

            return 0.72;
        }

        Texture MakeShadowTextureFor(const SceneModelAsset& asset)
        {
            if (asset.id == U"tree" || asset.id == U"pine")
            {
                return MakeTreeShadowTexture();
            }

            if (asset.id == U"mill" || asset.id == U"blacksmith")
            {
                return MakeBuildingShadowTexture();
            }

            return MakeSoftShadowTexture(Size{ 160, 120 }, 2.2, 0.92);
        }

        [[nodiscard]] const Texture& DefaultUnitShadowTexture()
        {
            static const Texture texture = MakeSoftShadowTexture(Size{ 160, 120 }, 2.0, 0.86);
            return texture;
        }

        [[nodiscard]] Mat4x4 MakeGroundProjectionMatrix(const Vec3& sunDirection)
        {
            const double clampedY = (sunDirection.y >= 0.0)
                ? Max(sunDirection.y, 0.18)
                : Min(sunDirection.y, -0.18);

            const double sx = (-sunDirection.x / clampedY);
            const double sz = (-sunDirection.z / clampedY);

            return Mat4x4{
                1.0f, 0.0f, 0.0f, 0.0f,
                static_cast<float>(sx), 0.0f, static_cast<float>(sz), 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
        }


        void DrawTexturedGroundShadow(const Vec3& worldOrigin, const Vec2& sizeXZ, const Vec2& offsetXZ, const Texture& texture, const double opacity)
        {
            const ScopedRenderStates3D states{ GroundShadowBlendState(), RasterizerState::SolidCullNone };
            const Transformer3D transform{
                Mat4x4::Identity()
                    .scaled(Float3{ static_cast<float>(sizeXZ.x), 1.0f, static_cast<float>(sizeXZ.y) })
                    .translated(worldOrigin + Vec3{ offsetXZ.x, 0.014, offsetXZ.y })
            };

            Plane{ Vec3{ 0, 0, 0 }, 1.0 }.draw(texture, ColorF{ 1.0, 1.0, 1.0, Clamp(opacity, 0.0, 1.0) }.removeSRGBCurve());
        }
    }

    SceneAssets::SceneAssets(const FilePath& configPath)
        : m_configPath{ configPath }
        , m_settings{ LoadSettings(configPath) }
        , m_groundPlane{ MeshData::OneSidedPlane(m_settings.groundSize, m_settings.groundResolution) }
        , m_groundTexture{ m_settings.groundTexturePath, TextureDesc::MippedSRGB }
        , m_defaultForwardVS{ HLSL{ Pi3D::PiShaderLoader::HLSL(U"default3d_forward"), U"VS" } | GLSL{ Pi3D::PiShaderLoader::GLSLVertex(U"default3d_forward"), m_defaultForwardVSBindings } }
        , m_defaultForwardPS{ HLSL{ Pi3D::PiShaderLoader::HLSL(U"default3d_forward"), U"PS" } | GLSL{ Pi3D::PiShaderLoader::GLSLFragment(U"default3d_forward"), m_defaultForwardPSBindings } }
        , m_projectedShadowPS{ HLSL{ Pi3D::PiShaderLoader::HLSL(U"projected_shadow"), U"PS" } }
    {
        for (const auto& modelSettings : m_settings.models)
        {
            LoadedModelAsset loaded{ .settings = modelSettings, .model = Model{ modelSettings.path } };
            loaded.shadowTexture = MakeShadowTextureFor(modelSettings);
            loaded.shadowSize = GetDefaultShadowSize(modelSettings);
            loaded.shadowOffsetXZ = GetDefaultShadowOffsetXZ(modelSettings);
            loaded.shadowOpacity = ClampShadowOpacity(GetDefaultShadowOpacity(modelSettings));
            loaded.useProjectedShadow = IsProjectedShadowTarget(modelSettings);

            if (modelSettings.shadowSizeXZ)
            {
                loaded.shadowSize = *modelSettings.shadowSizeXZ;
            }

            if (modelSettings.shadowOffsetXZ)
            {
                loaded.shadowOffsetXZ = *modelSettings.shadowOffsetXZ;
            }

            if (modelSettings.shadowOpacity)
            {
                loaded.shadowOpacity = ClampShadowOpacity(*modelSettings.shadowOpacity);
            }

            if (modelSettings.projectedShadow)
            {
                loaded.useProjectedShadow = *modelSettings.projectedShadow;
            }

            if (modelSettings.registerDiffuseTextures)
            {
                Model::RegisterDiffuseTextures(loaded.model, TextureDesc::MippedSRGB);
            }
            m_models << std::move(loaded);
        }
    }

    void SceneAssets::drawStaticScene(const Vec3& sunDirection, const KickerLightDrawParams& kickerLight) const
    {
        m_kickerBuffer->directionIntensity = Float4{
            static_cast<float>(kickerLight.direction.x),
            static_cast<float>(kickerLight.direction.y),
            static_cast<float>(kickerLight.direction.z),
            static_cast<float>(Clamp(kickerLight.intensity, 0.0, 2.0)) };
        m_kickerBuffer->colorEnable = Float4{
            static_cast<float>(kickerLight.colorLinear.r),
            static_cast<float>(kickerLight.colorLinear.g),
            static_cast<float>(kickerLight.colorLinear.b),
            kickerLight.enabled ? 1.0f : 0.0f };
        Graphics3D::SetPSConstantBuffer(2, m_kickerBuffer);
        const ScopedCustomShader3D defaultForwardShader{ m_defaultForwardVS, m_defaultForwardPS };

        DrawTexturedGroundShadow(Vec3{ 0, 0, 0 }, Vec2{ 1.9, 1.3 }, Vec2{ 0.12, 0.10 }, DefaultUnitShadowTexture(), 0.66);
        Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

        const Vec3 normalizedSunDirection = sunDirection.normalized();

        for (const auto& asset : m_models)
        {
            if (asset.useProjectedShadow)
            {
                const double baseOpacity = Clamp(asset.shadowOpacity, 0.0, 0.9);
                const double densityScale = Clamp(0.75 + (normalizedSunDirection.y * 0.5), 0.45, 1.0);
                const double shadowOpacity = Clamp(baseOpacity * densityScale, 0.0, 0.9);

                const Mat4x4 world = asset.settings.rotateYDegrees
                    ? Mat4x4::Identity()
                        .rotatedY(*asset.settings.rotateYDegrees * Math::Pi / 180.0)
                        .translated(asset.settings.position)
                    : Mat4x4::Identity().translated(asset.settings.position);

                const Mat4x4 groundBias = Mat4x4::Identity().translated(0.0, 0.016, 0.0);
                const Mat4x4 projection = MakeGroundProjectionMatrix(normalizedSunDirection);

                const ScopedRenderStates3D states{ GroundShadowBlendState(), RasterizerState::SolidCullNone };
                m_projectedShadowParams = Float4{ static_cast<float>(shadowOpacity), 0.0f, 0.0f, 0.0f };
                Graphics3D::SetPSConstantBuffer(2, m_projectedShadowParams);
                const ScopedCustomShader3D shader{ m_projectedShadowPS };
                const Transformer3D transform{ world * projection * groundBias };
                asset.model.draw();

                Graphics3D::SetPSConstantBuffer(2, m_kickerBuffer);
            }
            else
            {
                DrawTexturedGroundShadow(asset.settings.position, asset.shadowSize, asset.shadowOffsetXZ, asset.shadowTexture, asset.shadowOpacity);
            }

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

    size_t SceneAssets::editableModelCount() const noexcept
    {
        return m_models.size();
    }

    Array<String> SceneAssets::editableModelNames() const
    {
        Array<String> modelNames;
        modelNames.reserve(m_models.size());

        for (const auto& model : m_models)
        {
            modelNames << model.settings.id;
        }

        return modelNames;
    }

    Optional<SceneAssetEditorModel> SceneAssets::getEditableModel(const size_t index) const
    {
        if (index >= m_models.size())
        {
            return none;
        }

        const auto& model = m_models[index];
        return SceneAssetEditorModel{
            .id = model.settings.id,
            .shadowSize = model.shadowSize,
            .shadowOffsetXZ = model.shadowOffsetXZ,
            .shadowOpacity = model.shadowOpacity,
            .useProjectedShadow = model.useProjectedShadow,
        };
    }

    bool SceneAssets::applyEditableModel(const size_t index, const SceneAssetEditorModel& model)
    {
        if ((index >= m_models.size()) || (index >= m_settings.models.size()))
        {
            return false;
        }

        auto& loaded = m_models[index];
        auto& settings = m_settings.models[index];

        loaded.shadowSize = model.shadowSize;
        loaded.shadowOffsetXZ = model.shadowOffsetXZ;
        loaded.shadowOpacity = ClampShadowOpacity(model.shadowOpacity);
        loaded.useProjectedShadow = model.useProjectedShadow;

        loaded.settings.shadowSizeXZ = loaded.shadowSize;
        loaded.settings.shadowOffsetXZ = loaded.shadowOffsetXZ;
        loaded.settings.shadowOpacity = loaded.shadowOpacity;
        loaded.settings.projectedShadow = loaded.useProjectedShadow;

        settings.shadowSizeXZ = loaded.shadowSize;
        settings.shadowOffsetXZ = loaded.shadowOffsetXZ;
        settings.shadowOpacity = loaded.shadowOpacity;
        settings.projectedShadow = loaded.useProjectedShadow;
        return true;
    }

    bool SceneAssets::saveSettingsToConfig() const
    {
        const FilePath dir = FileSystem::ParentPath(m_configPath);
        if (not dir.isEmpty())
        {
            FileSystem::CreateDirectories(dir);
        }

        TextWriter writer{ m_configPath };
        if (not writer)
        {
            return false;
        }

        writer.writeln(U"[ground]");
        writer.writeln(U"texturePath = \"{}\""_fmt(m_settings.groundTexturePath));
        writer.writeln(U"size = {:.3f}"_fmt(m_settings.groundSize));
        writer.writeln(U"resolutionX = {}"_fmt(m_settings.groundResolution.x));
        writer.writeln(U"resolutionY = {}"_fmt(m_settings.groundResolution.y));
        writer.writeln(U"");

        for (const auto& model : m_settings.models)
        {
            writer.writeln(U"[[models]]");
            writer.writeln(U"id = \"{}\""_fmt(model.id));
            writer.writeln(U"path = \"{}\""_fmt(model.path));
            writer.writeln(U"positionX = {:.3f}"_fmt(model.position.x));
            writer.writeln(U"positionY = {:.3f}"_fmt(model.position.y));
            writer.writeln(U"positionZ = {:.3f}"_fmt(model.position.z));
            if (model.rotateYDegrees)
            {
                writer.writeln(U"rotateYDegrees = {:.3f}"_fmt(*model.rotateYDegrees));
            }
            writer.writeln(U"registerDiffuseTextures = {}"_fmt(model.registerDiffuseTextures ? U"true" : U"false"));
            writer.writeln(U"drawDoubleSided = {}"_fmt(model.drawDoubleSided ? U"true" : U"false"));
            if (model.shadowSizeXZ)
            {
                writer.writeln(U"shadowSizeX = {:.3f}"_fmt(model.shadowSizeXZ->x));
                writer.writeln(U"shadowSizeZ = {:.3f}"_fmt(model.shadowSizeXZ->y));
            }
            if (model.shadowOffsetXZ)
            {
                writer.writeln(U"shadowOffsetX = {:.3f}"_fmt(model.shadowOffsetXZ->x));
                writer.writeln(U"shadowOffsetZ = {:.3f}"_fmt(model.shadowOffsetXZ->y));
            }
            if (model.shadowOpacity)
            {
                writer.writeln(U"shadowOpacity = {:.3f}"_fmt(*model.shadowOpacity));
            }
            if (model.projectedShadow)
            {
                writer.writeln(U"projectedShadow = {}"_fmt(*model.projectedShadow ? U"true" : U"false"));
            }
            writer.writeln(U"");
        }

        return true;
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
            if (const auto shadowSizeX = v[U"shadowSizeX"].getOpt<double>())
            {
                model.shadowSizeXZ = Vec2{ *shadowSizeX, v[U"shadowSizeZ"].getOr<double>(1.4) };
            }
            if (const auto shadowOffsetX = v[U"shadowOffsetX"].getOpt<double>())
            {
                model.shadowOffsetXZ = Vec2{ *shadowOffsetX, v[U"shadowOffsetZ"].getOr<double>(0.10) };
            }
            if (const auto shadowOpacity = v[U"shadowOpacity"].getOpt<double>())
            {
                model.shadowOpacity = ClampShadowOpacity(*shadowOpacity);
            }
            if (const auto projectedShadow = v[U"projectedShadow"].getOpt<bool>())
            {
                model.projectedShadow = *projectedShadow;
            }

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
