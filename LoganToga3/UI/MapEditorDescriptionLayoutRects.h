#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	// 説明文編集パネル全体の矩形を返す。
	inline RectF EditorDescriptionPanelRect()
	{
		return RectF{ 520.0, 220.0, 560.0, 300.0 };
	}

	// 説明文編集テキスト領域の矩形を返す。
	inline RectF EditorDescriptionTextRect()
	{
		const RectF panel = EditorDescriptionPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 62.0, panel.w - 36.0, 132.0 };
	}

	// 説明文コピーボタンの矩形を返す。
	inline RectF EditorDescriptionCopyRect()
	{
		const RectF panel = EditorDescriptionPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 210.0, 120.0, 28.0 };
	}

	// 説明文貼り付けボタンの矩形を返す。
	inline RectF EditorDescriptionPasteRect()
	{
		const RectF panel = EditorDescriptionPanelRect();
		return RectF{ panel.x + 148.0, panel.y + 210.0, 120.0, 28.0 };
	}

	// 説明文保存ボタンの矩形を返す。
	inline RectF EditorDescriptionSaveRect()
	{
		const RectF panel = EditorDescriptionPanelRect();
		return RectF{ panel.x + panel.w - 158.0, panel.y + panel.h - 42.0, 64.0, 28.0 };
	}

	// 説明文キャンセルボタンの矩形を返す。
	inline RectF EditorDescriptionCancelRect()
	{
		const RectF panel = EditorDescriptionPanelRect();
		return RectF{ panel.x + panel.w - 84.0, panel.y + panel.h - 42.0, 64.0, 28.0 };
	}
}
