# pragma once
# include <Siv3D.hpp>

namespace road
{
    enum class PlacementCategory
    {
        Grass,
        Pebble,
        DirtDecal,
        Prop,
    };

    enum class PlacementMode
    {
        Single,
        Brush,
        Erase,
        Select,
    };

    enum class PlacementRenderType
    {
        Model,
        Texture,
    };

    struct PlacementAsset
    {
        String id;
        String displayName;
        PlacementCategory category = PlacementCategory::Grass;
        PlacementRenderType renderType = PlacementRenderType::Model;
        FilePath resourcePath;
        double defaultScaleMin = 1.0;
        double defaultScaleMax = 1.0;
        bool alignToGround = true;
        bool randomYaw = true;
    };

    struct PlacedScatterItem
    {
        String assetId;
        PlacementCategory category = PlacementCategory::Grass;
        Vec3 position{ 0, 0, 0 };
        double yawRadians = 0.0;
        double scale = 1.0;
        ColorF tint{ 1.0 };
    };

    struct PlacementSettings
    {
        size_t activeCategoryIndex = 0;
        size_t activeModeIndex = 0;
        String activeAssetId = U"grass_short";
        double brushRadius = 1.5;
        double density = 0.55;
        double jitter = 0.35;
        double edgeBias = 0.8;
        double intersectionBoost = 0.4;
    };

    struct PlacementDensityProfile
    {
        double grassDensity = 0.0;
        double pebbleDensity = 0.0;
        double mudDensity = 0.0;
    };

    [[nodiscard]] inline Array<String> PlacementCategoryLabels()
    {
        return{ U"Grass", U"Pebble", U"Dirt", U"Props" };
    }

    [[nodiscard]] inline Array<String> PlacementModeLabels()
    {
        return{ U"Single", U"Brush", U"Erase", U"Select" };
    }

    [[nodiscard]] inline PlacementCategory PlacementCategoryFromIndex(const size_t index)
    {
        switch (index)
        {
        case 1:
            return PlacementCategory::Pebble;
        case 2:
            return PlacementCategory::DirtDecal;
        case 3:
            return PlacementCategory::Prop;
        default:
            return PlacementCategory::Grass;
        }
    }

    [[nodiscard]] inline PlacementMode PlacementModeFromIndex(const size_t index)
    {
        switch (index)
        {
        case 1:
            return PlacementMode::Brush;
        case 2:
            return PlacementMode::Erase;
        case 3:
            return PlacementMode::Select;
        default:
            return PlacementMode::Single;
        }
    }

    [[nodiscard]] inline PlacementCategory ParsePlacementCategory(const String& value)
    {
        const String v = value.lowercased();
        if (v == U"pebble") return PlacementCategory::Pebble;
        if ((v == U"dirt") || (v == U"dirtdecal") || (v == U"mud")) return PlacementCategory::DirtDecal;
        if ((v == U"prop") || (v == U"props")) return PlacementCategory::Prop;
        return PlacementCategory::Grass;
    }

    [[nodiscard]] inline PlacementRenderType ParsePlacementRenderType(const String& value)
    {
        const String v = value.lowercased();
        if ((v == U"texture") || (v == U"decal")) return PlacementRenderType::Texture;
        return PlacementRenderType::Model;
    }

    [[nodiscard]] inline Array<PlacementAsset> DefaultPlacementAssets()
    {
        return{
            { U"grass_short", U"Short Grass", PlacementCategory::Grass, PlacementRenderType::Model, U"example/obj/pine.obj", 0.35, 0.65, true, true },
            { U"grass_dry", U"Dry Grass", PlacementCategory::Grass, PlacementRenderType::Model, U"example/obj/tree.obj", 0.25, 0.45, true, true },
            { U"pebble_small", U"Small Pebbles", PlacementCategory::Pebble, PlacementRenderType::Model, U"example/obj/crystal1.obj", 0.08, 0.16, true, true },
            { U"pebble_scatter", U"Pebble Scatter", PlacementCategory::Pebble, PlacementRenderType::Model, U"example/obj/crystal2.obj", 0.08, 0.16, true, true },
            { U"dirt_soft", U"Soft Dirt Decal", PlacementCategory::DirtDecal, PlacementRenderType::Texture, U"example/texture/ground.jpg", 0.9, 1.5, true, true },
            { U"mud_edge", U"Mud Edge Decal", PlacementCategory::DirtDecal, PlacementRenderType::Texture, U"texture/road.jpg", 0.9, 1.8, true, true },
            { U"prop_rock", U"Small Rock", PlacementCategory::Prop, PlacementRenderType::Model, U"example/obj/crystal3.obj", 0.18, 0.32, true, true },
            { U"prop_tuft", U"Grass Tuft", PlacementCategory::Prop, PlacementRenderType::Model, U"example/obj/siv3d-kun.obj", 0.08, 0.12, true, true },
        };
    }

    inline void LoadPlacementAssetsFromToml(const FilePath& tomlPath, Array<PlacementAsset>& assets)
    {
        if (not FileSystem::Exists(tomlPath))
        {
            return;
        }

        const TOMLReader toml{ tomlPath };
        if (not toml)
        {
            return;
        }

        Array<PlacementAsset> loaded;
        for (const auto& assetValue : toml[U"assets"].tableArrayView())
        {
            PlacementAsset asset;
            asset.id = assetValue[U"id"].getOr<String>(U"");
            asset.displayName = assetValue[U"displayName"].getOr<String>(asset.id);
            asset.category = ParsePlacementCategory(assetValue[U"category"].getOr<String>(U"grass"));
            asset.renderType = ParsePlacementRenderType(assetValue[U"renderType"].getOr<String>(U"model"));
            asset.resourcePath = assetValue[U"resourcePath"].getOr<String>(U"");
            asset.defaultScaleMin = assetValue[U"defaultScaleMin"].getOr<double>(asset.defaultScaleMin);
            asset.defaultScaleMax = assetValue[U"defaultScaleMax"].getOr<double>(asset.defaultScaleMax);
            asset.alignToGround = assetValue[U"alignToGround"].getOr<bool>(asset.alignToGround);
            asset.randomYaw = assetValue[U"randomYaw"].getOr<bool>(asset.randomYaw);

            if (asset.id.isEmpty())
            {
                continue;
            }

            if (asset.defaultScaleMax < asset.defaultScaleMin)
            {
                std::swap(asset.defaultScaleMin, asset.defaultScaleMax);
            }

            loaded << std::move(asset);
        }

        if (not loaded.isEmpty())
        {
            assets = std::move(loaded);
        }
    }
}
