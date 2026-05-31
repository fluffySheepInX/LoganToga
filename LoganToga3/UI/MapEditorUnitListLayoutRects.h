#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	// ユニット一覧パネル矩形を返す。
	inline RectF EditorUnitListPanelRect()
	{
		return RectF{ 24, 72, 650, 610 };
	}

	// ユニット一覧ビューポート矩形を返す。
	inline RectF EditorUnitListViewportRect()
	{
		return RectF{ 44, 126, 610, 530 };
	}

	// ユニット一覧の行矩形を返す。
	inline RectF EditorUnitListRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 86.0 - scroll, viewport.w, 78.0 };
	}

	// ユニット一覧プレビュー矩形を返す。
	inline RectF EditorUnitListPreviewRect(const RectF& row)
	{
		return RectF{ row.x + 10.0, row.y + 11.0, 48.0, 48.0 };
	}

	// ユニット右クリックメニュー矩形を返す。
	inline RectF EditorUnitContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 132.0, 90.0 };
	}

	// ユニット右クリックメニュー項目矩形を返す。
	inline RectF EditorUnitContextMenuItemRect(const Vec2& pos, int32 index)
	{
		const RectF menu = EditorUnitContextMenuRect(pos);
		return RectF{ menu.x + 4.0, menu.y + 4.0 + index * 30.0, menu.w - 8.0, 26.0 };
	}

	// ユニット名変更オーバーレイ矩形を返す。
	inline RectF EditorUnitRenameOverlayRect(const RectF& row)
	{
		return RectF{ row.x + 70.0, row.y + 6.0, 360.0, 30.0 };
	}

	// ユニット ID 正規化ボタン矩形を返す。
	inline RectF EditorUnitNormalizeIdsRect()
	{
		const RectF panel = EditorUnitListPanelRect();
		return RectF{ panel.x + 20.0, panel.y + panel.h + 8.0, 164.0, 34.0 };
	}

	// StoreId を tag へコピーするボタン矩形を返す。
	inline RectF EditorUnitStoreIdToTagRect()
	{
		const RectF normalizeRect = EditorUnitNormalizeIdsRect();
		return RectF{ normalizeRect.x + normalizeRect.w + 8.0, normalizeRect.y, 198.0, normalizeRect.h };
	}
}
