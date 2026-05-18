#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "BuildingEditorCommon.h"

namespace LT3
{
	inline void UpdateParamEditorDrag(MapEditorState& editor)
	{
		if (!editor.uiLayoutEditEnabled)
		{
			return;
		}

		if (editor.uiLayoutDraggingParamEditor)
		{
			const RectF panel = EditorUnitParameterPanelRect(editor);
			const Vec2 snapped = SnapUiLayoutPosition(Cursor::PosF() - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
			editor.uiParamEditorPos.x = Clamp(snapped.x, 0.0, 1600.0 - panel.w);
			editor.uiParamEditorPos.y = Clamp(snapped.y, 0.0, 900.0 - panel.h);
			if (MouseL.up())
			{
				editor.uiLayoutDraggingParamEditor = false;
				SaveBattleUiLayoutToml(editor, false);
			}
		}
		else if (UiLayoutTopAnchorToggleRect(EditorUnitParameterDragHandleRect(editor)).leftClicked())
		{
			editor.uiParamEditorTopAnchor = !editor.uiParamEditorTopAnchor;
			SaveBattleUiLayoutToml(editor, false);
		}
		else if (EditorUnitParameterDragHandleRect(editor).leftClicked())
		{
			editor.uiLayoutDraggingParamEditor = true;
			editor.uiLayoutDragOffset = Cursor::PosF() - editor.uiParamEditorPos;
		}
	}

	inline void UpdateBuildingEditorDrag(MapEditorState& editor)
	{
		if (!editor.uiLayoutEditEnabled)
		{
			return;
		}

		if (editor.uiLayoutDraggingBuildingEditor)
		{
			const RectF panel = BuildingEditorPanelWithPosRect(editor);
			const Vec2 snapped = SnapUiLayoutPosition(Cursor::PosF() - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
			editor.uiBuildingEditorPos.x = Clamp(snapped.x, 0.0, 1600.0 - panel.w);
			editor.uiBuildingEditorPos.y = Clamp(snapped.y, 0.0, 900.0 - panel.h);
			if (MouseL.up())
			{
				editor.uiLayoutDraggingBuildingEditor = false;
				SaveBattleUiLayoutToml(editor, false);
			}
		}
		else if (UiLayoutTopAnchorToggleRect(EditorBuildingEditorDragHandleRect(editor)).leftClicked())
		{
			editor.uiBuildingEditorTopAnchor = !editor.uiBuildingEditorTopAnchor;
			SaveBattleUiLayoutToml(editor, false);
		}
		else if (EditorBuildingEditorDragHandleRect(editor).leftClicked())
		{
			editor.uiLayoutDraggingBuildingEditor = true;
			editor.uiLayoutDragOffset = Cursor::PosF() - editor.uiBuildingEditorPos;
		}
	}
}
