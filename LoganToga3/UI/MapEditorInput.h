#pragma once
# include <Siv3D.hpp>
# include "MapEditorCanvasInput.h"
# include "MapEditorMapData.h"
# include "MapEditorPerlinNoise.h"
# include "MapEditorResourceInput.h"
# include "MapEditorToolbarInput.h"
# include "MapEditorUiLayoutInput.h"
# include "MapEditorUnitCatalogInput.h"
# include "BuildingEditor.h"

namespace LT3
{
    inline bool HandleUnitBuildingEditorTabBar(MapEditorState& editor)
    {
        const bool show = editor.showUnitParameterEditor || editor.showBuildingEditor;
        if (!show)
        {
            return false;
        }

        const bool layoutEditing = editor.uiLayoutEditEnabled;
        const RectF tabParam = EditorUnitBuildingTabRect(editor, 0);
        const RectF tabBuilding = EditorUnitBuildingTabRect(editor, 1);
        const RectF closeRect = EditorUnitBuildingCloseRect(editor);

        // 共通 × ボタン
        if (!layoutEditing && closeRect.leftClicked())
        {
            editor.showUnitParameterEditor = false;
            editor.showBuildingEditor = false;
            editor.statusText = U"Editor closed";
            return true;
        }

        // Unit Param タブ
        if (tabParam.leftClicked())
        {
            editor.showUnitParameterEditor = true;
            editor.showBuildingEditor = false;
            return true;
        }

        // Building Edit タブ
        if (tabBuilding.leftClicked())
        {
            editor.showBuildingEditor = true;
            editor.showUnitParameterEditor = false;
            return true;
        }

        return false;
    }

    inline bool ProcessMapEditorInput(MapEditorState& editor, const BattleWorld& world, const DefinitionStores& defs, UnitCatalog& catalog, const Vec2& screenMouse)
    {
        bool consumed = false;
        if (HandleUnitBuildingEditorTabBar(editor))
        {
            consumed = true;
        }

        if (ProcessMapEditorToolbarInput(editor))
        {
            consumed = true;
        }

        if (ProcessStarToolMenuInput(editor))
        {
            consumed = true;
        }

        if (ProcessPerlinNoisePanelInput(editor))
        {
            consumed = true;
        }

        if (ProcessFogPanelInput(editor))
        {
            consumed = true;
        }

        if (ProcessUnitCatalogEditorInput(editor, catalog))
        {
            consumed = true;
        }

        if (ProcessBuildingEditorInput(editor, catalog, defs))
        {
            consumed = true;
        }

        if (editor.showResourcePanels && ProcessResourcePaletteInput(editor))
        {
            consumed = true;
        }

        if (editor.showResourcePanels && ProcessResourceNodeListInput(editor))
        {
            consumed = true;
        }

        if (editor.showResourcePanels && ProcessResourceNodeEditorPanelInput(editor))
        {
            consumed = true;
        }

        if (ProcessMapEditorDecalEditorInput(editor))
        {
            consumed = true;
        }

        if (ProcessMapEditorZOrderPanelInput(editor))
        {
            consumed = true;
        }

        if (editor.enabled && editor.showResourcePanels && editor.resourcePlacementDragKind && MouseL.up())
        {
            if (const Optional<Point> releaseCell = PickMapEditorCell(editor, screenMouse))
            {
                CommitDraggedResourcePlacement(editor, *releaseCell);
            }
            else
            {
                editor.resourcePlacementDragKind.reset();
            }

            return true;
        }

        if (ProcessMapEditorUiLayoutInput(editor, world, defs, screenMouse))
        {
            consumed = true;
        }

        if (!editor.enabled)
        {
            return consumed;
        }

        if (ProcessMapEditorPaletteInput(editor))
        {
            consumed = true;
        }

        if (consumed)
        {
            return true;
        }

        return ProcessMapEditorCanvasInput(editor, screenMouse);
    }
}
