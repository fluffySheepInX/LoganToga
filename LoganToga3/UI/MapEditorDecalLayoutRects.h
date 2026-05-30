#pragma once
# include "MapEditorCoreTypes.h"
# include "RectLayoutPrimitives.h"

namespace LT3
{
	inline RectF EditorDecalEditorPanelRect()
	{
		return RectF{ 680, 260, 420, 628 };
	}

	inline RectF EditorDecalEditorPanelRect(const MapEditorState& editor)
	{
		return RectF{ editor.uiDecalEditorPos.x, editor.uiDecalEditorPos.y, 420.0, 628.0 };
	}

	inline RectF EditorDecalEditorDragHandleRect(const MapEditorState& editor)
	{
		const RectF panel = EditorDecalEditorPanelRect(editor);
		const RectF closeRect = RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
		constexpr double handleSize = 18.0;
		constexpr double handleOffset = 6.0;
		return RectF{ closeRect.x - handleSize - handleOffset, closeRect.y + (closeRect.h - handleSize) * 0.5, handleSize, handleSize };
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

	struct EditorDecalBasicLayoutRects
	{
		RectF viewport;
		RectF preview;
		RectF apply;
		RectF opacityRow;
		RectF scaleRow;
		RectF renderKindRow;
		RectF opacityRandomRow;
		RectF opacityRandomToggle;
		RectF opacityRandomValue;
		RectF opacityRandomButtonBand;
		RectF opacityMinDec;
		RectF opacityMinInc;
		RectF opacityMaxDec;
		RectF opacityMaxInc;
		RectF scaleRandomRow;
		RectF scaleRandomToggle;
		RectF scaleRandomValue;
		RectF scaleRandomButtonBand;
		RectF scaleMinDec;
		RectF scaleMinInc;
		RectF scaleMaxDec;
		RectF scaleMaxInc;
		RectF ambientSoundRow;
		RectF ambientSoundBrowse;
		RectF ambientSoundClear;
		RectF ambientSoundName;
		RectF ambientVolumeRow;
		RectF ambientVolumeDec;
		RectF ambientVolumeInc;
		double contentHeight = 0.0;
	};

	inline EditorDecalBasicLayoutRects BuildEditorDecalBasicLayoutRects(double scroll = 0.0)
	{
		const RectF panel = EditorDecalEditorPanelRect();
		const RectF tabBar = EditorDecalEditorTabBarRect();
		EditorDecalBasicLayoutRects layout;
		layout.viewport = RectF{ panel.x + 20.0, tabBar.y + tabBar.h + 16.0, panel.w - 40.0, panel.h - (tabBar.y + tabBar.h + 28.0 - panel.y) };

		const double contentX = layout.viewport.x + 4.0;
		const double contentW = layout.viewport.w - 8.0;
		double currentY = layout.viewport.y + 4.0 - scroll;

		layout.preview = RectF{ contentX, currentY, 64.0, 64.0 };
		layout.apply = RectF{ contentX + contentW - 134.0, currentY + 2.0, 134.0, 30.0 };
		currentY = Max(layout.preview.y + layout.preview.h, layout.apply.y + layout.apply.h) + 18.0;

		layout.opacityRow = RectF{ contentX, currentY, contentW, 44.0 };
		currentY += layout.opacityRow.h + 12.0;

		layout.scaleRow = RectF{ contentX, currentY, contentW, 44.0 };
		currentY += layout.scaleRow.h + 18.0;

		layout.renderKindRow = RectF{ contentX, currentY, contentW, 44.0 };
		currentY += layout.renderKindRow.h + 18.0;

		layout.opacityRandomRow = RectF{ contentX, currentY, contentW, 96.0 };
		layout.opacityRandomToggle = RectF{ layout.opacityRandomRow.x, layout.opacityRandomRow.y, layout.opacityRandomRow.w, 28.0 };
		layout.opacityRandomValue = RectF{ layout.opacityRandomToggle.x, layout.opacityRandomToggle.y + layout.opacityRandomToggle.h + 6.0, layout.opacityRandomToggle.w, 16.0 };
		layout.opacityRandomButtonBand = RectF{ layout.opacityRandomValue.x, layout.opacityRandomValue.y + layout.opacityRandomValue.h + 8.0, layout.opacityRandomValue.w, 34.0 };
		layout.opacityMinDec = RectF{ layout.opacityRandomButtonBand.x, layout.opacityRandomButtonBand.y, 52.0, layout.opacityRandomButtonBand.h };
		layout.opacityMinInc = RectF{ layout.opacityMinDec.x + layout.opacityMinDec.w + 18.0, layout.opacityMinDec.y, layout.opacityMinDec.w, layout.opacityMinDec.h };
		layout.opacityMaxDec = RectF{ layout.opacityRandomButtonBand.x + layout.opacityRandomButtonBand.w - 122.0, layout.opacityRandomButtonBand.y, 52.0, layout.opacityRandomButtonBand.h };
		layout.opacityMaxInc = RectF{ layout.opacityMaxDec.x + layout.opacityMaxDec.w + 18.0, layout.opacityMaxDec.y, layout.opacityMaxDec.w, layout.opacityMaxDec.h };
		currentY += layout.opacityRandomRow.h + 18.0;

		layout.scaleRandomRow = RectF{ contentX, currentY, contentW, 96.0 };
		layout.scaleRandomToggle = RectF{ layout.scaleRandomRow.x, layout.scaleRandomRow.y, layout.scaleRandomRow.w, 28.0 };
		layout.scaleRandomValue = RectF{ layout.scaleRandomToggle.x, layout.scaleRandomToggle.y + layout.scaleRandomToggle.h + 6.0, layout.scaleRandomToggle.w, 16.0 };
		layout.scaleRandomButtonBand = RectF{ layout.scaleRandomValue.x, layout.scaleRandomValue.y + layout.scaleRandomValue.h + 8.0, layout.scaleRandomValue.w, 34.0 };
		layout.scaleMinDec = RectF{ layout.scaleRandomButtonBand.x, layout.scaleRandomButtonBand.y, 52.0, layout.scaleRandomButtonBand.h };
		layout.scaleMinInc = RectF{ layout.scaleMinDec.x + layout.scaleMinDec.w + 18.0, layout.scaleMinDec.y, layout.scaleMinDec.w, layout.scaleMinDec.h };
		layout.scaleMaxDec = RectF{ layout.scaleRandomButtonBand.x + layout.scaleRandomButtonBand.w - 122.0, layout.scaleRandomButtonBand.y, 52.0, layout.scaleRandomButtonBand.h };
		layout.scaleMaxInc = RectF{ layout.scaleMaxDec.x + layout.scaleMaxDec.w + 18.0, layout.scaleMaxDec.y, layout.scaleMaxDec.w, layout.scaleMaxDec.h };
		currentY += layout.scaleRandomRow.h + 18.0;

		layout.ambientSoundRow = RectF{ contentX, currentY, contentW, 108.0 };
		layout.ambientSoundBrowse = RectF{ layout.ambientSoundRow.x, layout.ambientSoundRow.y, 94.0, 30.0 };
		layout.ambientSoundClear = RectF{ layout.ambientSoundBrowse.x + layout.ambientSoundBrowse.w + 8.0, layout.ambientSoundBrowse.y, 44.0, layout.ambientSoundBrowse.h };
		layout.ambientSoundName = RectF{ layout.ambientSoundClear.x + layout.ambientSoundClear.w + 10.0, layout.ambientSoundClear.y + 2.0, Max(0.0, layout.ambientSoundRow.x + layout.ambientSoundRow.w - (layout.ambientSoundClear.x + layout.ambientSoundClear.w + 14.0)), 24.0 };
		layout.ambientVolumeRow = RectF{ layout.ambientSoundRow.x, layout.ambientSoundRow.y + 44.0, layout.ambientSoundRow.w, 40.0 };
		layout.ambientVolumeDec = RectF{ layout.ambientVolumeRow.x, layout.ambientVolumeRow.y + 2.0, 52.0, layout.ambientVolumeRow.h - 4.0 };
		layout.ambientVolumeInc = RectF{ layout.ambientVolumeRow.x + layout.ambientVolumeRow.w - 52.0, layout.ambientVolumeRow.y + 2.0, 52.0, layout.ambientVolumeRow.h - 4.0 };

		layout.contentHeight = (layout.ambientSoundRow.y + scroll + layout.ambientSoundRow.h + 12.0) - layout.viewport.y;
		return layout;
	}

	inline RectF EditorDecalBasicViewportRect()
	{
		return BuildEditorDecalBasicLayoutRects().viewport;
	}

	inline RectF EditorDecalEditorApplyRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).apply;
	}

	inline double EditorDecalEditorPreviewBottomY(double scroll = 0.0)
	{
		const auto layout = BuildEditorDecalBasicLayoutRects(scroll);
		return Max(layout.preview.y + layout.preview.h, layout.apply.y + layout.apply.h);
	}

	inline RectF EditorDecalEditorPreviewRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).preview;
	}

	inline RectF EditorDecalOpacityRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityRow;
	}

	inline RectF EditorDecalScaleRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleRow;
	}

	inline RectF EditorDecalRenderKindRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).renderKindRow;
	}

	inline RectF EditorDecalRenderKindButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF row = EditorDecalRenderKindRowRect(scroll);
		const double gap = 8.0;
		const double width = (row.w - gap * 2.0) / 3.0;
		return RectF{ row.x + index * (width + gap), row.y + 8.0, width, 30.0 };
	}

	inline RectF EditorDecalOpacityRandomRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityRandomRow;
	}

	inline RectF EditorDecalScaleRandomRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleRandomRow;
	}

	inline RectF EditorDecalAmbientSoundRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientSoundRow;
	}

	inline RectF EditorDecalAmbientSoundBrowseRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientSoundBrowse;
	}

	inline RectF EditorDecalAmbientSoundClearRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientSoundClear;
	}

	inline RectF EditorDecalAmbientSoundNameRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientSoundName;
	}

	inline RectF EditorDecalAmbientVolumeRowRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientVolumeRow;
	}

	inline RectF EditorDecalAmbientVolumeDecRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientVolumeDec;
	}

	inline RectF EditorDecalAmbientVolumeIncRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).ambientVolumeInc;
	}

	inline RectF EditorDecalEditorCloseRect()
	{
		const RectF panel = EditorDecalEditorPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorDecalOpacityDecRect(double scroll = 0.0)
	{
		const RectF row = EditorDecalOpacityRowRect(scroll);
		return RectF{ row.x, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalOpacityIncRect(double scroll = 0.0)
	{
		const RectF row = EditorDecalOpacityRowRect(scroll);
		return RectF{ row.x + row.w - 52.0, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalScaleDecRect(double scroll = 0.0)
	{
		const RectF row = EditorDecalScaleRowRect(scroll);
		return RectF{ row.x, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalScaleIncRect(double scroll = 0.0)
	{
		const RectF row = EditorDecalScaleRowRect(scroll);
		return RectF{ row.x + row.w - 52.0, row.y + 2.0, 52.0, row.h - 4.0 };
	}

	inline RectF EditorDecalApplyRect(double scroll = 0.0)
	{
		return EditorDecalEditorApplyRect(scroll);
	}

	inline RectF EditorDecalOpacityRandomToggleRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityRandomToggle;
	}

	inline RectF EditorDecalOpacityRandomValueRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityRandomValue;
	}

	inline RectF EditorDecalOpacityRandomButtonBandRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityRandomButtonBand;
	}

	inline RectF EditorDecalOpacityMinDecRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityMinDec;
	}

	inline RectF EditorDecalOpacityMinIncRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityMinInc;
	}

	inline RectF EditorDecalOpacityMaxDecRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityMaxDec;
	}

	inline RectF EditorDecalOpacityMaxIncRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).opacityMaxInc;
	}

	inline RectF EditorDecalScaleRandomToggleRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleRandomToggle;
	}

	inline RectF EditorDecalScaleRandomValueRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleRandomValue;
	}

	inline RectF EditorDecalScaleRandomButtonBandRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleRandomButtonBand;
	}

	inline RectF EditorDecalScaleMinDecRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleMinDec;
	}

	inline RectF EditorDecalScaleMinIncRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleMinInc;
	}

	inline RectF EditorDecalScaleMaxDecRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleMaxDec;
	}

	inline RectF EditorDecalScaleMaxIncRect(double scroll = 0.0)
	{
		return BuildEditorDecalBasicLayoutRects(scroll).scaleMaxInc;
	}

	inline double EditorDecalBasicContentHeight()
	{
		return BuildEditorDecalBasicLayoutRects().contentHeight;
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

	inline RectF EditorZOrderPanelRect(const MapEditorState& editor, int32 maxStackSize = 1)
	{
		const int32 visibleRows = Clamp(maxStackSize, 1, EditorZOrderMaxVisibleLayers);
		const double panelH = EditorZOrderHeaderHeight + visibleRows * EditorZOrderLayerRowHeight + EditorZOrderFooterHeight;
		return RectF{ editor.uiZOrderPanelPos.x, editor.uiZOrderPanelPos.y, 360.0, panelH };
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

	inline RectF EditorZOrderDragHandleRect(const MapEditorState& editor, int32 maxStackSize = 1)
	{
		const RectF panel = EditorZOrderPanelRect(editor, maxStackSize);
		const RectF closeRect = EditorZOrderCloseRect(panel);
		constexpr double handleSize = 18.0;
		constexpr double handleOffset = 6.0;
		return RectF{ closeRect.x - handleSize - handleOffset, closeRect.y + (closeRect.h - handleSize) * 0.5, handleSize, handleSize };
	}

	// 旧シグネチャ互換（パネルサイズ不明時のフォールバック用、input側で使用）
	inline RectF EditorZOrderPanelRectFallback()
	{
		return EditorZOrderPanelRect(EditorZOrderMaxVisibleLayers);
	}
}
