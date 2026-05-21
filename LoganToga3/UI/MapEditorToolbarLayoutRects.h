#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	inline RectF EditorToolbarRect()
	{
		return RectF{ 0, 8, 1600, 52 };
	}

	inline double EditorBarPreviewHideSec()
	{
		return 3.0;
	}

	inline bool IsEditorBarPreviewHidden(const MapEditorState& editor)
	{
		return Scene::Time() < editor.editorBarHiddenUntilSec;
	}

	inline double EditorBarTopAnchorOffset(const MapEditorState& editor, bool topAnchor)
	{
		return (topAnchor && IsEditorBarPreviewHidden(editor)) ? -96.0 : 0.0;
	}

	inline RectF EditorStatusBarRect()
	{
		const RectF toolbar = EditorToolbarRect();
		return RectF{ toolbar.x, toolbar.y + toolbar.h + 4.0, toolbar.w, 32.0 };
	}

	inline RectF EditorToolbarButtonRect(int32 index)
	{
		return RectF{ 24.0 + index * 112.0, 18.0, 104.0, 32.0 };
	}

	inline RectF EditorToolbarPreviewHideButtonRect()
	{
		return RectF{ 1360.0, 18.0, 184.0, 32.0 };
	}

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
		default:
			return none;
		}
	}

	inline RectF EditorToolbarButtonRect(const MapEditorState& editor, int32 buttonIndex)
	{
		const Optional<int32> visualIndex = EditorToolbarVisualIndex(editor, buttonIndex);
		if (!visualIndex)
		{
			return RectF{ 0, 0, 0, 0 };
		}

		return EditorToolbarButtonRect(*visualIndex);
	}

	inline RectF EditorPaletteRect(int32 index)
	{
		return RectF{ 1240.0, 152.0 + index * 54.0, 330.0, 46.0 };
	}

	inline RectF EditorPalettePanelRect()
	{
		return RectF{ 1220, 64, 370, 808 };
	}

	inline RectF EditorPaletteTabRect(int32 index)
	{
		const RectF panel = EditorPalettePanelRect();
		const double tabWidth = (panel.w - 40.0) * 0.5;
		return RectF{ panel.x + 16.0 + index * (tabWidth + 8.0), panel.y + 36.0, tabWidth, 32.0 };
	}

	inline RectF EditorResourcePanelsToggleRect()
	{
		const RectF panel = EditorPalettePanelRect();
		return RectF{ panel.x - 72.0, panel.y, 64.0, 64.0 };
	}

	inline RectF EditorStarToolMenuRect()
	{
		const RectF toggle = EditorResourcePanelsToggleRect();
		return RectF{ toggle.x - 188.0, toggle.y, 180.0, 202.0 };
	}

	inline RectF EditorStarToolMenuItemRect(int32 index)
	{
		const RectF menu = EditorStarToolMenuRect();
		return RectF{ menu.x + 8.0, menu.y + 8.0 + index * 38.0, menu.w - 16.0, 32.0 };
	}

	inline RectF EditorPaletteViewportRect()
	{
		const RectF panel = EditorPalettePanelRect();
		return RectF{ panel.x + 20.0, panel.y + 80.0, panel.w - 40.0, panel.h - 92.0 };
	}

	inline RectF EditorPaletteDecalPassageCheckboxRect(const RectF& itemRect)
	{
		return RectF{ itemRect.x + itemRect.w - 30.0, itemRect.y + 12.0, 18.0, 18.0 };
	}

	inline RectF EditorFogPanelRect()
	{
		const RectF menu = EditorStarToolMenuRect();
		return RectF{ menu.x - 260.0, menu.y, 252.0, 286.0 };
	}

	inline RectF EditorFogCloseRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 38.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF EditorFogToggleRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 52.0, panel.w - 36.0, 32.0 };
	}

	inline RectF EditorFogColorDecRect(int32 channel)
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 104.0 + channel * 38.0, 38.0, 30.0 };
	}

	inline RectF EditorFogColorIncRect(int32 channel)
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 56.0, panel.y + 104.0 + channel * 38.0, 38.0, 30.0 };
	}

	inline RectF EditorFogOpacityDecRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 226.0, 38.0, 30.0 };
	}

	inline RectF EditorFogOpacityIncRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + panel.w - 56.0, panel.y + 226.0, 38.0, 30.0 };
	}

	inline RectF EditorFogPreviewToggleRect()
	{
		const RectF panel = EditorFogPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 260.0, panel.w - 36.0, 20.0 };
	}

	inline RectF EditorUnitListPanelRect()
	{
		return RectF{ 24, 72, 650, 610 };
	}

	inline RectF EditorUnitListViewportRect()
	{
		return RectF{ 44, 126, 610, 530 };
	}

	inline RectF EditorUnitListRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 86.0 - scroll, viewport.w, 78.0 };
	}

	inline RectF EditorUnitListPreviewRect(const RectF& row)
	{
		return RectF{ row.x + 10.0, row.y + 11.0, 48.0, 48.0 };
	}

	inline RectF EditorUnitContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 140.0, 72.0 };
	}

	inline RectF EditorUnitContextMenuItemRect(const Vec2& pos, int32 index)
	{
		const RectF menu = EditorUnitContextMenuRect(pos);
		return RectF{ menu.x + 4.0, menu.y + 4.0 + index * 30.0, menu.w - 8.0, 26.0 };
	}

	inline RectF EditorUnitRenameOverlayRect(const RectF& row)
	{
		return RectF{ row.x + 70.0, row.y + 6.0, 360.0, 30.0 };
	}

	inline RectF EditorUnitNormalizeIdsRect()
	{
		const RectF panel = EditorUnitListPanelRect();
		return RectF{ panel.x + 20.0, panel.y + panel.h + 8.0, 164.0, 34.0 };
	}

	inline RectF EditorUnitStoreIdToTagRect()
	{
		const RectF normalizeRect = EditorUnitNormalizeIdsRect();
		return RectF{ normalizeRect.x + normalizeRect.w + 8.0, normalizeRect.y, 198.0, normalizeRect.h };
	}

	inline RectF EditorCommandPanelRect()
	{
		return RectF{ 692.0, 72.0, 876.0, 610.0 };
	}

	inline RectF EditorCommandListViewportRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + 20.0, panel.y + 58.0, 316.0, panel.h - 78.0 };
	}

	inline RectF EditorCommandRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 66.0 - scroll, viewport.w, 58.0 };
	}

	inline RectF EditorCommandUnitViewportRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + 356.0, panel.y + 94.0, panel.w - 376.0, panel.h - 166.0 };
	}

	inline RectF EditorCommandModeTabRect(int32 index)
	{
		const RectF panel = EditorCommandPanelRect();
		const double x = panel.x + 356.0 + index * 108.0;
		return RectF{ x, panel.y + 58.0, 100.0, 28.0 };
	}

	inline RectF EditorCommandUnitCellRect(const RectF& viewport, int32 index, int32 columns, double scroll)
	{
		const int32 safeColumns = Max(1, columns);
		const int32 col = (index % safeColumns);
		const int32 row = (index / safeColumns);
		const Vec2 origin = viewport.pos + Vec2{ 8.0, 8.0 - scroll };
		const Vec2 step{ 96.0, 96.0 };
		return RectF{ origin + Vec2{ col * step.x, row * step.y }, 88.0, 88.0 };
	}

	inline RectF EditorCommandSaveRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 220.0, panel.y + panel.h - 56.0, 96.0, 34.0 };
	}

	inline RectF EditorCommandNormalizeIdsRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 364.0, panel.y + panel.h - 56.0, 132.0, 34.0 };
	}

	inline RectF EditorCommandCloseRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 112.0, panel.y + panel.h - 56.0, 88.0, 34.0 };
	}

	inline RectF EditorCommandInspectBottomPanelRect()
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		return RectF{ viewport.x, viewport.y + viewport.h - 76.0, viewport.w, 76.0 };
	}

	inline RectF EditorCommandInspectTopViewportRect()
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		const RectF bottomPanel = EditorCommandInspectBottomPanelRect();
		return RectF{ viewport.x, viewport.y, viewport.w, Max(120.0, bottomPanel.y - viewport.y - 8.0) };
	}

	inline RectF EditorCommandPlacementToggleRect(double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + 168.0 - scroll, Min(340.0, viewport.w - 24.0), 30.0 };
	}

	inline RectF EditorCommandPlacementModePointRect(double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + 224.0 - scroll, Min(164.0, (viewport.w - 36.0) * 0.5), 28.0 };
	}

	inline RectF EditorCommandPlacementModeLineRect(double scroll = 0.0)
	{
		const RectF pointRect = EditorCommandPlacementModePointRect(scroll);
		const RectF viewport = EditorCommandInspectTopViewportRect();
		const double rightMargin = 12.0;
		const double spacing = 12.0;
		const double width = Min(164.0, Max(80.0, viewport.x + viewport.w - rightMargin - (pointRect.x + pointRect.w + spacing)));
		return RectF{ pointRect.x + pointRect.w + spacing, pointRect.y, width, pointRect.h };
	}

	inline RectF EditorCommandLineDragPlacementToggleRect(double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + 278.0 - scroll, Min(340.0, viewport.w - 24.0), 28.0 };
	}

	inline RectF EditorCommandLineAxisAutoRect(double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		const double available = Max(120.0, viewport.w - 24.0);
		const double width = Min(108.0, (available - 16.0) / 3.0);
		return RectF{ viewport.x + 12.0, viewport.y + 332.0 - scroll, width, 28.0 };
	}

	inline RectF EditorCommandLineAxisHorizontalRect(double scroll = 0.0)
	{
		const RectF autoRect = EditorCommandLineAxisAutoRect(scroll);
		return RectF{ autoRect.x + autoRect.w + 8.0, autoRect.y, autoRect.w, autoRect.h };
	}

	inline RectF EditorCommandLineAxisVerticalRect(double scroll = 0.0)
	{
		const RectF horizontalRect = EditorCommandLineAxisHorizontalRect(scroll);
		return RectF{ horizontalRect.x + horizontalRect.w + 8.0, horizontalRect.y, horizontalRect.w, horizontalRect.h };
	}

	inline RectF EditorCommandCostRowRect(int32 index, double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + 380.0 + index * 30.0 - scroll, Min(340.0, viewport.w - 24.0), 26.0 };
	}

	inline RectF EditorCommandCostButtonRect(const RectF& row, int32 buttonIndex)
	{
		const double buttonSize = 24.0;
		const double gap = 4.0;
		const double startX = row.x + row.w - (buttonSize * 3.0 + gap * 2.0) - 4.0;
		return RectF{ startX + buttonIndex * (buttonSize + gap), row.y + 1.0, buttonSize, buttonSize };
	}

	inline RectF EditorCommandContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 180.0, 104.0 };
	}

	inline RectF EditorCommandContextMenuItemRect(const Vec2& pos, int32 index)
	{
		const RectF menu = EditorCommandContextMenuRect(pos);
		return RectF{ menu.x + 4.0, menu.y + 4.0 + index * 30.0, menu.w - 8.0, 26.0 };
	}

	inline RectF EditorCommandRenameOverlayRect(const RectF& row)
	{
		return RectF{ row.x + 56.0, row.y + 6.0, row.w - 64.0, 30.0 };
	}
}
