#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "RectLayoutPrimitives.h"
# include "RectNumberStepperTypes.h"
# include "RectValueRowPrimitives.h"

namespace LT3
{
	inline RectF SkillEditorPanelRect()
	{
		constexpr double panelX = 760.0;
		constexpr double panelY = 76.0;
		constexpr double panelW = 800.0;
		constexpr double bottomBarHeight = 30.0;
		constexpr double bottomMargin = 8.0;
		const double underBarY = (QuarterLogicalSceneHeight() - bottomBarHeight);
		const double panelH = Max(722.0, underBarY - panelY - bottomMargin);
		return RectF{ panelX, panelY, panelW, panelH };
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

	struct SkillEditorDetailLayoutModel
	{
		RectF detail;
		double scroll = 0.0;
		double contentTop = 0.0;
		double titleY = 0.0;
		double tagY = 0.0;
		double iconPreviewY = 0.0;
		double iconLabelY = 0.0;
		double iconBrowseY = 0.0;
		double projectileImageLabelY = 0.0;
		double projectileDiagonalLabelY = 0.0;
		double bomImageLabelY = 0.0;
		double soundEffectLabelY = 0.0;
		double soundEffectVolumeLabelY = 0.0;
		double warningIconY = 0.0;
		double kindLabelY = 0.0;
		double motionLabelY = 0.0;
		double centerLabelY = 0.0;
		double chainLabelY = 0.0;
		double nextButtonY = 0.0;
		double nextTagInputY = 0.0;
		double nextCandidateTopY = 0.0;
		double flagLabelY = 0.0;
		double flagButtonsY = 0.0;
		double valueRowsTopY = 0.0;
		double resourceCostsLabelY = 0.0;
		double resourceCostsTopY = 0.0;
		double contentHeight = 0.0;
	};

	/// <summary>
	/// SkillEditor 詳細パネルの各セクション位置を動的計算します。
	/// </summary>
	inline SkillEditorDetailLayoutModel BuildSkillEditorDetailLayoutModel(int32 resourceCostCount, double scroll = 0.0)
	{
		const RectF detail = SkillEditorDetailRect();
		SkillEditorDetailLayoutModel model;
		model.detail = detail;
		model.scroll = scroll;
		model.contentTop = detail.y - scroll;

		constexpr double titleTop = 12.0;
		constexpr double headerBottom = 266.0;
		constexpr double sectionGap = 12.0;
		constexpr double kindHeight = 30.0 * 2.0 + 8.0;
		constexpr double motionHeight = 30.0 * 2.0 + 18.0;
		constexpr double centerHeight = 24.0 + 18.0;
		constexpr double chainHeight = 24.0 + 24.0 + 24.0 + 72.0 + 18.0;
		constexpr double flagsHeight = 30.0 * 3.0 + 18.0;
		constexpr double valueRowsHeight = 30.0 * 38.0;
		constexpr double resourceHeaderHeight = 28.0;
		constexpr double resourceRowStride = 34.0;

		model.titleY = titleTop;
		model.tagY = 36.0;
		model.iconPreviewY = 58.0;
		model.iconLabelY = 58.0;
		model.iconBrowseY = 74.0;
		model.projectileImageLabelY = 108.0;
		model.projectileDiagonalLabelY = 138.0;
		model.bomImageLabelY = 168.0;
		model.soundEffectLabelY = 198.0;
		model.soundEffectVolumeLabelY = 230.0;
		model.warningIconY = 128.0;
		model.kindLabelY = titleTop + headerBottom;
		model.motionLabelY = model.kindLabelY + kindHeight + sectionGap;
		model.centerLabelY = model.motionLabelY + motionHeight + sectionGap;
		model.chainLabelY = model.centerLabelY + centerHeight + sectionGap;
		model.nextButtonY = model.chainLabelY + 28.0;
		model.nextTagInputY = model.nextButtonY + 32.0;
		model.nextCandidateTopY = model.nextTagInputY + 30.0;
		model.flagLabelY = model.chainLabelY + chainHeight + sectionGap;
		model.flagButtonsY = model.flagLabelY + 24.0;
		model.valueRowsTopY = model.flagLabelY + flagsHeight + sectionGap;
		model.resourceCostsLabelY = model.valueRowsTopY + valueRowsHeight + sectionGap;
		model.resourceCostsTopY = model.resourceCostsLabelY + resourceHeaderHeight;
		model.contentHeight = model.resourceCostsTopY
			+ Max(24.0, static_cast<double>(resourceCostCount) * resourceRowStride)
			+ resourceRowStride + 24.0;
		return model;
	}

	/// <summary>
	/// SkillEditor 詳細パネル全体のスクロール対象高さを返します。
	/// </summary>
	inline double SkillEditorDetailContentHeight(int32 resourceCostCount)
	{
		return BuildSkillEditorDetailLayoutModel(resourceCostCount).contentHeight;
	}

	inline RectF SkillEditorSkillRowRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorListViewportRect();
		return RectRow(viewport, visibleIndex, 42.0, 48.0, 0.0, 4.0, 4.0);
	}

	inline RectF SkillEditorContextMenuRect(const Vec2& pos)
	{
		return RectStepMenu(pos, 4, 156.0, 2.0, 28.0);
	}

	inline RectF SkillEditorContextMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectStepMenuItem(pos, index, 4.0, 4.0, SizeF{ 148.0, 22.0 }, 28.0);
	}

	// ユニットコンテキストメニュー (2項目: スキル全解除 / このスキルのみ残す)
	inline RectF SkillEditorUnitContextMenuRect(const Vec2& pos)
	{
		return RectStepMenu(pos, 2, 172.0, 2.0, 28.0);
	}

	inline RectF SkillEditorUnitContextMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectStepMenuItem(pos, index, 4.0, 4.0, SizeF{ 164.0, 22.0 }, 28.0);
	}

	inline RectF SkillEditorUnitIconRect(int32 visibleIndex)
	{
		const RectF viewport = SkillEditorUnitViewportRect();
		return RectLinearItem(viewport.pos + Vec2{ 10.0, 8.0 }, visibleIndex, SizeF{ 52.0, 52.0 }, 0.0, 58.0);
	}

	inline RectF SkillEditorIconPreviewRect(double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 10.0, layout.contentTop + layout.iconPreviewY, 54.0, 54.0 };
	}

	inline RectF SkillEditorIconBrowseRect(double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 72.0, layout.contentTop + layout.iconBrowseY, 82.0, 24.0 };
	}

	inline RectF SkillEditorProjectileImageBrowseRect(int32 index, double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		const double baseY = (index == 0) ? layout.projectileImageLabelY : layout.projectileDiagonalLabelY;
		return RectF{ layout.detail.x + 72.0, layout.contentTop + baseY, 82.0, 24.0 };
	}

	inline RectF SkillEditorProjectileImageClearRect(int32 index, double scroll)
	{
		const RectF browse = SkillEditorProjectileImageBrowseRect(index, scroll);
		return RectF{ browse.x + browse.w + 8.0, browse.y, 56.0, 24.0 };
	}

	inline RectF SkillEditorBomImageBrowseRect(double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 72.0, layout.contentTop + layout.bomImageLabelY, 82.0, 24.0 };
	}

	inline RectF SkillEditorBomImageClearRect(double scroll)
	{
		const RectF browse = SkillEditorBomImageBrowseRect(scroll);
		return RectF{ browse.x + browse.w + 8.0, browse.y, 56.0, 24.0 };
	}

	inline RectF SkillEditorSoundEffectBrowseRect(double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 72.0, layout.contentTop + layout.soundEffectLabelY, 82.0, 24.0 };
	}

	inline RectF SkillEditorSoundEffectClearRect(double scroll)
	{
		const RectF browse = SkillEditorSoundEffectBrowseRect(scroll);
		return RectF{ browse.x + browse.w + 8.0, browse.y, 56.0, 24.0 };
	}

	inline RectF SkillEditorSoundEffectPlayRect(double scroll)
	{
		const RectF clearRect = SkillEditorSoundEffectClearRect(scroll);
		return RectF{ clearRect.x + clearRect.w + 8.0, clearRect.y, 56.0, 24.0 };
	}

	inline RectNumberStepperRects SkillEditorSoundEffectVolumeStepperRects(double scroll)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		const double y = layout.contentTop + layout.soundEffectVolumeLabelY;
		return RectNumberStepperRects{
			RectF{ layout.detail.x + 72.0, y, 30.0, 24.0 },
			RectF{ layout.detail.x + 108.0, y, 82.0, 24.0 },
			RectF{ layout.detail.x + 196.0, y, 30.0, 24.0 },
			RectF{ layout.detail.x + 232.0, y, 54.0, 24.0 },
		};
	}

	inline RectF SkillEditorCenterButtonRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 78.0 + index * 62.0, layout.contentTop + layout.centerLabelY + 20.0, 56.0, 24.0 };
	}

	inline RectF SkillEditorToggleButtonRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		if (index < 5)
		{
			return RectF{ layout.detail.x + 78.0 + (index % 4) * 92.0, layout.contentTop + layout.flagButtonsY + (index / 4) * 30.0, 88.0, 24.0 };
		}
		return RectF{ layout.detail.x + 78.0 + ((index - 5) % 3) * 124.0, layout.contentTop + layout.flagButtonsY + 60.0 + ((index - 5) / 3) * 30.0, 118.0, 24.0 };
	}

	inline RectF SkillEditorNextSkillButtonRect(double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 78.0, layout.contentTop + layout.nextButtonY, 242.0, 24.0 };
	}

	inline RectF SkillEditorNextSkillInputRect(double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 78.0, layout.contentTop + layout.nextTagInputY, 242.0, 24.0 };
	}

	inline RectF SkillEditorNextSkillClearRect(double scroll = 0.0)
	{
		const RectF inputRect = SkillEditorNextSkillInputRect(scroll);
		return RectF{ inputRect.x + inputRect.w + 8.0, inputRect.y, 56.0, 24.0 };
	}

	inline RectF SkillEditorNextSkillCandidateRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 78.0, layout.contentTop + layout.nextCandidateTopY + index * 24.0, 320.0, 22.0 };
	}

	inline RectF SkillEditorKindButtonRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectGridItem(layout.detail.pos + Vec2{ 8.0, layout.kindLabelY + 20.0 - scroll }, index, 3, SizeF{ 78.0, 24.0 }, Vec2{ 86.0, 30.0 });
	}

	inline RectF SkillEditorMotionButtonRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectGridItem(layout.detail.pos + Vec2{ 8.0, layout.motionLabelY + 20.0 - scroll }, index, 4, SizeF{ 78.0, 24.0 }, Vec2{ 86.0, 30.0 });
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
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectValueRow(RectF{ layout.detail.x, layout.contentTop + layout.valueRowsTopY, layout.detail.w, 24.0 }, row, SkillEditorValueRowLayoutSpec());
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
		return RectF{ detail.x + 250.0, detail.y + 128.0 - scroll, 24.0, 24.0 };
	}

	inline RectF SkillEditorResourceCostTagRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 8.0, layout.contentTop + layout.resourceCostsTopY + index * 34.0, 116.0, 24.0 };
	}

	inline RectNumberStepperRects SkillEditorResourceCostAmountStepperRects(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		const double y = layout.contentTop + layout.resourceCostsTopY + index * 34.0;
		return RectNumberStepperRects{
			RectF{ layout.detail.x + 128.0, y, 30.0, 24.0 },
			RectF{ layout.detail.x + 164.0, y, 82.0, 24.0 },
			RectF{ layout.detail.x + 252.0, y, 30.0, 24.0 },
			RectF{ layout.detail.x + 288.0, y, 54.0, 24.0 },
		};
	}

	inline RectF SkillEditorResourceCostRemoveRect(int32 index, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 348.0, layout.contentTop + layout.resourceCostsTopY + index * 34.0, 54.0, 24.0 };
	}

	inline RectF SkillEditorResourceCostAddRect(int32 count, double scroll = 0.0)
	{
		const auto layout = BuildSkillEditorDetailLayoutModel(0, scroll);
		return RectF{ layout.detail.x + 8.0, layout.contentTop + layout.resourceCostsTopY + count * 34.0 + 8.0, 154.0, 24.0 };
	}

	inline RectF SkillEditorResourceCostStepMenuRect(const Vec2& pos, int32 itemCount)
	{
		return RectValueRowStepMenu(pos, itemCount);
	}

	inline RectF SkillEditorResourceCostStepMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectValueRowStepMenuItem(pos, index);
	}

	inline double SkillEditorDetailContentHeight()
	{
		return SkillEditorDetailContentHeight(0);
	}
}
