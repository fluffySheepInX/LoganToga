#pragma once
# include <Siv3D.hpp>
# include "MapEditorCanvasInput.h"
# include "MapEditorMapData.h"
# include "MapEditorResourceInput.h"
# include "MapEditorToolbarInput.h"
# include "MapEditorUiLayoutInput.h"
# include "MapEditorUnitCatalogInput.h"
# include "BuildingEditor.h"

namespace LT3
{
    inline bool ProcessMapEditorInput(MapEditorState& editor, const BattleWorld& world, const DefinitionStores& defs, UnitCatalog& catalog, const Vec2& screenMouse)
    {
        bool consumed = false;
        if (ProcessMapEditorToolbarInput(editor))
        {
            consumed = true;
        }

        if (ProcessUnitCatalogEditorInput(editor, catalog))
        {
            consumed = true;
        }

        if (ProcessBuildingEditorInput(editor, catalog))
        {
            consumed = true;
        }

        if (ProcessResourcePaletteInput(editor))
        {
            consumed = true;
        }

        if (ProcessResourceNodeListInput(editor))
        {
            consumed = true;
        }

        if (ProcessResourceNodeEditorPanelInput(editor))
        {
            consumed = true;
        }

        if (editor.enabled && editor.resourcePlacementDragKind && MouseL.up())
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

        if (IsValidSelectedResourceNodeIndex(editor))
        {
            if (KeyUp.down())
            {
                NudgeSelectedResourceNode(editor, 0, -1);
                consumed = true;
            }
            if (KeyDown.down())
            {
                NudgeSelectedResourceNode(editor, 0, 1);
                consumed = true;
            }
            if (KeyLeft.down())
            {
                NudgeSelectedResourceNode(editor, -1, 0);
                consumed = true;
            }
            if (KeyRight.down())
            {
                NudgeSelectedResourceNode(editor, 1, 0);
                consumed = true;
            }
            if (KeyD.down() && KeyControl.pressed())
            {
                DuplicateSelectedResourceNode(editor);
                consumed = true;
            }
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
