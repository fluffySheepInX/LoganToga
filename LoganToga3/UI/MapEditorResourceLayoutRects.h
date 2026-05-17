#pragma once
# include "MapEditorToolbarLayoutRects.h"

namespace LT3
{
	inline RectF EditorUiLayoutGridPanelRect()
	{
		return RectF{ 1040, 72, 236, 52 };
	}

	inline RectF EditorUiLayoutGridDecrementRect()
	{
		const RectF panel = EditorUiLayoutGridPanelRect();
		return RectF{ panel.x + 10.0, panel.y + 10.0, 32.0, 32.0 };
	}

	inline RectF EditorUiLayoutGridIncrementRect()
	{
		const RectF panel = EditorUiLayoutGridPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 32.0, 32.0 };
	}

	inline RectF EditorUiLayoutGridValueRect()
	{
		const RectF panel = EditorUiLayoutGridPanelRect();
		return RectF{ panel.x + 48.0, panel.y + 10.0, panel.w - 96.0, 32.0 };
	}

	inline RectF EditorResourceNodePanelRect()
	{
		return RectF{ 700, 404, 360, 248 };
	}

	inline RectF EditorResourceNodeCloseRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorResourceNodeRemoveRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + panel.w - 128.0, panel.y + panel.h - 42.0, 104.0, 28.0 };
	}

	inline RectF EditorResourceNodeListPanelRect()
	{
		const RectF starButton = EditorResourcePanelsToggleRect();
		constexpr double offsetFromStarX = 12.0;
		const double panelWidth = 236.0;
		return RectF{ starButton.x - offsetFromStarX - panelWidth, starButton.y, panelWidth, 320.0 };
	}

	inline RectF EditorResourceNodeListViewportRect()
	{
		const RectF panel = EditorResourceNodeListPanelRect();
		return RectF{ panel.x + 10.0, panel.y + 34.0, panel.w - 20.0, panel.h - 44.0 };
	}

	inline RectF EditorResourceNodeListRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 54.0 - scroll, viewport.w, 46.0 };
	}

	inline RectF EditorResourceNodeFilterRect(int32 index)
	{
		const RectF panel = EditorResourceNodeListPanelRect();
		return RectF{ panel.x + 10.0 + index * 54.0, panel.y + panel.h + 8.0, 48.0, 28.0 };
	}

	inline RectF EditorResourceValidationPanelRect()
	{
		const RectF panel = EditorResourceNodeListPanelRect();
		const RectF filter = EditorResourceNodeFilterRect(0);
		return RectF{ panel.x, filter.y + filter.h + 8.0, panel.w, 190.0 };
	}

	inline RectF EditorResourcePalettePanelRect()
	{
		const RectF nodePanel = EditorResourceNodeListPanelRect();
		return RectF{ nodePanel.x, 662, 236, 150 };
	}

	inline RectF EditorResourcePaletteIconRect(int32 index)
	{
		const RectF panel = EditorResourcePalettePanelRect();
		return RectF{ panel.x + 14.0 + index * 72.0, panel.y + 30.0, 56.0, 44.0 };
	}

	inline RectF EditorResourceClearAllRect()
	{
		const RectF panel = EditorResourcePalettePanelRect();
		constexpr double bottomOffset = 8.0;
		constexpr double buttonWidth = 88.0;
		constexpr double buttonHeight = 22.0;
		return RectF{ panel.x + panel.w - buttonWidth - 10.0, panel.y + panel.h - bottomOffset - buttonHeight, buttonWidth, buttonHeight };
	}

	inline RectF EditorPerlinNoisePanelRect()
	{
		return RectF{ 700, 132, 320, 284 };
	}

	inline RectF EditorPerlinNoiseCloseRect()
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorPerlinNoiseFileDialogRect()
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + 18.0, panel.y + 48.0, 132.0, 32.0 };
	}

	inline RectF EditorPerlinNoiseRunRect()
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + panel.w - 116.0, panel.y + panel.h - 42.0, 98.0, 30.0 };
	}

	inline RectF EditorPerlinNoiseSizeDecRect(bool widthAxis)
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + 18.0, panel.y + (widthAxis ? 100.0 : 142.0), 42.0, 30.0 };
	}

	inline RectF EditorPerlinNoiseSizeIncRect(bool widthAxis)
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + 202.0, panel.y + (widthAxis ? 100.0 : 142.0), 42.0, 30.0 };
	}

	inline RectF EditorPerlinNoiseStackViewportRect()
	{
		const RectF panel = EditorPerlinNoisePanelRect();
		return RectF{ panel.x + 18.0, panel.y + 188.0, panel.w - 36.0, 42.0 };
	}

	inline RectF EditorResourceNodeKindRect(int32 index)
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + 24.0 + index * 104.0, panel.y + 96.0, 92.0, 32.0 };
	}

	inline RectF EditorResourceNodeAmountDecRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + 24.0, panel.y + 158.0, 48.0, 40.0 };
	}

	inline RectF EditorResourceNodeAmountIncRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + 288.0, panel.y + 158.0, 48.0, 40.0 };
	}

	inline RectF EditorResourceNodeIncomeDecRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + 24.0, panel.y + 206.0, 48.0, 40.0 };
	}

	inline RectF EditorResourceNodeIncomeIncRect()
	{
		const RectF panel = EditorResourceNodePanelRect();
		return RectF{ panel.x + 288.0, panel.y + 206.0, 48.0, 40.0 };
	}
}
