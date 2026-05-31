#pragma once
# include <Siv3D.hpp>
# include "MapEditorCoreTypes.h"
# include "MapEditorMapData.h"
# include "MapEditorToolbarLayoutRects.h"
# include "RectUiHelpers.h"

namespace LT3
{
	// AI Editor 全体パネルの矩形を返す。
	inline RectF AiEditorPanelRect()
	{
		return RectF{ 692.0, 72.0, 876.0, 610.0 };
	}

	// プロファイル一覧ビューポートの矩形を返す。
	inline RectF AiEditorListViewportRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + 20.0, panel.y + 58.0, 280.0, panel.h - 86.0 };
	}

	// 詳細パネルの矩形を返す。
	inline RectF AiEditorDetailRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + 320.0, panel.y + 58.0, panel.w - 340.0, panel.h - 86.0 };
	}

	// プロファイル一覧の行矩形を返す。
	inline RectF AiEditorProfileRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 58.0 - scroll, viewport.w, 50.0 };
	}

	// 保存ボタンの矩形を返す。
	inline RectF AiEditorSaveRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 220.0, panel.y + 14.0, 96.0, 32.0 };
	}

	// 閉じるボタンの矩形を返す。
	inline RectF AiEditorCloseRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 112.0, panel.y + 14.0, 88.0, 32.0 };
	}

	// 適用ボタンの矩形を返す。
	inline RectF AiEditorApplyRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 328.0, panel.y + 14.0, 96.0, 32.0 };
	}

	// 詳細行の矩形を返す。
	inline RectF AiEditorValueRowRect(int32 row, double scroll)
	{
		const RectF detail = AiEditorDetailRect();
		return RectF{ detail.x + 16.0, detail.y + 126.0 + row * 42.0 - scroll, detail.w - 32.0, 34.0 };
	}

	// 詳細コンテンツの上端 Y を返す。
	inline double AiEditorDetailContentTop()
	{
		return AiEditorDetailRect().y + 118.0;
	}

	// 行が詳細ビューポート内に収まるかを判定する。
	inline bool AiEditorRowVisible(const RectF& row)
	{
		const RectF detail = AiEditorDetailRect();
		const double top = AiEditorDetailContentTop();
		return top <= row.y && row.y + row.h <= detail.y + detail.h;
	}

	// 行の値表示開始 X を返す。
	inline double AiEditorRowValueX(const RectF& row)
	{
		return row.x + 130.0;
	}

	// 行右端のアクション基準 X を返す。
	inline double AiEditorActionRightX(const RectF& row)
	{
		return row.x + row.w - 32.0;
	}

	// 基本値編集ボタンの矩形を返す。
	inline RectF AiEditorValueButtonRect(const RectF& row, int32 index)
	{
		const double w = 44.0;
		const double gap = 6.0;
		const double x = AiEditorActionRightX(row) - (w * 4.0 + gap * 3.0);
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

	// 横並びボタンの矩形を返す。
	inline RectF AiEditorInlineButtonRect(const RectF& row, int32 index, int32 count)
	{
		const double w = 58.0;
		const double gap = 6.0;
		const double x = AiEditorActionRightX(row) - (w * count + gap * (count - 1));
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

	// 小型横並びボタンの矩形を返す。
	inline RectF AiEditorTinyInlineButtonRect(const RectF& row, int32 index, int32 count)
	{
		const double w = 32.0;
		const double gap = 4.0;
		const double x = AiEditorActionRightX(row) - (w * count + gap * (count - 1));
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

	// 小型ボタン群の開始 X を返す。
	inline double AiEditorTinyButtonStartX(const RectF& row, int32 count)
	{
		return AiEditorTinyInlineButtonRect(row, 0, count).x;
	}

	// コンパクト横並びボタンの矩形を返す。
	inline RectF AiEditorCompactInlineButtonRect(const RectF& row, int32 index, int32 count)
	{
		const double w = 38.0;
		const double gap = 4.0;
		const double x = AiEditorActionRightX(row) - (w * count + gap * (count - 1));
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

	// コンパクトボタン群の開始 X を返す。
	inline double AiEditorCompactButtonStartX(const RectF& row, int32 count)
	{
		const RectF firstButton = AiEditorCompactInlineButtonRect(row, 0, count);
		return firstButton.x;
	}
}
