#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleQueries.h"
# include "../Systems/SelectionSystem.h"
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
        const bool showMultiUnitList = (GetSelectedUnits(world).size() >= 2);
        const Array<BuildActionUiState> visibleActions = CollectVisibleBuildActionsForSelectedUnit(world, defs);
        const int32 commandRows = Max(1, (static_cast<int32>(visibleActions.size()) + 2) / 3);
        const RectF infoRect = showMultiUnitList
            ? BattleInfoPanelMultiRect(editor, GetSelectedUnits(world).size())
            : (showDetail ? BattleInfoPanelDetailRect(editor) : BattleInfoPanelCompactRect(editor));
        const RectF commandRect = BattleCommandPanelRect(editor, commandRows);
        const RectF resourceRect = BattleResourcePanelRect(editor, defs.resources.size());

        const RectF infoHandle = UiLayoutDragHandleRect(infoRect);
        const RectF commandHandle = UiLayoutDragHandleRect(commandRect);
        const RectF resourceHandle = UiLayoutDragHandleRect(resourceRect);
        const RectF infoTopAnchorRect = UiLayoutTopAnchorToggleRect(infoHandle);
        const RectF commandTopAnchorRect = UiLayoutTopAnchorToggleRect(commandHandle);
        const RectF resourceTopAnchorRect = UiLayoutTopAnchorToggleRect(resourceHandle);

        if (infoTopAnchorRect.leftClicked())
        {
            editor.uiSelectedInfoTopAnchor = !editor.uiSelectedInfoTopAnchor;
            SaveBattleUiLayoutToml(editor, false);
            consumed = true;
        }
        if (commandTopAnchorRect.leftClicked())
        {
            editor.uiCommandPanelTopAnchor = !editor.uiCommandPanelTopAnchor;
            SaveBattleUiLayoutToml(editor, false);
            consumed = true;
        }
        if (resourceTopAnchorRect.leftClicked())
        {
            editor.uiResourcePanelTopAnchor = !editor.uiResourcePanelTopAnchor;
            SaveBattleUiLayoutToml(editor, false);
            consumed = true;
        }

        if (!editor.uiLayoutDraggingCommandPanel && !editor.uiLayoutDraggingResourcePanel && MouseL.down() && infoHandle.mouseOver())
        {
            editor.uiLayoutDraggingSelectedInfo = true;
            editor.uiLayoutDragOffset = screenMouse - editor.uiSelectedInfoAnchor;
            consumed = true;
        }
        if (!editor.uiLayoutDraggingSelectedInfo && !editor.uiLayoutDraggingResourcePanel && MouseL.down() && commandHandle.mouseOver())
        {
            editor.uiLayoutDraggingCommandPanel = true;
            editor.uiLayoutDragOffset = screenMouse - editor.uiCommandPanelPos;
            consumed = true;
        }
        if (!editor.uiLayoutDraggingSelectedInfo && !editor.uiLayoutDraggingCommandPanel && MouseL.down() && resourceHandle.mouseOver())
        {
            editor.uiLayoutDraggingResourcePanel = true;
            editor.uiLayoutDragOffset = screenMouse - editor.uiResourcePanelPos;
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
        if (editor.uiLayoutDraggingResourcePanel && MouseL.pressed())
        {
            const Vec2 snapped = SnapUiLayoutPosition(screenMouse - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
            editor.uiResourcePanelPos.x = Clamp(snapped.x, 0.0, 1600.0 - resourceRect.w);
            editor.uiResourcePanelPos.y = Clamp(snapped.y, 0.0, 900.0 - resourceRect.h);
            consumed = true;
        }

        if (MouseL.up())
        {
            const bool wasDragging = editor.uiLayoutDraggingSelectedInfo || editor.uiLayoutDraggingCommandPanel || editor.uiLayoutDraggingResourcePanel;
            editor.uiLayoutDraggingSelectedInfo = false;
            editor.uiLayoutDraggingCommandPanel = false;
            editor.uiLayoutDraggingResourcePanel = false;
            if (wasDragging)
            {
                SaveBattleUiLayoutToml(editor, false);
                editor.statusText = U"UI Layout updated";
            }
        }

        if (infoRect.mouseOver() || commandRect.mouseOver() || resourceRect.mouseOver())
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
