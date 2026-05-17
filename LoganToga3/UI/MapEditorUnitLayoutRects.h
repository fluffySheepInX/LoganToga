#pragma once
# include "MapEditorToolbarLayoutRects.h"

namespace LT3
{
	inline RectF EditorUnitBuildingTabBarRect()
	{
		return RectF{ 600, 60, 420, 32 };
	}

	inline RectF EditorUnitBuildingTabBarRect(const MapEditorState& editor)
	{
		const Vec2 panelPos = editor.showBuildingEditor ? editor.uiBuildingEditorPos : editor.uiParamEditorPos;
		const bool topAnchor = editor.showBuildingEditor ? editor.uiBuildingEditorTopAnchor : editor.uiParamEditorTopAnchor;
		return RectF{ panelPos.x, panelPos.y - 32.0 + EditorBarTopAnchorOffset(editor, topAnchor), 420.0, 32.0 };
	}

	inline RectF EditorUnitBuildingTabRect(int32 index)
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		const double tabW = 140.0;
		return RectF{ bar.x + 8.0 + index * (tabW + 4.0), bar.y + 4.0, tabW, bar.h - 8.0 };
	}

	inline RectF EditorUnitBuildingTabRect(const MapEditorState& editor, int32 index)
	{
		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		const double tabW = 140.0;
		return RectF{ bar.x + 8.0 + index * (tabW + 4.0), bar.y + 4.0, tabW, bar.h - 8.0 };
	}

	inline RectF EditorUnitBuildingCloseRect()
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		return RectF{ bar.x + bar.w - 36.0, bar.y + 4.0, 28.0, bar.h - 8.0 };
	}

	inline RectF EditorUnitBuildingCloseRect(const MapEditorState& editor)
	{
		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		return RectF{ bar.x + bar.w - 36.0, bar.y + 4.0, 28.0, bar.h - 8.0 };
	}

	inline RectF EditorUnitParameterPanelRect()
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		return RectF{ bar.x, bar.y + bar.h, bar.w, 520.0 };
	}

	inline RectF EditorUnitParameterPanelRect(const MapEditorState& editor)
	{
		return RectF{ editor.uiParamEditorPos.x, editor.uiParamEditorPos.y + EditorBarTopAnchorOffset(editor, editor.uiParamEditorTopAnchor), 420.0, 520.0 };
	}

	inline RectF EditorUnitParamInnerTabRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 16.0 + index * 96.0, panel.y + 12.0, 88.0, 26.0 };
	}

	inline RectF EditorUnitParamHeaderDividerRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 86.0, panel.w - 28.0, 1.0 };
	}

	inline RectF EditorUnitParamListHeaderRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 92.0, panel.w - 28.0, 34.0 };
	}

	inline RectF EditorUnitParamListViewportRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 128.0, panel.w - 28.0, panel.h - 146.0 };
	}

	inline RectF EditorUnitParamRowRect(const RectF& viewport, int32 index)
	{
		return RectF{ viewport.x, viewport.y + index * 56.0, viewport.w, 50.0 };
	}

	inline RectF EditorUnitParamRowNameRect(const RectF& row)
	{
		return RectF{ row.x + 6.0, row.y + 7.0, 104.0, row.h - 14.0 };
	}

	inline RectF EditorUnitParamRowValueRect(const RectF& row)
	{
		return RectF{ row.x + 118.0, row.y + 7.0, 92.0, row.h - 14.0 };
	}

	inline RectF EditorUnitParamRowButtonRect(const RectF& row, int32 buttonIndex)
	{
		return RectF{ row.x + 218.0 + buttonIndex * 36.0, row.y + 7.0, 32.0, row.h - 14.0 };
	}

	inline RectF EditorUnitScaleDecrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleIncrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleResetRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 132.0, panel.y + 176.0, 96.0, 34.0 };
	}

	inline RectF EditorUnitScaleResetRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 132.0, panel.y + 176.0, 96.0, 34.0 };
	}

	inline RectF EditorUnitMoveDecrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveIncrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveUseSpeedRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 116.0, panel.y + 276.0, 128.0, 28.0 };
	}

	inline RectF EditorUnitMoveUseSpeedRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 116.0, panel.y + 276.0, 128.0, 28.0 };
	}

	inline RectF EditorUnitVisionDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 338.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitVisionIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 338.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitVisionResetRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 132.0, panel.y + 386.0, 96.0, 28.0 };
	}

	inline RectF EditorUnitParameterCloseRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorUnitParameterCloseRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorUnitParameterPanelWithPosRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 420.0, 430.0 };
	}

	inline RectF EditorUnitParameterPanelWithPosRect(const MapEditorState& editor)
	{
		return EditorUnitParameterPanelRect(editor);
	}

	inline RectF EditorUnitParameterDragHandleRect(const Vec2& pos)
	{
		const RectF panel = EditorUnitParameterPanelWithPosRect(pos);
		return RectF{ panel.x + panel.w - 24.0, panel.y + 6.0, 18.0, 18.0 };
	}

	inline RectF EditorUnitParameterDragHandleRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelWithPosRect(editor);
		return RectF{ panel.x + panel.w - 24.0, panel.y + 6.0, 18.0, 18.0 };
	}

	inline RectF BuildingEditorPanelWithPosRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 420.0, 680.0 };
	}

	inline RectF BuildingEditorPanelWithPosRect(const MapEditorState& editor)
	{
		return RectF{ editor.uiBuildingEditorPos.x, editor.uiBuildingEditorPos.y + EditorBarTopAnchorOffset(editor, editor.uiBuildingEditorTopAnchor), 420.0, 680.0 };
	}

	inline RectF EditorBuildingEditorDragHandleRect(const Vec2& pos)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(pos);
		return RectF{ panel.x + panel.w - 24.0, panel.y + 6.0, 18.0, 18.0 };
	}

	inline RectF EditorBuildingEditorDragHandleRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + panel.w - 24.0, panel.y + 6.0, 18.0, 18.0 };
	}
}
