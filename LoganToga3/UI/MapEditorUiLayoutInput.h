#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleQueries.h"
# include "MapEditorMapData.h"

namespace LT3
{
    inline bool ProcessMapEditorUiLayoutInput(MapEditorState& editor, const BattleWorld& world, const DefinitionStores& defs, const Vec2& screenMouse)
    {
        if (!editor.uiLayoutEditEnabled)
        {
            return false;
        }

        bool consumed = false;
        if (EditorUiLayoutGridDecrementRect().leftClicked())
        {
            editor.uiLayoutGridSize = Clamp(editor.uiLayoutGridSize - 8, 8, 160);
            SaveBattleUiLayoutToml(editor, false);
            editor.statusText = U"UI Grid: {}"_fmt(editor.uiLayoutGridSize);
            consumed = true;
        }
        if (EditorUiLayoutGridIncrementRect().leftClicked())
        {
            editor.uiLayoutGridSize = Clamp(editor.uiLayoutGridSize + 8, 8, 160);
            SaveBattleUiLayoutToml(editor, false);
            editor.statusText = U"UI Grid: {}"_fmt(editor.uiLayoutGridSize);
            consumed = true;
        }

        const bool showDetail = KeyControl.pressed();
        const Array<BuildActionUiState> visibleActions = CollectVisibleBuildActionsForSelectedUnit(world, defs);
        const int32 commandRows = Max(1, (static_cast<int32>(visibleActions.size()) + 2) / 3);
        const RectF infoRect = showDetail ? BattleInfoPanelDetailRect(editor) : BattleInfoPanelCompactRect(editor);
        const RectF commandRect = BattleCommandPanelRect(editor, commandRows);

        const RectF infoHandle = UiLayoutDragHandleRect(infoRect);
        const RectF commandHandle = UiLayoutDragHandleRect(commandRect);

        if (!editor.uiLayoutDraggingCommandPanel && MouseL.down() && infoHandle.mouseOver())
        {
            editor.uiLayoutDraggingSelectedInfo = true;
            editor.uiLayoutDragOffset = screenMouse - editor.uiSelectedInfoAnchor;
            consumed = true;
        }
        if (!editor.uiLayoutDraggingSelectedInfo && MouseL.down() && commandHandle.mouseOver())
        {
            editor.uiLayoutDraggingCommandPanel = true;
            editor.uiLayoutDragOffset = screenMouse - editor.uiCommandPanelPos;
            consumed = true;
        }

        if (editor.uiLayoutDraggingSelectedInfo && MouseL.pressed())
        {
            const Vec2 snapped = SnapUiLayoutPosition(screenMouse - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
            editor.uiSelectedInfoAnchor.x = Clamp(snapped.x, 0.0, 1600.0 - infoRect.w);
            editor.uiSelectedInfoAnchor.y = Clamp(snapped.y, infoRect.h, 900.0);
            consumed = true;
        }
        if (editor.uiLayoutDraggingCommandPanel && MouseL.pressed())
        {
            const Vec2 snapped = SnapUiLayoutPosition(screenMouse - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
            editor.uiCommandPanelPos.x = Clamp(snapped.x, 0.0, 1600.0 - commandRect.w);
            editor.uiCommandPanelPos.y = Clamp(snapped.y, 70.0, Max(70.0, BattleUiBottomSafeY() - commandRect.h));
            consumed = true;
        }

        if (MouseL.up())
        {
            const bool wasDragging = editor.uiLayoutDraggingSelectedInfo || editor.uiLayoutDraggingCommandPanel;
            editor.uiLayoutDraggingSelectedInfo = false;
            editor.uiLayoutDraggingCommandPanel = false;
            if (wasDragging)
            {
                SaveBattleUiLayoutToml(editor, false);
                editor.statusText = U"UI Layout updated";
            }
        }

        if (infoRect.mouseOver() || commandRect.mouseOver())
        {
            consumed = true;
        }
        if (EditorUiLayoutGridPanelRect().mouseOver())
        {
            consumed = true;
        }

        return consumed;
    }
}
