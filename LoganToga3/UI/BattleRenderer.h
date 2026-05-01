#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
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
        if (IsValidUnit(world, world.selection.selected))
        {
            const UnitId selected = world.selection.selected;
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

    inline void DrawSelectedUnitPanel(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont)
    {
        if (!IsValidUnit(world, world.selection.selected))
        {
            return;
        }

        const UnitId selected = world.selection.selected;
        const UnitDef& def = defs.units[world.units.defId[selected]];
        RectF info{ 24, 640, 1520, 190 };
        info.draw(ColorF{ 0.02, 0.03, 0.045, 0.78 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
        uiFont(U"Selected: {} ({})"_fmt(def.name, FormatFaction(world.units.faction[selected]))).draw(42, 662, Palette::White);
        uiFont(U"Tag: {}"_fmt(def.tag)).draw(42, 686, Palette::Skyblue);
        uiFont(U"HP: {}/{}  ATK: {}  SPD: {:.0f}  Task: {}"_fmt(world.units.hp[selected], def.hp, def.attack, def.speed, static_cast<int32>(world.units.task[selected]))).draw(42, 710, Palette::Lightgray);
        DrawBattleDebugOverlay(world, defs, uiFont, info);

        if (world.units.task[selected] == UnitTask::Building && world.units.buildAction[selected] < defs.buildActions.size())
        {
            const BuildActionDef& action = defs.buildActions[world.units.buildAction[selected]];
            const double rate = Clamp(world.units.buildProgressSec[selected] / action.buildTimeSec, 0.0, 1.0);
            RectF{ 290, 777, 180, 12 }.draw(ColorF{ 0, 0, 0, 0.45 });
            RectF{ 290, 777, 180 * rate, 12 }.draw(Palette::Gold);
        }
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

    struct UnitVisualInfo
    {
        String image;
        String kind;
    };

    struct BattleRenderAssets
    {
        HashTable<String, UnitVisualInfo> unitVisualByTag;
        mutable HashTable<String, Texture> unitTextureCache;
        mutable HashTable<String, Texture> iconTextureCache;
    };

    inline BattleRenderAssets BuildBattleRenderAssets(const UnitCatalog& catalog)
    {
        BattleRenderAssets assets;
        for (const auto& entry : catalog.entries)
        {
            if (!entry.tag.isEmpty())
            {
                assets.unitVisualByTag[entry.tag] = UnitVisualInfo{ entry.image, entry.kind };
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
        const double imageSize = (visual.kind.lowercased() == U"building") ? (def.radius * 2.2) : (def.radius * 2.0);
        texture.resized(imageSize, imageSize).drawAt(pos);
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
            const bool selected = world.selection.selected == unit;
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

    inline void DrawQuarterCommandBar(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        const Array<BuildActionUiState> visibleActions = CollectVisibleBuildActions(world, defs, world.selection.selected);

        if (visibleActions.isEmpty())
        {
            return;
        }

        const int32 rows = (static_cast<int32>(visibleActions.size()) + 2) / 3;
        const RectF panel{ 1088, 668, 286, Max(112.0, rows * 88.0 + 24.0) };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.20 });

        for (int32 visibleIndex = 0; visibleIndex < static_cast<int32>(visibleActions.size()); ++visibleIndex)
        {
            const BuildActionUiState& actionState = visibleActions[visibleIndex];
            const BuildActionDef& action = defs.buildActions[actionState.actionId];
            const RectF rect = BuildCommandIconRect(visibleIndex);
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

            FilePath iconPath;
            if (action.spawnUnit < defs.units.size())
            {
                const String unitTag = defs.units[action.spawnUnit].tag;
                const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, unitTag);
                if (!visual.image.isEmpty())
                {
                    iconPath = (visual.kind.lowercased() == U"building")
                        ? ResolveBuildingChipPath(visual.image)
                        : ResolveUnitChipPath(visual.image);
                }
            }
            if (iconPath.isEmpty() && !action.icon.isEmpty())
            {
                iconPath = ResolveBuildIconPath(action.icon);
            }

            const bool hasIcon = (!iconPath.isEmpty() && FileSystem::Exists(iconPath));
            if (hasIcon)
            {
                if (!assets.iconTextureCache.contains(iconPath))
                {
                    assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
                }
                const Texture& texture = assets.iconTextureCache.at(iconPath);
                texture.resized(60, 60).drawAt(rect.center().movedBy(0, -5));
            }

            if (!hasIcon)
            {
                uiFont(U"{}"_fmt(visibleIndex + 1)).drawAt(16, rect.center().movedBy(0, -4), Palette::White);
            }

            const ColorF costColor = affordable ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.25, 0.20 };
            uiFont(U"{}G"_fmt(action.costGold)).drawAt(12, rect.center().movedBy(0, 26), costColor);
        }
    }

    inline void DrawBattleWorld(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const ClickDebugState& debugState, const Font& uiFont, const Font& titleFont)
    {
        Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0.06, 0.10, 0.09 });

        Rect{ 0, 0, 1600, 70 }.draw(ColorF{ 0.03, 0.04, 0.06, 0.86 });
        titleFont(U"LoganToga3 - Quarter View Battle").draw(24, 16, Palette::White);
        uiFont(U"Gold: {}  Units: {}  Time: {:.1f}s"_fmt(world.resources.playerGold, world.units.size(), world.elapsedSec)).draw(760, 23, Palette::Gold);

        {
            const auto cameraTransform = CreateQuarterViewTransformer();

            DrawQuarterMap(world, defs, uiFont);
            DrawResourceNodes(world, defs, uiFont);
            DrawProjectiles(world, defs);
            DrawUnits(world, defs, uiFont);
        }

        RectF sidePanel{ 1240, 90, 330, 245 };
        sidePanel.draw(ColorF{ 0.02, 0.03, 0.045, 0.78 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
        uiFont(U"Controls").draw(1260, 108, Palette::White);
        uiFont(U"Left click: select player unit").draw(1260, 142, Palette::Lightgray);
        uiFont(U"Right click: move selected unit").draw(1260, 170, Palette::Lightgray);
        uiFont(U"Move workers onto gold to gather").draw(1260, 198, Palette::Lightgray);
        uiFont(U"Number keys: execute icon order").draw(1260, 240, Palette::Lightgray);
        uiFont(U"Destroy the enemy base.").draw(1260, 292, Palette::Orange);

        DrawQuarterCommandBar(world, defs, assets, uiFont);
        DrawClickDebugOverlay(debugState, uiFont);
        DrawSelectionDebugOverlay(world, defs, uiFont);
        DrawSelectedUnitPanel(world, defs, uiFont);
        DrawResultOverlay(world, uiFont, titleFont);
    }

    inline void DrawBattleWorld(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const MapEditorState& mapEditor, const ClickDebugState& debugState, const Font& uiFont, const Font& titleFont)
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

        DrawQuarterCommandBar(world, defs, assets, uiFont);
        DrawClickDebugOverlay(debugState, uiFont);
        DrawSelectionDebugOverlay(world, defs, uiFont);
        DrawSelectedUnitPanel(world, defs, uiFont);
        DrawResultOverlay(world, uiFont, titleFont);
    }
}
