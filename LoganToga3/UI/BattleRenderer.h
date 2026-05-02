#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/UnitCatalog.h"
# include "BattleDebugOverlay.h"
# include "QuarterView.h"
# include "MapEditor.h"

namespace LT3
{
    struct ClickDebugState
    {
        Vec2 currentScreen{ 0, 0 };
        Vec2 currentWorld{ 0, 0 };
        Optional<Point> currentCell;
        Optional<Vec2> lastLeftScreen;
        Optional<Vec2> lastLeftWorld;
        Optional<Point> lastLeftCell;
        Optional<Vec2> lastRightScreen;
        Optional<Vec2> lastRightWorld;
        Optional<Point> lastRightCell;
    };

    inline String FormatFaction(Faction faction)
    {
        switch (faction)
        {
        case Faction::Player:
            return U"Player";
        case Faction::Enemy:
            return U"Enemy";
        default:
            return U"Neutral";
        }
    }

    inline void DrawClickDebugOverlay(const ClickDebugState& debugState, const Font& uiFont)
    {
        const RectF panel{ 24, 72, 520, 176 };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Click Debug").draw(44, 88, Palette::White);
        uiFont(U"Cursor screen: ({:.1f}, {:.1f})"_fmt(debugState.currentScreen.x, debugState.currentScreen.y)).draw(13, 44, 116, Palette::Lightgray);
        uiFont(U"Cursor world : ({:.1f}, {:.1f})"_fmt(debugState.currentWorld.x, debugState.currentWorld.y)).draw(13, 44, 136, Palette::Lightgray);
        uiFont(debugState.currentCell
            ? U"Cursor cell  : ({}, {})"_fmt(debugState.currentCell->x, debugState.currentCell->y)
            : U"Cursor cell  : (n/a)").draw(13, 44, 156, Palette::Lightgray);

        const String lastLeft = debugState.lastLeftScreen
            ? U"L screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
                debugState.lastLeftScreen->x,
                debugState.lastLeftScreen->y,
                debugState.lastLeftWorld ? debugState.lastLeftWorld->x : 0.0,
                debugState.lastLeftWorld ? debugState.lastLeftWorld->y : 0.0,
                debugState.lastLeftCell ? U"({}, {})"_fmt(debugState.lastLeftCell->x, debugState.lastLeftCell->y) : U"n/a")
            : U"L screen=(n/a) world=(n/a) cell=n/a";
        const String lastRight = debugState.lastRightScreen
            ? U"R screen=({:.1f}, {:.1f}) world=({:.1f}, {:.1f}) cell={}"_fmt(
                debugState.lastRightScreen->x,
                debugState.lastRightScreen->y,
                debugState.lastRightWorld ? debugState.lastRightWorld->x : 0.0,
                debugState.lastRightWorld ? debugState.lastRightWorld->y : 0.0,
                debugState.lastRightCell ? U"({}, {})"_fmt(debugState.lastRightCell->x, debugState.lastRightCell->y) : U"n/a")
            : U"R screen=(n/a) world=(n/a) cell=n/a";

        uiFont(lastLeft).draw(13, 44, 184, Palette::Skyblue);
        uiFont(lastRight).draw(13, 44, 204, Palette::Orange);
    }

    inline void DrawSelectionDebugOverlay(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont)
    {
        const RectF panel{ 24, 256, 520, 64 };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Selection Debug").draw(44, 272, Palette::White);

        String selectionText = U"Selected unit: id=n/a tag=n/a";
        const UnitId selected = GetSelectedUnit(world);
        if (selected != InvalidUnitId)
        {
            selectionText = U"Selected unit: id={} tag={}"_fmt(selected, defs.units[world.units.defId[selected]].tag);
        }

        uiFont(selectionText).draw(13, 44, 296, Palette::Lightgray);
    }

    inline void DrawResourceNodes(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont)
    {
        for (size_t i = 0; i < world.resourceNodes.position.size(); ++i)
        {
            const Vec2 pos = ToQuarterScreen(world.resourceNodes.position[i]);
            const bool depleted = world.resourceNodes.amount[i] <= 0;
            ColorF color = defs.resources[world.resourceNodes.defId[i]].color;
            ColorF textColor{ 1.0, 1.0, 1.0 };
            if (depleted)
            {
                color = ColorF{ 0.3, 0.25, 0.08 };
                textColor = ColorF{ 0.5, 0.5, 0.5 };
            }
            Ellipse{ pos + Vec2{ 0, 6 }, 30, 15 }.draw(ColorF{ 0, 0, 0, 0.25 });
            Circle{ pos, 22 }.draw(color);
            Circle{ pos, 22 }.drawFrame(2, ColorF{ 0.25, 0.18, 0.02 });
            uiFont(U"{}"_fmt(world.resourceNodes.amount[i])).drawAt(13, pos + Vec2{ 0, 34 }, textColor);
        }
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

    inline ColorF TileColor(int32 x, int32 y)
    {
        const bool alternate = ((x + y) % 2) == 0;
        if (alternate)
        {
            return ColorF{ 0.18, 0.31, 0.22 };
        }
        return ColorF{ 0.14, 0.27, 0.20 };
    }

    inline void DrawQuarterMap(const BattleWorld& world, const DefinitionStores&, const Font&)
    {
        for (int32 diagonal = 0; diagonal < (world.mapWidth + world.mapHeight - 1); ++diagonal)
        {
            const int32 xBegin = Max(0, diagonal - (world.mapHeight - 1));
            const int32 xEnd = Min(world.mapWidth - 1, diagonal);
            for (int32 x = xBegin; x <= xEnd; ++x)
            {
                const int32 y = diagonal - x;
                const Vec2 center{ 200.0 + x * QuarterTileStep, 90.0 + y * QuarterTileStep };
                const Quad tile = ToQuarterTile(center);
                tile.draw(TileColor(x, y));
                tile.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.08 });
            }
        }
    }

    inline void DrawUiLayoutDragHandle(const RectF& panelRect, bool active)
    {
        if (!active)
        {
            return;
        }

        const RectF handle = UiLayoutDragHandleRect(panelRect);
        handle.draw(ColorF{ 1.0, 0.84, 0.0, 0.18 }).drawFrame(2.0, ColorF{ 1.0, 0.84, 0.0, 0.90 });
    }

    inline void DrawSelectedUnitPanel(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const Font& uiFont, bool showDebugInfo)
    {
        const UnitId selected = GetSelectedUnit(world);
        if (selected == InvalidUnitId)
        {
            return;
        }

        const UnitDef& def = defs.units[world.units.defId[selected]];
        const bool showDetail = KeyControl.pressed();

        const RectF info = showDetail
            ? BattleInfoPanelDetailRect(mapEditor)
            : BattleInfoPanelCompactRect(mapEditor);
        info.draw(ColorF{ 0.02, 0.03, 0.045, 0.78 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
        DrawUiLayoutDragHandle(info, mapEditor.uiLayoutEditEnabled);

        const double nameY = info.y + 18.0;
        uiFont(def.name).draw(info.x + 18.0, nameY, Palette::White);

        const int32 maxHp = Max(1, def.hp);
        const double hpRate = Clamp(static_cast<double>(world.units.hp[selected]) / maxHp, 0.0, 1.0);
        const double hpY = nameY + 34.0;
        uiFont(U"HP:").draw(info.x + 18.0, hpY, Palette::Lightgray);

        const RectF hpBack{ info.x + 66.0, hpY + 4.0, 180.0, 14.0 };
        hpBack.draw(ColorF{ 0.08, 0.08, 0.08, 0.82 });
        ColorF hpColor{ 1.0, 0.25, 0.20 };
        if (hpRate > 0.35)
        {
            hpColor = ColorF{ 0.20, 0.80, 0.20 };
        }
        RectF{ hpBack.pos, hpBack.w * hpRate, hpBack.h }.draw(hpColor);
        hpBack.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });

        uiFont(U"{}/{}  {}%"_fmt(world.units.hp[selected], def.hp, static_cast<int32>(hpRate * 100.0))).draw(info.x + 258.0, hpY, Palette::White);
        if (hpRate <= 0.30)
        {
            uiFont(U"⚠").draw(info.x + 412.0, hpY, ColorF{ 1.0, 0.70, 0.20 });
        }

        if (!showDetail)
        {
            uiFont(U"Ctrl: details").draw(info.x + 18.0, info.y + info.h - 24.0, ColorF{ 0.70, 0.80, 0.95 });
            return;
        }

        uiFont(U"Tag: {}  Faction: {}"_fmt(def.tag, FormatFaction(world.units.faction[selected]))).draw(info.x + 18.0, info.y + 82.0, Palette::Skyblue);
        uiFont(U"ATK: {}  SPD: {:.0f}  Task: {}"_fmt(def.attack, def.speed, static_cast<int32>(world.units.task[selected]))).draw(info.x + 18.0, info.y + 106.0, Palette::Lightgray);

        const Array<BuildActionDefId>& buildQueue = GetQueuedBuildActions(world, selected);
        if (!buildQueue.isEmpty() && buildQueue.front() < defs.buildActions.size())
        {
            const BuildActionDef& action = defs.buildActions[buildQueue.front()];
            const double rate = Clamp(world.buildQueues.progressSec[selected] / Max(0.001, action.buildTimeSec), 0.0, 1.0);
            uiFont(U"Build: {}  Queue: {}"_fmt(action.name, buildQueue.size())).draw(info.x + 18.0, info.y + 132.0, Palette::Gold);
            RectF{ info.x + 240.0, info.y + 138.0, 180.0, 12.0 }.draw(ColorF{ 0, 0, 0, 0.45 });
            RectF{ info.x + 240.0, info.y + 138.0, 180.0 * rate, 12.0 }.draw(Palette::Gold);

            const size_t previewCount = Min<size_t>(3, buildQueue.size());
            for (size_t i = 0; i < previewCount; ++i)
            {
                const BuildActionDefId queuedActionId = buildQueue[i];
                if (queuedActionId >= defs.buildActions.size())
                {
                    continue;
                }

                const String prefix = (i == 0) ? U">" : U"-";
                uiFont(U"{} {}"_fmt(prefix, defs.buildActions[queuedActionId].name)).draw(16, info.x + 18.0, info.y + 156.0 + static_cast<int32>(i) * 18, Palette::Lightgray);
            }
        }

        if (showDebugInfo)
        {
            DrawBattleDebugOverlay(world, defs, uiFont, info);
        }
    }

    inline void DrawResourcePanel(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont)
    {
        const RectF panel{ 1320.0, 72.0, 250.0, 72.0 };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

        String goldName = U"Gold";
        ColorF goldColor = Palette::Gold;
        if (const auto it = defs.resourceByTag.find(U"gold"); it != defs.resourceByTag.end())
        {
            const ResourceDef& def = defs.resources[it->second];
            goldName = def.name;
            goldColor = def.color;
        }

        uiFont(U"資源").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);
        Circle{ panel.x + 30.0, panel.y + 50.0, 10.0 }.draw(goldColor).drawFrame(1.0, ColorF{ 0.25, 0.18, 0.02 });
        uiFont(U"{}: {}"_fmt(goldName, world.resources.playerGold)).draw(16, panel.x + 48.0, panel.y + 38.0, Palette::White);
    }

    inline void DrawResultOverlay(const BattleWorld& world, const Font& uiFont, const Font& titleFont)
    {
        if (!(world.victory || world.defeat))
        {
            return;
        }

        Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0, 0, 0, 0.58 });
        String resultText = U"DEFEAT";
        ColorF resultColor{ 1.0, 0.25, 0.20 };
        if (world.victory)
        {
            resultText = U"VICTORY";
            resultColor = ColorF{ 1.0, 0.84, 0.0 };
        }
        titleFont(resultText).drawAt(90, Vec2{ 800, 410 }, resultColor);
        uiFont(U"Press ESC or close from the Gaussian menu.").drawAt(800, 500, Palette::White);
    }

    inline void DrawNineSliceTexture(const Texture& texture, const RectF& rect, const double cornerSize)
    {
        if (rect.w <= 0.0 || rect.h <= 0.0)
        {
            return;
        }

        const double left = Min(cornerSize, rect.w * 0.5);
        const double right = left;
        const double top = Min(cornerSize, rect.h * 0.5);
        const double bottom = top;
        const double centerW = Max(0.0, rect.w - left - right);
        const double centerH = Max(0.0, rect.h - top - bottom);
        constexpr int32 sourceCorner = 16;
        constexpr int32 sourceCenter = 64;

        texture(Rect{ 0, 0, sourceCorner, sourceCorner }).resized(left, top).draw(rect.pos);
        texture(Rect{ 80, 0, sourceCorner, sourceCorner }).resized(right, top).draw(rect.pos.movedBy(rect.w - right, 0));
        texture(Rect{ 0, 80, sourceCorner, sourceCorner }).resized(left, bottom).draw(rect.pos.movedBy(0, rect.h - bottom));
        texture(Rect{ 80, 80, sourceCorner, sourceCorner }).resized(right, bottom).draw(rect.pos.movedBy(rect.w - right, rect.h - bottom));

        if (centerW > 0.0)
        {
            texture(Rect{ sourceCorner, 0, sourceCenter, sourceCorner }).resized(centerW, top).draw(rect.pos.movedBy(left, 0));
            texture(Rect{ sourceCorner, 80, sourceCenter, sourceCorner }).resized(centerW, bottom).draw(rect.pos.movedBy(left, rect.h - bottom));
        }
        if (centerH > 0.0)
        {
            texture(Rect{ 0, sourceCorner, sourceCorner, sourceCenter }).resized(left, centerH).draw(rect.pos.movedBy(0, top));
            texture(Rect{ 80, sourceCorner, sourceCorner, sourceCenter }).resized(right, centerH).draw(rect.pos.movedBy(rect.w - right, top));
        }
        if (centerW > 0.0 && centerH > 0.0)
        {
            texture(Rect{ sourceCorner, sourceCorner, sourceCenter, sourceCenter }).resized(centerW, centerH).draw(rect.pos.movedBy(left, top));
        }
    }

    struct UnitVisualInfo
    {
        String image;
        String kind;
        double visualScale = 1.0;
    };

    struct BattleRenderAssets
    {
        HashTable<String, UnitVisualInfo> unitVisualByTag;
        mutable HashTable<String, Texture> unitTextureCache;
        mutable HashTable<String, Texture> iconTextureCache;
    };

    inline void DrawAreaSelectionFrame(const BattleWorld& world, const BattleRenderAssets& assets)
    {
        if (!IsDragSelectionActive(world))
        {
            return;
        }

        const RectF rect = MakeDragSelectionRect(world.selection.areaDragStartScreen, world.selection.areaDragCurrentScreen);
        const FilePath framePath = ResolveSystemImagePath(U"areaWaku.png");
        if (!FileSystem::Exists(framePath))
        {
            rect.drawFrame(2, ColorF{ 0.2, 0.8, 1.0, 0.9 });
            return;
        }

        if (!assets.iconTextureCache.contains(framePath))
        {
            assets.iconTextureCache.emplace(framePath, Texture{ framePath });
        }

        DrawNineSliceTexture(assets.iconTextureCache.at(framePath), rect, 16.0);
    }

    inline void DrawFormationPlacementPreview(const BattleWorld& world, const DefinitionStores& defs)
    {
        if (!world.selection.formationPlacementActive || world.selection.formationUnits.isEmpty())
        {
            return;
        }

        const Vec2 destinationWorld = world.selection.formationDestinationWorld;
        const Vec2 facing = ResolveFormationFacingDirection(world, world.selection.formationUnits, destinationWorld, world.selection.formationCurrentWorld);
        const Array<Vec2> targets = BuildFormationMoveTargets(world, defs, world.selection.formationUnits, destinationWorld, facing);

        for (const Vec2& target : targets)
        {
            const Vec2 screen = ToQuarterViewportScreen(target);
            Circle{ screen, 12.0 }.draw(ColorF{ 0.0, 0.75, 1.0, 0.10 }).drawFrame(2.0, ColorF{ 0.0, 0.85, 1.0, 0.85 });
        }

        const Vec2 destinationScreen = ToQuarterViewportScreen(destinationWorld);
        Vec2 arrowTargetWorld = world.selection.formationCurrentWorld;
        if ((arrowTargetWorld - destinationWorld).lengthSq() < 16.0)
        {
            arrowTargetWorld = destinationWorld + (facing * 120.0);
        }
        const Vec2 arrowTargetScreen = ToQuarterViewportScreen(arrowTargetWorld);

        Circle{ destinationScreen, 7.0 }.draw(ColorF{ 1.0, 0.84, 0.0, 0.90 });
        Line{ destinationScreen, arrowTargetScreen }.drawArrow(4.0, Vec2{ 18.0, 18.0 }, ColorF{ 1.0, 0.84, 0.0, 0.95 });
    }

    inline BattleRenderAssets BuildBattleRenderAssets(const UnitCatalog& catalog)
    {
        BattleRenderAssets assets;
        for (const auto& entry : catalog.entries)
        {
            if (!entry.tag.isEmpty())
            {
                assets.unitVisualByTag[entry.tag] = UnitVisualInfo{ entry.image, entry.kind, entry.visualScale };
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

        const FilePath unitPath = (visual.kind.lowercased() == U"building")
            ? ResolveBuildingChipPath(visual.image)
            : ResolveUnitChipPath(visual.image);
        if (!FileSystem::Exists(unitPath))
        {
            return false;
        }

        if (!assets.unitTextureCache.contains(unitPath))
        {
            assets.unitTextureCache.emplace(unitPath, Texture{ unitPath });
        }
        const Texture& texture = assets.unitTextureCache.at(unitPath);
        const double imageSize = ((visual.kind.lowercased() == U"building") ? (def.radius * 2.2) : (def.radius * 2.0)) * visual.visualScale;
        texture.resized(imageSize, imageSize).drawAt(pos);
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
                return (visual.kind.lowercased() == U"building")
                    ? ResolveBuildingChipPath(visual.image)
                    : ResolveUnitChipPath(visual.image);
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
        Ellipse{ pos + Vec2{ 0, def.radius * 0.65 }, def.radius * 1.15, def.radius * 0.45 }.draw(ColorF{ 0, 0, 0, 0.28 });
        Circle{ pos, def.radius + (selected ? 6.0 : 2.0) }.drawFrame(selected ? 4.0 : 2.0, outline);

        if (!(assets && DrawUnitTexture(*assets, def, pos)))
        {
            DrawUnitShape(def, pos, outline);
        }
    }

    inline void DrawUnits(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const BattleRenderAssets* assets = nullptr)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!world.units.alive[unit]) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
            const bool selected = IsUnitSelected(world, unit);
            const ColorF outline = GetUnitOutlineColor(world, unit);

            DrawUnitMovePath(world, unit, outline);
            DrawUnitVisual(def, pos, selected, outline, assets);

            DrawHealthBar(pos, def.radius, world.units.hp[unit], def.hp);
            uiFont(def.name).drawAt(13, pos + Vec2{ 0, def.radius + 17 }, Palette::White);
        }
    }

    inline RectF BuildCommandIconRect(int32 index)
    {
        constexpr int32 columns = 3;
        const int32 col = index % columns;
        const int32 row = index / columns;
        const Vec2 origin{ 1110, 690 };
        const Vec2 step{ 88, 88 };
        return RectF{ origin + Vec2{ col * step.x, row * step.y }, 78, 78 };
    }

    inline RectF BattleBuildQueuePanelRect(const MapEditorState& mapEditor, int32 rows, size_t previewCount)
    {
        const RectF commandPanel = BattleCommandPanelRect(mapEditor, rows);
        const double desiredHeight = 94.0 + static_cast<double>(previewCount) * 62.0;
        const double panelHeight = Max(commandPanel.h, desiredHeight);
        return RectF{ commandPanel.x - 304.0, commandPanel.y + commandPanel.h - panelHeight, 292.0, panelHeight };
    }

    inline void DrawSelectedBuildQueuePanel(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const BattleRenderAssets& assets, const Font& uiFont, int32 commandRows)
    {
        const UnitId selected = GetSelectedUnit(world);
        const Array<BuildActionDefId>& queue = GetQueuedBuildActions(world, selected);
        if (queue.isEmpty())
        {
            return;
        }

        const size_t previewCount = Min<size_t>(4, queue.size());
        const RectF panel = BattleBuildQueuePanelRect(mapEditor, commandRows, previewCount);
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });
        uiFont(U"キュー").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);
        uiFont(U"{}件"_fmt(queue.size())).draw(14, panel.x + panel.w - 56.0, panel.y + 12.0, Palette::Gold);

        for (size_t i = 0; i < previewCount; ++i)
        {
            const BuildActionDefId actionId = queue[i];
            if (actionId >= defs.buildActions.size())
            {
                continue;
            }

            const BuildActionDef& action = defs.buildActions[actionId];
            const RectF slot{ panel.x + 16.0, panel.y + 42.0 + static_cast<double>(i) * 62.0, 56.0, 56.0 };
            slot.draw(i == 0 ? ColorF{ 0.14, 0.12, 0.06, 0.96 } : ColorF{ 0.08, 0.08, 0.10, 0.92 });
            slot.drawFrame(2.0, i == 0 ? ColorF{ 1.0, 0.84, 0.0, 0.90 } : ColorF{ 1, 1, 1, 0.18 });

            if (!DrawBuildActionIcon(action, defs, assets, slot.center().movedBy(0, -3), 42.0))
            {
                uiFont(U"{}"_fmt(i + 1)).drawAt(16, slot.center().movedBy(0, -3), Palette::White);
            }

            uiFont(action.name).draw(13, panel.x + 82.0, slot.y + 8.0, i == 0 ? Palette::Gold : Palette::Lightgray);
        }

        const BuildActionDefId currentActionId = queue.front();
        if (currentActionId < defs.buildActions.size())
        {
            const BuildActionDef& action = defs.buildActions[currentActionId];
            const double rate = Clamp(world.buildQueues.progressSec[selected] / Max(0.001, action.buildTimeSec), 0.0, 1.0);
            const RectF progressBack{ panel.x + 16.0, panel.y + panel.h - 24.0, panel.w - 32.0, 10.0 };
            progressBack.draw(ColorF{ 0, 0, 0, 0.48 });
            RectF{ progressBack.pos, progressBack.w * rate, progressBack.h }.draw(Palette::Gold);
            progressBack.drawFrame(1.0, ColorF{ 1, 1, 1, 0.14 });
            uiFont(action.name).draw(13, panel.x + 16.0, panel.y + panel.h - 48.0, Palette::Lightgray);
        }
    }

    inline void DrawQuarterCommandBar(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor, const BattleRenderAssets& assets, const Font& uiFont)
    {
        const Array<BuildActionUiState> visibleActions = CollectVisibleBuildActionsForSelectedUnit(world, defs);

        if (visibleActions.isEmpty())
        {
            return;
        }

        const int32 rows = (static_cast<int32>(visibleActions.size()) + 2) / 3;
        const RectF panel = BattleCommandPanelRect(mapEditor, rows);
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });
        DrawUiLayoutDragHandle(panel, mapEditor.uiLayoutEditEnabled);

        for (int32 visibleIndex = 0; visibleIndex < static_cast<int32>(visibleActions.size()); ++visibleIndex)
        {
            const BuildActionUiState& actionState = visibleActions[visibleIndex];
            const BuildActionDef& action = defs.buildActions[actionState.actionId];
            const RectF rect = BattleCommandIconRect(mapEditor, visibleIndex);
            const bool affordable = actionState.affordable;

            ColorF backColor{ 0.08, 0.08, 0.10, 0.92 };
            if (affordable)
            {
                backColor = ColorF{ 0.12, 0.20, 0.16, 0.96 };
            }
            rect.draw(backColor);

            ColorF frameColor{ 1, 1, 1, 0.18 };
            if (rect.mouseOver())
            {
                frameColor = ColorF{ 1.0, 0.84, 0.0 };
            }
            rect.drawFrame(2, frameColor);

            const bool hasIcon = DrawBuildActionIcon(action, defs, assets, rect.center().movedBy(0, -5), 60.0);

            if (!hasIcon)
            {
                uiFont(U"{}"_fmt(visibleIndex + 1)).drawAt(16, rect.center().movedBy(0, -4), Palette::White);
            }

            const ColorF costColor = affordable ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.25, 0.20 };
            uiFont(U"{}G"_fmt(action.costGold)).drawAt(12, rect.center().movedBy(0, 26), costColor);
        }

        DrawSelectedBuildQueuePanel(world, defs, mapEditor, assets, uiFont, rows);
    }

    inline void DrawBattleWorld(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const MapEditorState& mapEditor, const ClickDebugState& debugState, bool showDebugInfo, const Font& uiFont, const Font& titleFont)
    {
        Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0.06, 0.10, 0.09 });

        {
            const auto cameraTransform = CreateQuarterViewTransformer();

            DrawMapEditorTerrainLayer(mapEditor);
            DrawResourceNodes(world, defs, uiFont);
            DrawProjectiles(world, defs);
            DrawUnits(world, defs, uiFont, &assets);

            DrawMapEditorObjectLayer(mapEditor);
        }

        DrawQuarterCommandBar(world, defs, mapEditor, assets, uiFont);
        DrawResourcePanel(world, defs, uiFont);
        DrawAreaSelectionFrame(world, assets);
        DrawFormationPlacementPreview(world, defs);
        if (showDebugInfo)
        {
            DrawClickDebugOverlay(debugState, uiFont);
            DrawSelectionDebugOverlay(world, defs, uiFont);
        }
        DrawSelectedUnitPanel(world, defs, mapEditor, uiFont, showDebugInfo);
        DrawResultOverlay(world, uiFont, titleFont);
    }
}
