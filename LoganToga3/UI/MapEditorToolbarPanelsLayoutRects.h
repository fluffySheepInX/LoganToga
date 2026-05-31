#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	// エディタツールバー全体の矩形を返す。
	inline RectF EditorToolbarRect()
	{
		return RectF{ 0, 8, 1600, 52 };
	}

	// エディタバーのプレビュー非表示秒数を返す。
	inline double EditorBarPreviewHideSec()
	{
		return 3.0;
	}

	// エディタバーのプレビューが非表示か返す。
	inline bool IsEditorBarPreviewHidden(const MapEditorState& editor)
	{
		return Scene::Time() < editor.editorBarHiddenUntilSec;
	}

	// エディタバーのレイアウトが非表示か返す。
	inline bool IsEditorBarLayoutHidden(const MapEditorState& editor)
	{
		return (!editor.editorToolbarAllowed) || IsEditorBarPreviewHidden(editor);
	}

	// エディタバー上端アンカーのオフセットを返す。
	inline double EditorBarTopAnchorOffset(const MapEditorState& editor, bool topAnchor)
	{
		return (topAnchor && IsEditorBarLayoutHidden(editor)) ? -96.0 : 0.0;
	}

	// ステータスバー全体の矩形を返す。
	inline RectF EditorStatusBarRect()
	{
		const RectF toolbar = EditorToolbarRect();
		return RectF{ toolbar.x, toolbar.y + toolbar.h + 4.0, toolbar.w, 32.0 };
	}

	// ツールバーボタン矩形を返す。
	inline RectF EditorToolbarButtonRect(int32 index)
	{
		return RectF{ 18.0 + index * 104.0, 18.0, 96.0, 32.0 };
	}

	// プレビュー非表示ボタン矩形を返す。
	inline RectF EditorToolbarPreviewHideButtonRect()
	{
		return RectF{ 1472.0, 18.0, 104.0, 32.0 };
	}

	// ツールバーボタンの表示順 index を返す。
	inline Optional<int32> EditorToolbarVisualIndex(const MapEditorState& editor, int32 buttonIndex)
	{
		if (editor.enabled)
		{
			return buttonIndex;
		}

		switch (buttonIndex)
		{
		case 0:
			return 0;
		case 6:
			return 1;
		case 7:
			return 2;
		case 8:
			return 3;
		case 9:
			return 4;
		case 10:
			return 5;
		case 11:
			return 6;
		case 12:
			return 7;
		case 13:
			return 8;
		default:
			return none;
		}
	}

	// エディタ状態に応じたツールバーボタン矩形を返す。
	inline RectF EditorToolbarButtonRect(const MapEditorState& editor, int32 buttonIndex)
	{
		const Optional<int32> visualIndex = EditorToolbarVisualIndex(editor, buttonIndex);
		if (!visualIndex)
		{
			return RectF{ 0, 0, 0, 0 };
		}

		return EditorToolbarButtonRect(*visualIndex);
	}

	// パレット項目矩形を返す。
	inline RectF EditorPaletteRect(int32 index)
	{
		return RectF{ 1240.0, 152.0 + index * 54.0, 330.0, 46.0 };
	}

	// パレット全体パネル矩形を返す。
	inline RectF EditorPalettePanelRect()
	{
		return RectF{ 1220, 64, 370, 808 };
	}

	// パレットタブ矩形を返す。
	inline RectF EditorPaletteTabRect(int32 index)
	{
		const RectF panel = EditorPalettePanelRect();
		const double tabWidth = (panel.w - 40.0) * 0.5;
		return RectF{ panel.x + 16.0 + index * (tabWidth + 8.0), panel.y + 36.0, tabWidth, 32.0 };
	}

	// 資源パネル切替ボタン矩形を返す。
	inline RectF EditorResourcePanelsToggleRect()
	{
		const RectF panel = EditorPalettePanelRect();
		return RectF{ panel.x - 72.0, panel.y, 64.0, 64.0 };
	}

	// 星形ツールメニュー矩形を返す。
	inline RectF EditorStarToolMenuRect()
	{
		const RectF toggle = EditorResourcePanelsToggleRect();
		return RectF{ toggle.x - 188.0, toggle.y, 180.0, 202.0 };
	}

	// 星形ツールメニュー項目矩形を返す。
	inline RectF EditorStarToolMenuItemRect(int32 index)
	{
		const RectF menu = EditorStarToolMenuRect();
		return RectF{ menu.x + 8.0, menu.y + 8.0 + index * 38.0, menu.w - 16.0, 32.0 };
	}

	// パレットのビューポート矩形を返す。
	inline RectF EditorPaletteViewportRect()
	{
		const RectF panel = EditorPalettePanelRect();
		return RectF{ panel.x + 20.0, panel.y + 80.0, panel.w - 40.0, panel.h - 92.0 };
	}

	// デカール通行可チェックボックス矩形を返す。
	inline RectF EditorPaletteDecalPassageCheckboxRect(const RectF& itemRect)
	{
		return RectF{ itemRect.x + itemRect.w - 30.0, itemRect.y + 12.0, 18.0, 18.0 };
	}

	// Fog パネル矩形を返す。
	inline RectF EditorFogPanelRect()
	{
		const RectF menu = EditorStarToolMenuRect();
		return RectF{ menu.x - 260.0, menu.y, 252.0, 286.0 };
	}

	// Fog 閉じるボタン矩形を返す。
	inline RectF EditorFogCloseRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 38.0, panel.y + 10.0, 28.0, 28.0 };
	}

	// Fog 切替ボタン矩形を返す。
	inline RectF EditorFogToggleRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 52.0, panel.w - 36.0, 32.0 };
	}

	// Fog 色減算ボタン矩形を返す。
	inline RectF EditorFogColorDecRect(int32 channel)
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 104.0 + channel * 38.0, 38.0, 30.0 };
	}

	// Fog 色加算ボタン矩形を返す。
	inline RectF EditorFogColorIncRect(int32 channel)
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 56.0, panel.y + 104.0 + channel * 38.0, 38.0, 30.0 };
	}

	// Fog 不透明度減算ボタン矩形を返す。
	inline RectF EditorFogOpacityDecRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 226.0, 38.0, 30.0 };
	}

	// Fog 不透明度加算ボタン矩形を返す。
	inline RectF EditorFogOpacityIncRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 56.0, panel.y + 226.0, 38.0, 30.0 };
	}

	// Fog プレビュー切替矩形を返す。
	inline RectF EditorFogPreviewToggleRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 260.0, panel.w - 36.0, 20.0 };
	}
}
