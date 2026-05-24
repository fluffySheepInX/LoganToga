#pragma once
# include "MapEditorCanvasInput.DecalEditor.h"

namespace LT3
{
	inline bool ProcessMapEditorZOrderPanelInput(MapEditorState& editor)
	{
		if (!editor.zOrderMode || !editor.zOrderSelectionRect)
		{
			return false;
		}

		const int32 maxStackSize = MaxDecalStackSizeInRect(editor, *editor.zOrderSelectionRect);
		const RectF panel = EditorZOrderPanelRect(maxStackSize);
		const RectF downRect = EditorZOrderDownRect(panel);
		const RectF upRect = EditorZOrderUpRect(panel);
		const RectF sendToBackRect = EditorZOrderSendToBackRect(panel);
		const RectF bringToFrontRect = EditorZOrderBringToFrontRect(panel);
		const RectF closeRect = EditorZOrderCloseRect(panel);

		if (closeRect.leftClicked())
		{
			editor.zOrderSelectionRect.reset();
			editor.zOrderSelectedStackIndex = 0;
			editor.statusText = U"Z-order selection cleared";
			return true;
		}

		editor.zOrderSelectedStackIndex = Clamp(editor.zOrderSelectedStackIndex, 0, Max(0, maxStackSize - 1));

		const int32 visibleRows = Clamp(maxStackSize, 1, EditorZOrderMaxVisibleLayers);
		for (int32 i = 0; i < visibleRows; ++i)
		{
			const RectF rowRect = EditorZOrderLayerRowRect(panel, i);
			if (rowRect.leftClicked())
			{
				editor.zOrderSelectedStackIndex = i;
				editor.statusText = U"Z-order selected layer {}"_fmt(i + 1);
				return true;
			}
		}

		if (sendToBackRect.leftClicked())
		{
			const int32 changed = MoveDecalToBackInRect(editor, *editor.zOrderSelectionRect, editor.zOrderSelectedStackIndex);
			editor.zOrderSelectedStackIndex = 0;
			editor.statusText = U"Z-order sent to BACK: {} cells"_fmt(changed);
			return true;
		}

		if (bringToFrontRect.leftClicked())
		{
			const int32 changed = MoveDecalToFrontInRect(editor, *editor.zOrderSelectionRect, editor.zOrderSelectedStackIndex);
			editor.zOrderSelectedStackIndex = Max(0, maxStackSize - 1);
			editor.statusText = U"Z-order brought to FRONT: {} cells"_fmt(changed);
			return true;
		}

		if (downRect.leftClicked())
		{
			const int32 changed = MoveDecalZOrderInRect(editor, *editor.zOrderSelectionRect, editor.zOrderSelectedStackIndex, -1);
			if (changed > 0)
			{
				editor.zOrderSelectedStackIndex = Max(0, editor.zOrderSelectedStackIndex - 1);
			}
			editor.statusText = U"Z-order moved back: {} cells"_fmt(changed);
			return true;
		}

		if (upRect.leftClicked())
		{
			const int32 changed = MoveDecalZOrderInRect(editor, *editor.zOrderSelectionRect, editor.zOrderSelectedStackIndex, 1);
			if (changed > 0)
			{
				editor.zOrderSelectedStackIndex = Min(maxStackSize - 1, editor.zOrderSelectedStackIndex + 1);
			}
			editor.statusText = U"Z-order moved front: {} cells"_fmt(changed);
			return true;
		}

		return panel.mouseOver();
	}
}
