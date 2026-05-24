#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "MapEditorToolbarModel.h"
# include "RectUiHelpers.h"

namespace LT3
{
    inline void ExecuteMapEditorToolbarAction(MapEditorState& editor, MapEditorToolbarAction action)
    {
        switch (action)
        {
        case MapEditorToolbarAction::HideBarPreview:
            editor.editorBarHiddenUntilSec = Scene::Time() + EditorBarPreviewHideSec();
            break;
        case MapEditorToolbarAction::ToggleMapEditor:
            editor.enabled = !editor.enabled;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::SaveToml:
            SaveMapEditorToml(editor);
            break;
        case MapEditorToolbarAction::DecreaseMapWidth:
            ResizeMapEditorCells(editor, editor.mapWidth - 1, editor.mapHeight);
            break;
        case MapEditorToolbarAction::IncreaseMapWidth:
            ResizeMapEditorCells(editor, editor.mapWidth + 1, editor.mapHeight);
            break;
        case MapEditorToolbarAction::DecreaseMapHeight:
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight - 1);
            break;
        case MapEditorToolbarAction::IncreaseMapHeight:
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight + 1);
            break;
        case MapEditorToolbarAction::ToggleUnitList:
            editor.showUnitList = !editor.showUnitList;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleBuildingEditor:
            editor.showBuildingEditor = !editor.showBuildingEditor;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleDebugInfo:
            editor.showDebugInfo = !editor.showDebugInfo;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleUiLayoutEdit:
            editor.uiLayoutEditEnabled = !editor.uiLayoutEditEnabled;
            editor.uiLayoutDraggingSelectedInfo = false;
            editor.uiLayoutDraggingCommandPanel = false;
            if (!editor.uiLayoutEditEnabled)
            {
                SaveBattleUiLayoutToml(editor);
            }
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleBattleGrid:
            editor.showBattleGrid = !editor.showBattleGrid;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleCommandEditor:
            editor.showCommandEditor = !editor.showCommandEditor;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleSkillEditor:
            editor.showSkillEditor = !editor.showSkillEditor;
            SaveMapEditorToml(editor, false);
            break;
        case MapEditorToolbarAction::ToggleAiEditor:
            editor.showAiEditor = !editor.showAiEditor;
            SaveMapEditorToml(editor, false);
            break;
        }
    }

    inline bool ProcessMapEditorToolbarInput(MapEditorState& editor)
    {
        if (IsEditorBarPreviewHidden(editor))
        {
            return false;
        }

        bool consumed = false;

        const MapEditorToolbarButtonSpec previewHideSpec = MapEditorToolbarPreviewHideButtonSpec();
        if (HandleRectButtonClick(MapEditorToolbarButtonRect(editor, previewHideSpec)))
        {
            ExecuteMapEditorToolbarAction(editor, previewHideSpec.action);
            consumed = true;
        }

        for (const auto& spec : MapEditorToolbarButtonSpecs())
        {
            if (!IsMapEditorToolbarButtonVisible(editor, spec))
            {
                continue;
            }

            if (HandleRectButtonClick(MapEditorToolbarButtonRect(editor, spec)))
            {
                ExecuteMapEditorToolbarAction(editor, spec.action);
                consumed = true;
            }
        }
        if (EditorToolbarRect().mouseOver())
        {
            consumed = true;
        }

        return consumed;
    }
}
