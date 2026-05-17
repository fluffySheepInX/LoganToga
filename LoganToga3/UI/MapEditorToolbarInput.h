#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
    inline bool ProcessMapEditorToolbarInput(MapEditorState& editor)
    {
        if (IsEditorBarPreviewHidden(editor))
        {
            return false;
        }

        bool consumed = false;

        if (EditorToolbarPreviewHideButtonRect().leftClicked())
        {
            editor.editorBarHiddenUntilSec = Scene::Time() + EditorBarPreviewHideSec();
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 0).leftClicked())
        {
            editor.enabled = !editor.enabled;
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 1).leftClicked())
        {
            SaveMapEditorToml(editor);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 2).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth - 1, editor.mapHeight);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 3).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth + 1, editor.mapHeight);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 4).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight - 1);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 5).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight + 1);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 6).leftClicked())
        {
            editor.showUnitList = !editor.showUnitList;
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 7).leftClicked())
        {
            editor.showBuildingEditor = !editor.showBuildingEditor;
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 8).leftClicked())
        {
            editor.showDebugInfo = !editor.showDebugInfo;
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 9).leftClicked())
        {
            editor.uiLayoutEditEnabled = !editor.uiLayoutEditEnabled;
            editor.uiLayoutDraggingSelectedInfo = false;
            editor.uiLayoutDraggingCommandPanel = false;
            if (!editor.uiLayoutEditEnabled)
            {
                SaveBattleUiLayoutToml(editor);
            }
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 10).leftClicked())
        {
            editor.showBattleGrid = !editor.showBattleGrid;
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarRect().mouseOver())
        {
            consumed = true;
        }

        return consumed;
    }
}
