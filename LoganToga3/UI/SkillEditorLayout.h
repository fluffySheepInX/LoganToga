#pragma once
# include <Siv3D.hpp>
# include "RectLayoutPrimitives.h"
# include "RectNumberStepperTypes.h"
# include "RectValueRowPrimitives.h"

namespace LT3
{
	inline RectF SkillEditorPanelRect()
	{
		return RectF{ 760.0, 84.0, 800.0, 722.0 };
	}

	inline RectF SkillEditorCloseRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectCloseButton(panel);
	}

	inline RectF SkillEditorSandboxToggleRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectF{ panel.x + 146.0, panel.y + 12.0, 156.0, 26.0 };
	}

	inline RectF SkillEditorSaveRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectFromPanelBottomRight(panel, 138.0, 38.0, 118.0, 26.0);
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
		return RectFromPanel(panel, 106.0, 54.0, 188.0, panel.h - 82.0);
	}

	inline RectF SkillEditorUnitViewportRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectFromPanel(panel, 18.0, 54.0, 76.0, panel.h - 82.0);
	}

	inline RectF SkillEditorDetailRect()
	{
		const RectF panel = SkillEditorPanelRect();
		return RectFromPanel(panel, 310.0, 54.0, panel.w - 328.0, panel.h - 82.0);
	}

	inline RectF SkillEditorDetailViewportRect()
	{
		const RectF detail = SkillEditorDetailRect();
		return RectInset(detail, 8.0, 8.0, 8.0, 44.0);
	}

	inline RectF SkillEditorSkillRowRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorListViewportRect();
		return RectRow(viewport, visibleIndex, 42.0, 48.0, 0.0, 4.0, 4.0);
	}

	inline RectF SkillEditorContextMenuRect(const Vec2& pos)
	{
		return RectStepMenu(pos, 1, 156.0, 2.0, 28.0);
	}

	inline RectF SkillEditorContextMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectStepMenuItem(pos, index, 4.0, 4.0, SizeF{ 148.0, 22.0 }, 28.0);
	}

	inline RectF SkillEditorUnitIconRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorUnitViewportRect();
		return RectLinearItem(viewport.pos + Vec2{ 10.0, 8.0 }, visibleIndex, SizeF{ 52.0, 52.0 }, 0.0, 58.0);
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
		return RectGridItem(detail.pos + Vec2{ 8.0, 150.0 - scroll }, index, 3, SizeF{ 78.0, 24.0 }, Vec2{ 86.0, 30.0 });
	}

	inline RectF SkillEditorMotionButtonRect(int32 index, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectGridItem(detail.pos + Vec2{ 8.0, 228.0 - scroll }, index, 4, SizeF{ 78.0, 24.0 }, Vec2{ 86.0, 30.0 });
	}

	inline RectValueRowLayoutSpec SkillEditorValueRowLayoutSpec()
	{
		RectValueRowLayoutSpec spec;
		spec.rowHeight = 24.0;
		spec.rowStride = 38.0;
		spec.fieldY = 0.0;
		spec.fieldHeightInset = 0.0;
		spec.valueX = 164.0;
		spec.valueW = 82.0;
		spec.minusX = 128.0;
		spec.plusX = 252.0;
		spec.stepX = 288.0;
		spec.stepW = 54.0;
		spec.buttonX = 128.0;
		spec.buttonW = 30.0;
		spec.buttonStride = 36.0;
		spec.stepperButtonW = 30.0;
		return spec;
	}

	inline RectF SkillEditorValueRowRect(int32 row, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		return RectValueRow(RectF{ detail.x, detail.y + 358.0 - scroll, detail.w, 24.0 }, row, SkillEditorValueRowLayoutSpec());
	}

	inline RectF SkillEditorValueButtonRect(int32 row, int32 buttonIndex, double scroll = 0.0)
	{
		return RectValueRowButton(SkillEditorValueRowRect(row, scroll), buttonIndex, SkillEditorValueRowLayoutSpec());
	}

	inline RectF SkillEditorValueFieldRect(int32 row, double scroll = 0.0)
	{
		return RectValueRowValue(SkillEditorValueRowRect(row, scroll), SkillEditorValueRowLayoutSpec());
	}

	inline RectF SkillEditorValueStepRect(int32 row, double scroll = 0.0)
	{
		return RectValueRowStepper(SkillEditorValueRowRect(row, scroll), SkillEditorValueRowLayoutSpec()).step;
	}

	inline RectNumberStepperRects SkillEditorValueStepperRects(int32 row, double scroll = 0.0)
	{
		return RectValueRowStepper(SkillEditorValueRowRect(row, scroll), SkillEditorValueRowLayoutSpec());
	}

	inline RectF SkillEditorValueStepMenuRect(const Vec2& pos, int32 itemCount)
	{
		return RectValueRowStepMenu(pos, itemCount);
	}

	inline RectF SkillEditorValueStepMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectValueRowStepMenuItem(pos, index);
	}

	inline RectF SkillEditorValueHelpIconRect(int32 row, double scroll = 0.0)
	{
		const RectF stepRect = SkillEditorValueStepRect(row, scroll);
		return RectF{ stepRect.x + stepRect.w + 8.0, stepRect.y + 2.0, 20.0, 20.0 };
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
		return 1100.0;
	}
}
