#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	inline RectF EditorDecalEditorPanelRect()
	{
		return RectF{ 700, 404, 360, 344 };
	}

	inline RectF EditorDecalEditorCloseRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorDecalOpacityDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 96.0, 48.0, 40.0 };
	}

	inline RectF EditorDecalOpacityIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 96.0, 48.0, 40.0 };
	}

	inline RectF EditorDecalScaleDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 146.0, 48.0, 40.0 };
	}

	inline RectF EditorDecalScaleIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 146.0, 48.0, 40.0 };
	}

	inline RectF EditorDecalApplyRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 214.0, panel.y + 44.0, 122.0, 30.0 };
	}

	inline RectF EditorDecalOpacityRandomToggleRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 204.0, panel.w - 48.0, 28.0 };
	}

	inline RectF EditorDecalOpacityMinDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 238.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalOpacityMinIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 94.0, panel.y + 238.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalOpacityMaxDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 198.0, panel.y + 238.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalOpacityMaxIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 268.0, panel.y + 238.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalScaleRandomToggleRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 278.0, panel.w - 48.0, 28.0 };
	}

	inline RectF EditorDecalScaleMinDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 312.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalScaleMinIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 94.0, panel.y + 312.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalScaleMaxDecRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 198.0, panel.y + 312.0, 48.0, 34.0 };
	}

	inline RectF EditorDecalScaleMaxIncRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 268.0, panel.y + 312.0, 48.0, 34.0 };
	}

	inline constexpr double EditorZOrderLayerRowHeight = 38.0;
	inline constexpr double EditorZOrderHeaderHeight  = 52.0;
	inline constexpr double EditorZOrderFooterHeight  = 84.0;
	inline constexpr int32  EditorZOrderMaxVisibleLayers = 5;

	// maxStackSize を渡してパネル高さを動的に計算する
	inline RectF EditorZOrderPanelRect(int32 maxStackSize = 1)
	{
		const int32 visibleRows = Clamp(maxStackSize, 1, EditorZOrderMaxVisibleLayers);
		const double h = EditorZOrderHeaderHeight + visibleRows * EditorZOrderLayerRowHeight + EditorZOrderFooterHeight;
		// 下端が画面に収まるよう y を決める（1600x900想定）
		const double panelH = h;
		const double panelY = Min(900.0 - panelH - 8.0, 740.0);
		return RectF{ 700, panelY, 360, panelH };
	}

	// layerRowRect: i 番目レイヤー行の Rect
	inline RectF EditorZOrderLayerRowRect(const RectF& panel, int32 rowIndex)
	{
		return RectF{ panel.x + 8.0, panel.y + EditorZOrderHeaderHeight + rowIndex * EditorZOrderLayerRowHeight, panel.w - 16.0, EditorZOrderLayerRowHeight - 4.0 };
	}

	// row1: ⇤ SendToBack / ← Back / Front → / BringToFront ⇥
	inline RectF EditorZOrderSendToBackRect(const RectF& panel)
	{
		return RectF{ panel.x + 8.0, panel.y + panel.h - EditorZOrderFooterHeight + 8.0, 74.0, 30.0 };
	}

	inline RectF EditorZOrderDownRect(const RectF& panel)
	{
		return RectF{ panel.x + 88.0, panel.y + panel.h - EditorZOrderFooterHeight + 8.0, 74.0, 30.0 };
	}

	inline RectF EditorZOrderUpRect(const RectF& panel)
	{
		return RectF{ panel.x + panel.w - 162.0, panel.y + panel.h - EditorZOrderFooterHeight + 8.0, 74.0, 30.0 };
	}

	inline RectF EditorZOrderBringToFrontRect(const RectF& panel)
	{
		return RectF{ panel.x + panel.w - 82.0, panel.y + panel.h - EditorZOrderFooterHeight + 8.0, 74.0, 30.0 };
	}

	inline RectF EditorZOrderCloseRect(const RectF& panel)
	{
		return RectF{ panel.x + panel.w - 38.0, panel.y + 10.0, 28.0, 28.0 };
	}

	// 旧シグネチャ互換（パネルサイズ不明時のフォールバック用、input側で使用）
	inline RectF EditorZOrderPanelRectFallback()
	{
		return EditorZOrderPanelRect(EditorZOrderMaxVisibleLayers);
	}
}
