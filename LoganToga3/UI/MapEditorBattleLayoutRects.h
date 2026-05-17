#pragma once
# include "MapEditorToolbarLayoutRects.h"

namespace LT3
{
	inline RectF BattleResourcePanelRect(const MapEditorState& editor, size_t resourceCount)
	{
		const double rowHeight = 22.0;
		const double panelHeight = 42.0 + rowHeight * Max<size_t>(1, resourceCount);
		return RectF{ editor.uiResourcePanelPos.x, editor.uiResourcePanelPos.y + EditorBarTopAnchorOffset(editor, editor.uiResourcePanelTopAnchor), 282.0, panelHeight };
	}

	inline RectF UiLayoutTopAnchorToggleRect(const RectF& dragHandleRect)
	{
		return RectF{ dragHandleRect.x - 24.0, dragHandleRect.y, 18.0, 18.0 };
	}

	inline RectF BattleResourcePanelDragHandleRect(const MapEditorState& editor, size_t resourceCount)
	{
		const RectF panel = BattleResourcePanelRect(editor, resourceCount);
		return RectF{ panel.x + panel.w - 24.0, panel.y + 6.0, 18.0, 18.0 };
	}
}
