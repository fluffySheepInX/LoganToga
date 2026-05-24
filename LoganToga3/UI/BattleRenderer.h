#pragma once
# include "BattleMapRenderer.h"
# include "BattleNineSlice.h"
# include "BattleSelectionRenderer.h"
# include "BattleUiPanels.h"
# include "BattleUnitRenderer.h"
# include "MapEditor.h"

namespace LT3
{
    inline void DrawBattleWorld(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const MapEditorState& mapEditor, const ClickDebugState& debugState, bool showDebugInfo, const Font& uiFont, const Font& titleFont)
    {
        Rect{ 0, 0, 1600, 900 }.draw(ColorF{ 0.06, 0.10, 0.09 });

        {
            const auto cameraTransform = CreateQuarterViewTransformer();
            const Array<bool> fogVisibleMask = BuildFogVisibleCellMask(world, defs);
            const Array<bool>* visibleMask = mapEditor.fogEnabled ? &fogVisibleMask : nullptr;

            DrawMapEditorTerrainLayer(mapEditor, mapEditor.enabled || mapEditor.showBattleGrid);
            DrawResourceNodes(world, defs, assets, uiFont);
            DrawProjectiles(world, defs, &assets);
            DrawPlacedBattleObjects(world, uiFont, assets);
            DrawMapEditorObjectLayer(mapEditor, MapEditorObjectLayerPass::Back);
            DrawUnits(world, defs, uiFont, &assets, visibleMask, world.mapWidth, world.mapHeight, mapEditor.showEnemyMoveMarkers);
        DrawSelectedUnitSkillRangeOutline(world, defs);
            DrawMapEditorObjectLayer(mapEditor, MapEditorObjectLayerPass::Front);
            DrawFogOfWarOverlay(world, defs, mapEditor);
        }

        DrawQuarterCommandBar(world, defs, mapEditor, assets, uiFont);
        DrawResourcePanel(world, defs, assets, uiFont, mapEditor);
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
