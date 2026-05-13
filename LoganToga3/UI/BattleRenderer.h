#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Systems/SelectionSystem.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleDebugOverlay.h"
# include "BattleUnitRenderer.h"
# include "BattleResourceRenderer.h"
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

        const Array<QueuedBuildAction>& buildQueue = GetQueuedBuildActionEntries(world, selected);
        if (!buildQueue.isEmpty() && buildQueue.front().actionId < defs.buildActions.size())
        {
            const BuildActionDef& action = defs.buildActions[buildQueue.front().actionId];
            const double rate = Clamp(world.buildQueues.progressSec[selected] / Max(0.001, action.buildTimeSec), 0.0, 1.0);
            uiFont(U"Build: {}  Queue: {}"_fmt(action.name, buildQueue.size())).draw(info.x + 18.0, info.y + 132.0, Palette::Gold);
            RectF{ info.x + 240.0, info.y + 138.0, 180.0, 12.0 }.draw(ColorF{ 0, 0, 0, 0.45 });
            RectF{ info.x + 240.0, info.y + 138.0, 180.0 * rate, 12.0 }.draw(Palette::Gold);

            const size_t previewCount = Min<size_t>(3, buildQueue.size());
            for (size_t i = 0; i < previewCount; ++i)
            {
                const BuildActionDefId queuedActionId = buildQueue[i].actionId;
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

    inline void DrawBuildActionPlacementPreview(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        if (!world.selection.actionPlacementActive || world.selection.actionId >= defs.buildActions.size())
        {
            return;
        }

        const BuildActionDef& action = defs.buildActions[world.selection.actionId];
        if (action.placementMode == BuildPlacementMode::Line && action.useRightDragPlacement)
        {
            Array<Vec2> targets = world.selection.actionLineTargets;
            if (targets.isEmpty())
            {
                targets << world.selection.actionTargetWorld;
            }

            for (const Vec2& target : targets)
            {
                const Vec2 targetScreen = ToQuarterViewportScreen(target);
                RectF{ Arg::center = targetScreen, 28.0, 28.0 }.draw(ColorF{ 0.0, 0.75, 1.0, 0.14 }).drawFrame(2.0, ColorF{ 0.0, 0.85, 1.0, 0.90 });
            }

            if (targets.size() >= 2)
            {
                Line{ ToQuarterViewportScreen(targets.front()), ToQuarterViewportScreen(targets.back()) }.draw(5.0, ColorF{ 0.0, 0.85, 1.0, 0.45 });
            }

            const Vec2 labelScreen = ToQuarterViewportScreen(targets.back()).movedBy(0, 34);
            uiFont(U"右ドラッグ範囲: {} x{}"_fmt(action.name, targets.size())).drawAt(14, labelScreen, Palette::Gold);
            return;
        }

        const Vec2 screen = ToQuarterViewportScreen(world.selection.actionTargetWorld);
        Circle{ screen, 16.0 }.draw(ColorF{ 0.0, 0.75, 1.0, 0.12 }).drawFrame(3.0, ColorF{ 0.0, 0.85, 1.0, 0.90 });
        if (!DrawBuildActionIcon(action, defs, assets, screen.movedBy(0, -28), 42.0))
        {
            uiFont(action.name).drawAt(12, screen.movedBy(0, -30), Palette::White);
        }
        uiFont(U"配置: {}"_fmt(action.name)).drawAt(14, screen.movedBy(0, 28), Palette::Gold);
    }

    inline void DrawPlacedBattleObjects(const BattleWorld& world, const Font& uiFont, const BattleRenderAssets& assets)
    {
        for (size_t i = 0; i < world.placedObjects.position.size(); ++i)
        {
            const Vec2 screen = ToQuarterScreen(world.placedObjects.position[i]);
            const String& iconName = world.placedObjects.icon[i];
            const FilePath iconPath = iconName.isEmpty() ? FilePath{} : ResolveBuildIconPath(iconName);
            if (!iconPath.isEmpty() && FileSystem::Exists(iconPath))
            {
                if (!assets.iconTextureCache.contains(iconPath))
                {
                    assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
                }
                assets.iconTextureCache.at(iconPath).resized(54.0, 54.0).drawAt(screen.movedBy(0, -24));
            }
            else
            {
                RectF{ Arg::center = screen.movedBy(0, -18), 42.0, 42.0 }.draw(ColorF{ 0.26, 0.26, 0.30, 0.88 }).drawFrame(2.0, ColorF{ 1, 1, 1, 0.18 });
                uiFont(world.placedObjects.tag[i].isEmpty() ? U"obj" : world.placedObjects.tag[i]).drawAt(10, screen.movedBy(0, -18), Palette::White);
            }
        }
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
        const Array<QueuedBuildAction>& queue = GetQueuedBuildActionEntries(world, selected);
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
            const BuildActionDefId actionId = queue[i].actionId;
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

        const BuildActionDefId currentActionId = queue.front().actionId;
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
            const RectF rect = BattleCommandIconRect(mapEditor, visibleIndex, rows);
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
            DrawResourceNodes(world, defs, assets, uiFont);
            DrawProjectiles(world, defs);
            DrawPlacedBattleObjects(world, uiFont, assets);
            DrawUnits(world, defs, uiFont, &assets);

            DrawMapEditorObjectLayer(mapEditor);
        }

        DrawQuarterCommandBar(world, defs, mapEditor, assets, uiFont);
        DrawResourcePanel(world, defs, assets, uiFont);
        DrawAreaSelectionFrame(world, assets);
        DrawFormationPlacementPreview(world, defs);
        DrawBuildActionPlacementPreview(world, defs, assets, uiFont);
        if (showDebugInfo)
        {
            DrawClickDebugOverlay(debugState, uiFont);
            DrawSelectionDebugOverlay(world, defs, assets, uiFont);
        }
        DrawSelectedUnitPanel(world, defs, mapEditor, uiFont, showDebugInfo);
        DrawResultOverlay(world, uiFont, titleFont);
    }
}
