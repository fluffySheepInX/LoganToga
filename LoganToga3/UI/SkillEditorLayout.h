#pragma once
# include <Siv3D.hpp>

namespace LT3
{
	inline RectF SkillEditorPanelRect()
	{
		return RectF{ 760.0, 84.0, 800.0, 722.0 };
	}

	inline RectF SkillEditorCloseRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF SkillEditorSandboxToggleRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 146.0, panel.y + 12.0, 156.0, 26.0 };
	}

	inline RectF SkillEditorSaveRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + panel.w - 138.0, panel.y + panel.h - 38.0, 118.0, 26.0 };
	}

	inline RectF SkillEditorSandboxPreviewRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ 24.0, panel.y, panel.x - 48.0, panel.h };
	}

	inline RectF SkillEditorSandboxArenaRect()
	{
		const RectF preview = SkillEditorSandboxPreviewRect();
		return RectF{ preview.x + 20.0, preview.y + 102.0, preview.w - 40.0, preview.h - 142.0 };
	}

	inline RectF SkillEditorSandboxButtonRect(int32 index)
	{
		const RectF preview = SkillEditorSandboxPreviewRect();
		return RectF{ preview.x + 20.0 + index * 96.0, preview.y + 68.0, 88.0, 24.0 };
	}

	inline RectF SkillEditorListViewportRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 106.0, panel.y + 54.0, 188.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorUnitViewportRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 18.0, panel.y + 54.0, 76.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorDetailRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 310.0, panel.y + 54.0, panel.w - 328.0, panel.h - 82.0 };
	}

	inline RectF SkillEditorDetailViewportRect()
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0, detail.y + 8.0, detail.w - 16.0, detail.h - 52.0 };
	}

	inline RectF SkillEditorSkillRowRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorListViewportRect();
		return RectF{ viewport.x + 4.0, viewport.y + 4.0 + visibleIndex * 48.0, viewport.w - 8.0, 42.0 };
	}

	inline RectF SkillEditorContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 156.0, 30.0 };
	}

	inline RectF SkillEditorContextMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectF{ pos.x + 4.0, pos.y + 4.0 + index * 28.0, 148.0, 22.0 };
	}

	inline RectF SkillEditorUnitIconRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorUnitViewportRect();
		return RectF{ viewport.x + 10.0, viewport.y + 8.0 + visibleIndex * 58.0, 52.0, 52.0 };
	}

	inline RectF SkillEditorIconPreviewRect(double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 10.0, detail.y + 58.0 - scroll, 54.0, 54.0 };
	}

	inline RectF SkillEditorIconBrowseRect(double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 72.0, detail.y + 74.0 - scroll, 82.0, 24.0 };
	}

	inline RectF SkillEditorProjectileImageBrowseRect(int32 index, double scroll)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 72.0, detail.y + 104.0 + index * 30.0 - scroll, 82.0, 24.0 };
	}

	inline RectF SkillEditorCenterButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 78.0 + index * 62.0, detail.y + 284.0 - scroll, 56.0, 24.0 };
	}

	inline RectF SkillEditorToggleButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 78.0 + index * 92.0, detail.y + 318.0 - scroll, 84.0, 24.0 };
	}

	inline RectF SkillEditorKindButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0 + (index % 3) * 86.0, detail.y + 150.0 + (index / 3) * 30.0 - scroll, 78.0, 24.0 };
	}

	inline RectF SkillEditorMotionButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 8.0 + (index % 4) * 86.0, detail.y + 228.0 + (index / 4) * 30.0 - scroll, 78.0, 24.0 };
	}

	inline RectF SkillEditorValueButtonRect(int32 row, int32 buttonIndex, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 128.0 + buttonIndex * 42.0, detail.y + 358.0 + row * 38.0 - scroll, 36.0, 24.0 };
	}

	inline RectF SkillEditorValueHelpIconRect(int32 row, double scroll = 0.0)
	{
		const RectF lastButton = SkillEditorValueButtonRect(row, 3, scroll);
		return RectF{ lastButton.x + lastButton.w + 8.0, lastButton.y + 2.0, 20.0, 20.0 };
	}

	inline RectF SkillEditorValueNoteIconRect(int32 row, double scroll = 0.0)
	{
		const RectF helpRect = SkillEditorValueHelpIconRect(row, scroll);
		return RectF{ helpRect.x + helpRect.w + 6.0, helpRect.y, 20.0, 20.0 };
	}

	inline RectF SkillEditorWarningIconRect(double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectF{ detail.x + 250.0, detail.y + 88.0 - scroll, 24.0, 24.0 };
	}

	inline double SkillEditorDetailContentHeight()
	{
		return 980.0;
	}
}
