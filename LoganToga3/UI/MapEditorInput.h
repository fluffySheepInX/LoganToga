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

    inline bool ProcessMapEditorInput(MapEditorState& editor, BattleWorld& world, DefinitionStores& defs, UnitCatalog& catalog, const Vec2& screenMouse)
    {
        if (ProcessDescriptionEditorInput(editor, catalog, defs))
        {
            return true;
        }

        if (ProcessMapEditorUiLayoutDragInput(editor, world, defs, screenMouse))
        {
            return true;
        }

        if (ProcessMapEditorZOrderPanelInput(editor))
        {
            return true;
        }

        if (ProcessMapEditorDecalEditorInput(editor))
        {
            return true;
        }

        if (editor.showResourcePanels && ProcessResourceNodeEditorPanelInput(editor))
        {
            return true;
        }

        if (ProcessBuildingEditorInput(editor, catalog, defs))
        {
            return true;
        }

        if (ProcessAiEditorInput(editor, defs))
        {
            return true;
        }

        if (ProcessSkillEditorInput(editor, world, defs, catalog))
        {
            return true;
        }

        if (ProcessCommandEditorInput(editor, catalog, defs))
        {
            return true;
        }

        if (ProcessUnitCatalogEditorInput(editor, catalog))
        {
            return true;
        }

        if (HandleUnitBuildingEditorTabBar(editor))
        {
            return true;
        }

        if (ProcessFogPanelInput(editor))
        {
            return true;
        }

        if (ProcessPerlinNoisePanelInput(editor))
        {
            return true;
        }

        if (ProcessStarToolMenuInput(editor))
        {
            return true;
        }

        if (editor.showResourcePanels && ProcessResourcePaletteInput(editor))
        {
            return true;
        }

        if (editor.showResourcePanels && ProcessResourceNodeListInput(editor))
        {
            return true;
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
            return true;
        }

        if (ProcessMapEditorToolbarInput(editor))
        {
            return true;
        }

        if (!editor.enabled)
        {
            return false;
        }

        if (ProcessMapEditorPaletteInput(editor))
        {
            return true;
        }

        return ProcessMapEditorCanvasInput(editor, screenMouse);
    }
}
