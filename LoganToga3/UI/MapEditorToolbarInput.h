#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
    inline bool ProcessMapEditorToolbarInput(MapEditorState& editor)
    {
        bool consumed = false;

        if (EditorToolbarButtonRect(editor, 0).leftClicked())
        {
            editor.enabled = !editor.enabled;
            editor.statusText = editor.enabled ? U"Map editor ON" : U"Map editor OFF";
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
            editor.statusText = editor.showUnitList ? U"Unit List ON" : U"Unit List OFF";
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 7).leftClicked())
        {
            editor.showBuildingEditor = !editor.showBuildingEditor;
            editor.statusText = editor.showBuildingEditor ? U"BuildingEditor ON" : U"BuildingEditor OFF";
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 8).leftClicked())
        {
            editor.showDebugInfo = !editor.showDebugInfo;
            editor.statusText = editor.showDebugInfo ? U"Debug Info ON" : U"Debug Info OFF";
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
            else
            {
                editor.statusText = U"UI Layout Edit ON";
            }
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
