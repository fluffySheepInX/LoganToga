#pragma once
# include <algorithm>
# include "BattleMapRenderer.h"
# include "BattleNineSlice.h"
# include "BattleSelectionRenderer.h"
# include "BattleUiPanels.h"
# include "BattleUnitRenderer.h"
# include "../App/AppRuntimeState.h"
# include "MapEditor.h"

namespace LT3
{
    inline void DrawFogOfWarExtendedOverlay(const BattleWorld& world, const Array<bool>& visible, const ColorF& fogColor)
    {
        DrawFogOfWarCells(world, visible, fogColor);

        const int32 width = Max(1, world.mapWidth);
        const int32 height = Max(1, world.mapHeight);
        const double verticalReach = 300.0;

        for (int32 diagonal = 0; diagonal < (width + height - 1); ++diagonal)
        {
            const int32 xBegin = Max(0, diagonal - (height - 1));
            const int32 xEnd = Min(width - 1, diagonal);
            for (int32 x = xBegin; x <= xEnd; ++x)
            {
                const int32 y = diagonal - x;
                const size_t index = static_cast<size_t>(y * width + x);
                if (index >= visible.size() || visible[index])
                {
                    continue;
                }
                if ((x != 0) && (y != 0))
                {
                    continue;
                }

                const Quad tile = ToQuarterTile(QuarterBattleCellCenter(x, y));
                const Vec2 top = tile.p0;
                const Vec2 right = tile.p1;
                const Vec2 left = tile.p3;
                if (x == 0)
                {
                    Quad{ left, top, top.movedBy(0.0, -verticalReach), left.movedBy(0.0, -verticalReach) }.draw(fogColor);
                }
                if (y == 0)
                {
                    Quad{ top, right, right.movedBy(0.0, -verticalReach), top.movedBy(0.0, -verticalReach) }.draw(fogColor);
                }
            }
        }
    }

    enum class BattleWorldDepthRenderKind
    {
        Unit,
        TallDecal,
    };

    struct BattleWorldDepthRenderEntry
    {
        BattleWorldDepthRenderKind kind = BattleWorldDepthRenderKind::Unit;
        double depth = 0.0;
        int32 insertionOrder = 0;
        UnitId unit = 0;
        MapEditorObjectRenderEntry tallDecal;
    };

    inline void DrawUnitsAndTallMapEditorObjects(const BattleWorld& world, const DefinitionStores& defs, const Font& uiFont, const BattleRenderAssets* assets, const MapEditorState& mapEditor, const Array<bool>* visibleMask = nullptr, int32 maskWidth = 0, int32 maskHeight = 0, bool showEnemyMoveMarkers = false)
    {
        Array<BattleWorldDepthRenderEntry> renderList;
        int32 insertionOrder = 0;

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsUnitVisibleForRender(world, unit, visibleMask, maskWidth, maskHeight))
            {
                continue;
            }

            const Vec2 pos = ToQuarterScreen(world.units.position[unit]);
            BattleWorldDepthRenderEntry entry;
            entry.kind = BattleWorldDepthRenderKind::Unit;
            entry.depth = pos.y;
            entry.insertionOrder = insertionOrder++;
            entry.unit = unit;
            renderList << entry;
        }

        Array<MapEditorObjectRenderEntry> tallDecals;
        CollectMapEditorObjectRenderEntries(mapEditor, MapEditorObjectLayerPass::Tall, tallDecals);
        SortMapEditorObjectRenderEntries(tallDecals);
        for (const MapEditorObjectRenderEntry& decal : tallDecals)
        {
            BattleWorldDepthRenderEntry entry;
            entry.kind = BattleWorldDepthRenderKind::TallDecal;
            entry.depth = decal.depth;
            entry.insertionOrder = insertionOrder++;
            entry.tallDecal = decal;
            renderList << entry;
        }

        std::stable_sort(renderList.begin(), renderList.end(), [](const BattleWorldDepthRenderEntry& a, const BattleWorldDepthRenderEntry& b)
        {
            if (a.depth != b.depth)
            {
                return a.depth < b.depth;
            }
            return a.insertionOrder < b.insertionOrder;
        });

        for (const BattleWorldDepthRenderEntry& entry : renderList)
        {
            if (entry.kind == BattleWorldDepthRenderKind::Unit)
            {
                DrawUnitRenderContent(world, defs, entry.unit, uiFont, assets, showEnemyMoveMarkers);
            }
            else
            {
                DrawMapEditorObjectRenderEntry(mapEditor, entry.tallDecal);
            }
        }
    }

    inline void DrawBattleWorld(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const ResourceFlagRuntimeState& resourceFlags, const MapEditorState& mapEditor, const ClickDebugState& debugState, bool showDebugInfo, const Font& uiFont, const Font& titleFont)
    {
        Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0.06, 0.10, 0.09 });
        const Array<bool> fogVisibleMask = BuildFogVisibleCellMask(world, defs);
        const Array<bool>* visibleMask = mapEditor.fogEnabled ? &fogVisibleMask : nullptr;

        {
            const auto cameraTransform = CreateQuarterViewTransformer();
            const BattleDebugCursorState debugCursor = MakeBattleDebugCursorState(debugState.currentScreen);

            DrawMapEditorTerrainLayer(mapEditor, mapEditor.enabled || mapEditor.showBattleGrid);
            DrawResourceNodes(world, defs, assets, uiFont, nullptr);
            DrawProjectiles(world, defs, &assets);
            DrawPlacedBattleObjects(world, uiFont, assets);
            DrawMapEditorObjectLayer(mapEditor, MapEditorObjectLayerPass::Ground);
            DrawUnitsAndTallMapEditorObjects(world, defs, uiFont, &assets, mapEditor, visibleMask, world.mapWidth, world.mapHeight, mapEditor.showEnemyMoveMarkers);
            DrawSelectedUnitSkillRangeOutline(world, defs);
            DrawMapEditorObjectLayer(mapEditor, MapEditorObjectLayerPass::Overlay);
            if (mapEditor.fogEnabled)
            {
                const ColorF fogColor{ mapEditor.fogColor.r, mapEditor.fogColor.g, mapEditor.fogColor.b, mapEditor.fogOpacity };
                DrawFogOfWarExtendedOverlay(world, fogVisibleMask, fogColor);
                if (mapEditor.fogPreviewVision)
                {
                    const int32 width = Max(1, world.mapWidth);
                    const int32 height = Max(1, world.mapHeight);
                    for (int32 diagonal = 0; diagonal < (width + height - 1); ++diagonal)
                    {
                        const int32 xBegin = Max(0, diagonal - (height - 1));
                        const int32 xEnd = Min(width - 1, diagonal);
                        for (int32 x = xBegin; x <= xEnd; ++x)
                        {
                            const int32 y = diagonal - x;
                            const size_t index = static_cast<size_t>(y * width + x);
                            if (index < fogVisibleMask.size() && fogVisibleMask[index])
                            {
                                ToQuarterTile(QuarterBattleCellCenter(x, y)).drawFrame(1.0, ColorF{ 0.75, 0.95, 0.88, 0.10 });
                            }
                        }
                    }
                }
            }
            DrawUnitHealthBarsOverlay(world, defs, visibleMask, world.mapWidth, world.mapHeight);
            DrawResourceNodeOverlays(world, defs, assets, uiFont, &resourceFlags);
            if (showDebugInfo)
            {
                DrawBuildingUnitClickDebugOverlay(world, defs, debugCursor);
                DrawResourceNodeClickDebugOverlay(world, debugCursor);
            }
        }

        DrawQuarterCommandBar(world, defs, mapEditor, assets, uiFont);
        DrawResourcePanel(world, defs, assets, uiFont, mapEditor);
        DrawBattleSkillPanel(world, defs, assets, uiFont);
        DrawAreaSelectionFrame(world, assets);
        DrawFormationPlacementPreview(world, defs);
        DrawBuildActionPlacementPreview(world, defs, assets, uiFont);
        if (showDebugInfo)
        {
            DrawClickDebugOverlay(debugState, uiFont);
            DrawSelectionDebugOverlay(world, defs, assets, uiFont);
        }
        DrawBattleTimerOverlay(world, uiFont);
        DrawSelectedUnitPanel(world, defs, mapEditor, uiFont, showDebugInfo);
        DrawResultOverlay(world, uiFont, titleFont);
    }
}
