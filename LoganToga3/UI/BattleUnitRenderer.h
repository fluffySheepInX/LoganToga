#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/UnitCatalog.h"
# include "QuarterView.h"

namespace LT3
{
    struct UnitVisualInfo
    {
        String image;
        String kind;
        double visualScale = 1.0;
        Point visualOffset{ 0, 0 };
        Point shadowOffset{ 0, 0 };
    };

    struct BattleRenderAssets
    {
        HashTable<String, UnitVisualInfo> unitVisualByTag;
        mutable HashTable<String, Texture> unitTextureCache;
        mutable HashTable<String, Texture> iconTextureCache;
        mutable HashTable<String, Texture> resourceTextureCache;
    };

    inline BattleRenderAssets BuildBattleRenderAssets(const UnitCatalog& catalog)
    {
        BattleRenderAssets assets;
        for (const auto& entry : catalog.entries)
        {
            if (!entry.tag.isEmpty())
            {
                assets.unitVisualByTag[entry.tag] = UnitVisualInfo{ entry.image, entry.kind, entry.visualScale, entry.visualOffset, entry.shadowOffset };
            }
        }
        return assets;
    }

    inline UnitVisualInfo FindUnitVisualInfoByTag(const BattleRenderAssets& assets, const String& unitTag)
    {
        if (assets.unitVisualByTag.contains(unitTag))
        {
            return assets.unitVisualByTag.at(unitTag);
        }
        return UnitVisualInfo{};
    }

    inline void DrawHealthBar(const Vec2& pos, double radius, int32 hp, int32 maxHp)
    {
        const RectF back{ Arg::center = pos + Vec2{ 0, -radius - 12 }, radius * 2.2, 5 };
        back.draw(ColorF{ 0.08, 0.08, 0.08, 0.75 });
        const double rate = maxHp > 0 ? Clamp(static_cast<double>(hp) / maxHp, 0.0, 1.0) : 0.0;
        ColorF barColor{ 1.0, 0.25, 0.20 };
        if (rate > 0.35)
        {
            barColor = ColorF{ 0.20, 0.80, 0.20 };
        }
        RectF{ back.pos, back.w * rate, back.h }.draw(barColor);
    }

    inline void DrawSelectionDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        const RectF panel{ 24, 256, 640, 136 };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Selection Debug").draw(44, 272, Palette::White);

        String selectionText = U"Selected unit: id=n/a tag=n/a";
        String visualText = U"Visual: kind=n/a image=n/a exists=n/a";
        String pathText = U"Path: n/a";
        const UnitId selected = GetSelectedUnit(world);
        if (selected != InvalidUnitId)
        {
            const UnitDef& def = defs.units[world.units.defId[selected]];
            selectionText = U"Selected unit: id={} tag={} role={}"_fmt(selected, def.tag, static_cast<int32>(def.role));

            const UnitVisualInfo visualInfo = FindUnitVisualInfoByTag(assets, def.tag);
            const FilePath resolvedPath = ResolveCatalogVisualPath(visualInfo.kind, visualInfo.image);

            const String visualKind = visualInfo.kind.isEmpty() ? U"<empty>" : visualInfo.kind;
            const String visualImage = visualInfo.image.isEmpty() ? U"<empty>" : visualInfo.image;
            const String visualExists = (!resolvedPath.isEmpty() && FileSystem::Exists(resolvedPath)) ? U"yes" : U"no";
            visualText = U"Visual: kind={} image={} exists={}"_fmt(visualKind, visualImage, visualExists);
            pathText = U"Path: {}"_fmt(resolvedPath.isEmpty() ? U"<empty>" : resolvedPath);
        }

        uiFont(selectionText).draw(13, 44, 296, Palette::Lightgray);
        uiFont(visualText).draw(13, 44, 316, Palette::Skyblue);
        uiFont(pathText).draw(11, 44, 336, Palette::Lightgray);
        uiFont(U"Map: {} x {}"_fmt(world.mapWidth, world.mapHeight)).draw(11, 44, 358, ColorF{ 0.76, 0.80, 0.88 });
    }

    inline ColorF GetUnitOutlineColor(const BattleWorld& world, UnitId unit)
    {
        if (world.units.faction[unit] == Faction::Player)
        {
            return ColorF{ 0.0, 1.0, 1.0 };
        }

        return ColorF{ 1.0, 0.15, 0.10 };
    }

    inline void DrawUnitMovePath(const BattleWorld& world, UnitId unit, const ColorF& outline)
    {
        if (world.units.task[unit] != UnitTask::Moving)
        {
            return;
        }

        const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
        const Vec2 targetPos = ToQuarterScreen(world.units.targetPosition[unit]);
        Line{ pos, targetPos }.draw(1.5, ColorF{ outline, 0.35 });
        Circle{ targetPos, 5 }.drawFrame(1.5, ColorF{ outline, 0.45 });
    }

    inline void DrawProjectiles(const BattleWorld& world, const DefinitionStores& defs)
    {
        for (size_t i = 0; i < world.projectiles.position.size(); ++i)
        {
            const SkillDef& skill = defs.skills[world.projectiles.skill[i]];
            Circle{ ToQuarterScreen(world.projectiles.position[i]), 4 }.draw(skill.color);
        }
    }

    inline void DrawUnitShape(const UnitDef& def, const Vec2& pos, const ColorF& outline)
    {
        Circle{ pos, def.radius }.draw(def.color);

        if (def.role == UnitRole::Base)
        {
            RectF{ Arg::center = pos, def.radius * 1.45, def.radius * 1.45 }.draw(def.color);
            RectF{ Arg::center = pos, def.radius * 1.45, def.radius * 1.45 }.drawFrame(2, outline);
        }
    }

    inline void DrawUnitShape(const UnitDef& def, const Vec2& pos, bool selected, const ColorF& outline)
    {
        Ellipse{ pos + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
        Circle{ pos, def.radius + (selected ? 6.0 : 2.0) }.drawFrame(selected ? 4.0 : 2.0, outline);
        DrawUnitShape(def, pos, outline);
    }

    inline bool DrawUnitTexture(const BattleRenderAssets& assets, const UnitDef& def, const Vec2& pos)
    {
        const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, def.tag);
        if (visual.image.isEmpty())
        {
            return false;
        }

        const FilePath unitPath = ResolveCatalogVisualPath(visual.kind, visual.image);
        if (!FileSystem::Exists(unitPath))
        {
            return false;
        }

        if (!assets.unitTextureCache.contains(unitPath))
        {
            assets.unitTextureCache.emplace(unitPath, Texture{ unitPath });
        }
        const Texture& texture = assets.unitTextureCache.at(unitPath);
        const bool isBuildingVisual = (visual.kind.lowercased() == U"building");
        const double imageSize = (isBuildingVisual ? (def.radius * 2.2) : (def.radius * 2.0)) * visual.visualScale;
        const Vec2 visualOffset = Vec2{ static_cast<double>(visual.visualOffset.x), static_cast<double>(visual.visualOffset.y) } * visual.visualScale;
        if (isBuildingVisual)
        {
            texture.resized(imageSize, imageSize).draw(Arg::bottomCenter = pos + visualOffset);
        }
        else
        {
            texture.resized(imageSize, imageSize).drawAt(pos + visualOffset);
        }
        return true;
    }

    inline FilePath ResolveBuildActionIconPath(const BuildActionDef& action, const DefinitionStores& defs, const BattleRenderAssets& assets)
    {
        if (action.spawnUnit < defs.units.size())
        {
            const String unitTag = defs.units[action.spawnUnit].tag;
            const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, unitTag);
            if (!visual.image.isEmpty())
            {
                const FilePath iconPath = ResolveCatalogVisualPath(visual.kind, visual.image);
                if (FileSystem::Exists(iconPath))
                {
                    return iconPath;
                }
            }
        }

        if (!action.icon.isEmpty())
        {
            return ResolveBuildIconPath(action.icon);
        }

        return FilePath{};
    }

    inline bool DrawBuildActionIcon(const BuildActionDef& action, const DefinitionStores& defs, const BattleRenderAssets& assets, const Vec2& center, double size)
    {
        const FilePath iconPath = ResolveBuildActionIconPath(action, defs, assets);
        if (iconPath.isEmpty() || !FileSystem::Exists(iconPath))
        {
            return false;
        }

        if (!assets.iconTextureCache.contains(iconPath))
        {
            assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
        }
        assets.iconTextureCache.at(iconPath).resized(size, size).drawAt(center);
        return true;
    }

    inline void DrawUnitVisual(const UnitDef& def, const Vec2& pos, bool selected, const ColorF& outline, const BattleRenderAssets* assets)
    {
        const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
        Vec2 shadowCenter = pos;
        if (assets)
        {
            const UnitVisualInfo visual = FindUnitVisualInfoByTag(*assets, def.tag);
            if (!visual.image.isEmpty())
            {
                const Vec2 shadowOffset = Vec2{ static_cast<double>(visual.shadowOffset.x), static_cast<double>(visual.shadowOffset.y) } * visual.visualScale;
                shadowCenter += shadowOffset;
            }
        }

        Ellipse{ shadowCenter + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
        if (!isStaticStructure)
        {
            Circle{ pos, def.radius + (selected ? 6.0 : 2.0) }.drawFrame(selected ? 4.0 : 2.0, outline);
        }

        if (!(assets && DrawUnitTexture(*assets, def, pos)))
        {
            DrawUnitShape(def, pos, outline);
        }
    }

    inline void DrawUnits(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const BattleRenderAssets* assets = nullptr)
    {
        const Vec2 mouse = Cursor::PosF();
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!world.units.alive[unit]) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
            const bool selected = IsUnitSelected(world, unit);
            const ColorF outline = GetUnitOutlineColor(world, unit);

            DrawUnitMovePath(world, unit, outline);
            DrawUnitVisual(def, pos, selected, outline, assets);

            const bool isStaticStructure = (def.role == UnitRole::Base || def.role == UnitRole::Barrier);
            const bool hovered = (mouse.distanceFrom(pos) <= (def.radius + 16.0));
            if (!isStaticStructure || hovered)
            {
                DrawHealthBar(pos, def.radius, world.units.hp[unit], def.hp);
            }
        }
    }
}
