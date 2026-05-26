#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	inline RectF EditorDecalEditorPanelRect()
	{
		return RectF{ 680, 260, 420, 628 };
	}

	inline RectF EditorDecalEditorTabBarRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + 16.0, panel.y + 44.0, panel.w - 64.0, 32.0 };
	}

	inline RectF EditorDecalEditorTabRect(int32 index)
	{
		const RectF bar = EditorDecalEditorTabBarRect();
		const double tabWidth = (bar.w - 8.0) * 0.5;
		return RectF{ bar.x + index * (tabWidth + 8.0), bar.y, tabWidth, bar.h };
	}

	inline RectF EditorDecalEditorPreviewRect()
	{
		const RectF tabBar = EditorDecalEditorTabBarRect();
		return RectF{ tabBar.x, tabBar.y + tabBar.h + 14.0, 64.0, 64.0 };
	}

	inline RectF EditorDecalEditorApplyRect()
	{
		const RectF tabBar = EditorDecalEditorTabBarRect();
		return RectF{ tabBar.x + tabBar.w - 134.0, tabBar.y + tabBar.h + 10.0, 134.0, 30.0 };
	}

	inline double EditorDecalEditorPreviewBottomY()
	{
		const RectF preview = EditorDecalEditorPreviewRect();
		const RectF apply = EditorDecalEditorApplyRect();
		return Max(preview.y + preview.h, apply.y + apply.h);
	}

	inline RectF EditorDecalOpacityRowRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		const double y = EditorDecalEditorPreviewBottomY() + 18.0;
		return RectF{ panel.x + 24.0, y, panel.w - 48.0, 44.0 };
	}

	inline RectF EditorDecalScaleRowRect()
	{
		const RectF opacityRow = EditorDecalOpacityRowRect();
		return RectF{ opacityRow.x, opacityRow.y + opacityRow.h + 12.0, opacityRow.w, opacityRow.h };
	}

	inline RectF EditorDecalRenderKindRowRect()
	{
		const RectF scaleRow = EditorDecalScaleRowRect();
		return RectF{ scaleRow.x, scaleRow.y + scaleRow.h + 18.0, scaleRow.w, 44.0 };
	}

	inline RectF EditorDecalRenderKindButtonRect(int32 index)
	{
		const RectF row = EditorDecalRenderKindRowRect();
		const double gap = 8.0;
		const double width = (row.w - gap * 2.0) / 3.0;
		return RectF{ row.x + index * (width + gap), row.y + 8.0, width, 30.0 };
	}

	inline RectF EditorDecalOpacityRandomRowRect()
	{
		const RectF kindRow = EditorDecalRenderKindRowRect();
		return RectF{ kindRow.x, kindRow.y + kindRow.h + 18.0, kindRow.w, 96.0 };
	}

	inline RectF EditorDecalScaleRandomRowRect()
	{
		const RectF opacityRandomRow = EditorDecalOpacityRandomRowRect();
		return RectF{ opacityRandomRow.x, opacityRandomRow.y + opacityRandomRow.h + 18.0, opacityRandomRow.w, 96.0 };
	}

	inline RectF EditorDecalEditorCloseRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorDecalOpacityDecRect()
	{
		const RectF row = EditorDecalOpacityRowRect();
		return RectF{ row.x, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalOpacityIncRect()
	{
		const RectF row = EditorDecalOpacityRowRect();
		return RectF{ row.x + row.w - 52.0, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalScaleDecRect()
	{
		const RectF row = EditorDecalScaleRowRect();
		return RectF{ row.x, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalScaleIncRect()
	{
		const RectF row = EditorDecalScaleRowRect();
		return RectF{ row.x + row.w - 52.0, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalApplyRect()
	{
		return EditorDecalEditorApplyRect();
	}

	inline RectF EditorDecalOpacityRandomToggleRect()
	{
		const RectF row = EditorDecalOpacityRandomRowRect();
		return RectF{ row.x, row.y, row.w, 28.0 };
	}

	inline RectF EditorDecalOpacityRandomValueRect()
	{
		const RectF toggle = EditorDecalOpacityRandomToggleRect();
		return RectF{ toggle.x, toggle.y + toggle.h + 6.0, toggle.w, 16.0 };
	}

	inline RectF EditorDecalOpacityRandomButtonBandRect()
	{
		const RectF valueRect = EditorDecalOpacityRandomValueRect();
		return RectF{ valueRect.x, valueRect.y + valueRect.h + 8.0, valueRect.w, 34.0 };
	}

	inline RectF EditorDecalOpacityMinDecRect()
	{
		const RectF band = EditorDecalOpacityRandomButtonBandRect();
		return RectF{ band.x, band.y, 52.0, band.h };
	}

	inline RectF EditorDecalOpacityMinIncRect()
	{
		const RectF minDec = EditorDecalOpacityMinDecRect();
		return RectF{ minDec.x + minDec.w + 18.0, minDec.y, minDec.w, minDec.h };
	}

	inline RectF EditorDecalOpacityMaxDecRect()
	{
		const RectF row = EditorDecalOpacityRandomButtonBandRect();
		const double width = 52.0;
		return RectF{ row.x + row.w - width * 2.0 - 18.0, row.y, width, row.h };
	}

	inline RectF EditorDecalOpacityMaxIncRect()
	{
		const RectF maxDec = EditorDecalOpacityMaxDecRect();
		return RectF{ maxDec.x + maxDec.w + 18.0, maxDec.y, maxDec.w, maxDec.h };
	}

	inline RectF EditorDecalScaleRandomToggleRect()
	{
		const RectF row = EditorDecalScaleRandomRowRect();
		return RectF{ row.x, row.y, row.w, 28.0 };
	}

	inline RectF EditorDecalScaleRandomValueRect()
	{
		const RectF toggle = EditorDecalScaleRandomToggleRect();
		return RectF{ toggle.x, toggle.y + toggle.h + 6.0, toggle.w, 16.0 };
	}

	inline RectF EditorDecalScaleRandomButtonBandRect()
	{
		const RectF valueRect = EditorDecalScaleRandomValueRect();
		return RectF{ valueRect.x, valueRect.y + valueRect.h + 8.0, valueRect.w, 34.0 };
	}

	inline RectF EditorDecalScaleMinDecRect()
	{
		const RectF band = EditorDecalScaleRandomButtonBandRect();
		return RectF{ band.x, band.y, 52.0, band.h };
	}

	inline RectF EditorDecalScaleMinIncRect()
	{
		const RectF minDec = EditorDecalScaleMinDecRect();
		return RectF{ minDec.x + minDec.w + 18.0, minDec.y, minDec.w, minDec.h };
	}

	inline RectF EditorDecalScaleMaxDecRect()
	{
		const RectF row = EditorDecalScaleRandomButtonBandRect();
		const double width = 52.0;
		return RectF{ row.x + row.w - width * 2.0 - 18.0, row.y, width, row.h };
	}

	inline RectF EditorDecalScaleMaxIncRect()
	{
		const RectF maxDec = EditorDecalScaleMaxDecRect();
		return RectF{ maxDec.x + maxDec.w + 18.0, maxDec.y, maxDec.w, maxDec.h };
	}

	inline RectF EditorDecalShadowViewportRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		const RectF tabBar = EditorDecalEditorTabBarRect();
		return RectF{ panel.x + 20.0, tabBar.y + tabBar.h + 16.0, panel.w - 40.0, panel.h - (tabBar.y + tabBar.h + 28.0 - panel.y) };
	}

	inline RectF EditorDecalShadowPreviewRect(double scroll = 0.0)
	{
		const RectF viewport = EditorDecalShadowViewportRect();
		return RectF{ viewport.x, viewport.y + 4.0 - scroll, viewport.w, 112.0 };
	}

	inline RectF EditorDecalShadowEnabledRect(double scroll = 0.0)
	{
		const RectF preview = EditorDecalShadowPreviewRect(scroll);
		return RectF{ preview.x, preview.y + preview.h + 12.0, preview.w, 30.0 };
	}

	inline RectF EditorDecalShadowModeRect(int32 index, double scroll = 0.0)
	{
		const RectF enabled = EditorDecalShadowEnabledRect(scroll);
		const double width = (enabled.w - 10.0) * 0.5;
		return RectF{ enabled.x + index * (width + 10.0), enabled.y + enabled.h + 26.0, width, 30.0 };
	}

	inline RectF EditorDecalShadowDirectionGridRect(double scroll = 0.0)
	{
		const RectF modeRect = EditorDecalShadowModeRect(0, scroll);
		return RectF{ modeRect.x, modeRect.y + modeRect.h + 32.0, EditorDecalShadowViewportRect().w, 132.0 };
	}

	inline RectF EditorDecalShadowDirectionButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF grid = EditorDecalShadowDirectionGridRect(scroll);
		const int32 column = index % 4;
		const int32 row = index / 4;
		const double width = (grid.w - 18.0) / 4.0;
		return RectF{ grid.x + column * (width + 6.0), grid.y + 26.0 + row * 42.0, width, 36.0 };
	}

	inline RectF EditorDecalShadowLengthRowRect(double scroll = 0.0)
	{
		const RectF grid = EditorDecalShadowDirectionGridRect(scroll);
		return RectF{ grid.x, grid.y + grid.h + 18.0, grid.w, 40.0 };
	}

	inline RectF EditorDecalShadowOpacityRowRect(double scroll = 0.0)
	{
		const RectF lengthRow = EditorDecalShadowLengthRowRect(scroll);
		return RectF{ lengthRow.x, lengthRow.y + lengthRow.h + 12.0, lengthRow.w, 40.0 };
	}

	inline RectF EditorDecalShadowBlurRowRect(double scroll = 0.0)
	{
		const RectF opacityRow = EditorDecalShadowOpacityRowRect(scroll);
		return RectF{ opacityRow.x, opacityRow.y + opacityRow.h + 12.0, opacityRow.w, 40.0 };
	}

	inline RectF EditorDecalShadowValueDecRect(const RectF& row)
	{
		return RectF{ row.x, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalShadowValueIncRect(const RectF& row)
	{
		return RectF{ row.x + row.w - 52.0, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline double EditorDecalShadowContentHeight()
	{
		const RectF blurRow = EditorDecalShadowBlurRowRect();
		const RectF viewport = EditorDecalShadowViewportRect();
		return (blurRow.y + blurRow.h + 12.0) - viewport.y;
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
