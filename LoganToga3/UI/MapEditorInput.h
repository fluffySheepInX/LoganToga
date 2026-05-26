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
# include "SkillEditor.h"
# include "AiEditor.h"
# include "MapEditorDescriptionEditor.h"
# include "RectUiHelpers.h"

namespace LT3
{
    inline bool HandleUnitBuildingEditorTabBar(MapEditorState& editor)
    {
        const bool show = editor.showUnitParameterEditor || editor.showBuildingEditor || editor.showUniqueEditor;
        if (!show)
        {
            return false;
        }

        const bool layoutEditing = editor.uiLayoutEditEnabled;
        const RectF closeRect = EditorUnitBuildingCloseRect(editor);

        if (!layoutEditing && HandleRectButtonClick(closeRect))
        {
            editor.showUnitParameterEditor = false;
            editor.showBuildingEditor = false;
            editor.showUniqueEditor = false;
            editor.statusText = U"Editor closed";
            return true;
        }

        int32 selectedTab = editor.showBuildingEditor ? 1 : (editor.showUniqueEditor ? 2 : 0);
        if (HandleIntTabButtons(selectedTab, 3, [&](int32 index)
            {
                return EditorUnitBuildingTabRect(editor, index);
            }))
        {
            editor.showUnitParameterEditor = (selectedTab == 0);
            editor.showBuildingEditor = (selectedTab == 1);
            editor.showUniqueEditor = (selectedTab == 2);
            return true;
        }

        return false;
    }

    inline bool ProcessMapEditorInput(MapEditorState& editor, const BattleWorld& world, DefinitionStores& defs, UnitCatalog& catalog, const Vec2& screenMouse)
    {
        bool consumed = false;
        if (ProcessDescriptionEditorInput(editor, catalog, defs))
        {
            return true;
        }

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

        if (ProcessCommandEditorInput(editor, catalog, defs))
        {
            consumed = true;
        }

        if (ProcessSkillEditorInput(editor, defs, catalog))
        {
            consumed = true;
        }

        if (ProcessAiEditorInput(editor, defs))
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
