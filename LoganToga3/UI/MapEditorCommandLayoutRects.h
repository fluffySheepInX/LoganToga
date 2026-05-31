#pragma once
# include "MapEditorCoreTypes.h"

namespace LT3
{
	// Command パネル矩形を返す。
	inline RectF EditorCommandPanelRect()
	{
		return RectF{ 692.0, 72.0, 876.0, 610.0 };
	}

	// Command 一覧ビューポート矩形を返す。
	inline RectF EditorCommandListViewportRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + 20.0, panel.y + 58.0, 316.0, panel.h - 78.0 };
	}

	// Command 行矩形を返す。
	inline RectF EditorCommandRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 66.0 - scroll, viewport.w, 58.0 };
	}

	// Command 対象ユニットビューポート矩形を返す。
	inline RectF EditorCommandUnitViewportRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + 356.0, panel.y + 94.0, panel.w - 376.0, panel.h - 166.0 };
	}

	// Command モードタブ矩形を返す。
	inline RectF EditorCommandModeTabRect(int32 index)
	{
		const RectF panel = EditorCommandPanelRect();
		const double x = panel.x + 356.0 + index * 108.0;
		return RectF{ x, panel.y + 58.0, 100.0, 28.0 };
	}

	// Command 対象ユニットセル矩形を返す。
	inline RectF EditorCommandUnitCellRect(const RectF& viewport, int32 index, int32 columns, double scroll)
	{
		const int32 safeColumns = Max(1, columns);
		const int32 col = (index % safeColumns);
		const int32 row = (index / safeColumns);
		const Vec2 origin = viewport.pos + Vec2{ 8.0, 8.0 - scroll };
		const Vec2 step{ 96.0, 96.0 };
		return RectF{ origin + Vec2{ col * step.x, row * step.y }, 88.0, 88.0 };
	}

	// Spawn クリアボタン矩形を返す。
	inline RectF EditorCommandSpawnClearRect()
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + viewport.h - 72.0, 132.0, 28.0 };
	}

	// Spawn フッターテキスト矩形を返す。
	inline RectF EditorCommandSpawnFooterTextRect(int32 rowIndex)
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		const RectF clearRect = EditorCommandSpawnClearRect();
		const double x = (rowIndex == 0) ? (clearRect.x + clearRect.w + 12.0) : (viewport.x + 12.0);
		const double y = viewport.y + viewport.h - 68.0 + rowIndex * 20.0;
		const double w = (rowIndex == 0) ? Max(120.0, viewport.w - (x - viewport.x) - 12.0) : (viewport.w - 24.0);
		return RectF{ x, y, w, 18.0 };
	}

	// Command 保存ボタン矩形を返す。
	inline RectF EditorCommandSaveRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 220.0, panel.y + panel.h - 56.0, 96.0, 34.0 };
	}

	// Command ID 正規化ボタン矩形を返す。
	inline RectF EditorCommandNormalizeIdsRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 364.0, panel.y + panel.h - 56.0, 132.0, 34.0 };
	}

	// Command 閉じるボタン矩形を返す。
	inline RectF EditorCommandCloseRect()
	{
		const RectF panel = EditorCommandPanelRect();
		return RectF{ panel.x + panel.w - 112.0, panel.y + panel.h - 56.0, 88.0, 34.0 };
	}

	// Inspect 下部パネル矩形を返す。
	inline RectF EditorCommandInspectBottomPanelRect()
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		return RectF{ viewport.x, viewport.y + viewport.h - 76.0, viewport.w, 76.0 };
	}

	// Inspect 上部ビューポート矩形を返す。
	inline RectF EditorCommandInspectTopViewportRect()
	{
		const RectF viewport = EditorCommandUnitViewportRect();
		const RectF bottomPanel = EditorCommandInspectBottomPanelRect();
		return RectF{ viewport.x, viewport.y, viewport.w, Max(120.0, bottomPanel.y - viewport.y - 8.0) };
	}

	struct CommandInspectLayoutSpec
	{
		double insetX = 12.0;
		double topPadding = 12.0;
		double infoRowStride = 28.0;
		double sectionLabelGap = 6.0;
		double sectionSpacing = 26.0;
		double controlHeight = 28.0;
		double controlWidth = 340.0;
		double segmentedSpacing = 8.0;
		double segmentedTwoWidth = 164.0;
		double segmentedThreeWidth = 108.0;
		double lineControlSpacing = 20.0;
		double costRowSpacing = 30.0;
		double bottomPadding = 8.0;
	};

	enum class CommandInspectSection : int32
	{
		ResultType = 0,
		Placement,
		EnemyProduction,
		CarrierAction,
		CarrierRadius,
		CarrierCapacity,
		PlacementMode,
		LineDragInput,
		LineAxis,
		Costs,
	};

	// Inspect レイアウト定義を返す。
	inline const CommandInspectLayoutSpec& GetCommandInspectLayoutSpec()
	{
		static const CommandInspectLayoutSpec spec{};
		return spec;
	}

	// 表示条件込みの section index を返す。
	inline int32 GetCommandInspectSectionIndex(CommandInspectSection section, bool showCarrierSettings, bool showCarrierCapacity, bool showLineSettings)
	{
		int32 index = 0;
		for (const CommandInspectSection current : {
			CommandInspectSection::ResultType,
			CommandInspectSection::Placement,
			CommandInspectSection::EnemyProduction,
			CommandInspectSection::CarrierAction,
			CommandInspectSection::CarrierRadius,
			CommandInspectSection::CarrierCapacity,
			CommandInspectSection::PlacementMode,
			CommandInspectSection::LineDragInput,
			CommandInspectSection::LineAxis,
			CommandInspectSection::Costs })
		{
			const bool visible = ((current != CommandInspectSection::CarrierAction) || showCarrierSettings)
				&& ((current != CommandInspectSection::CarrierRadius) || showCarrierSettings)
				&& ((current != CommandInspectSection::CarrierCapacity) || showCarrierCapacity)
				&& ((current != CommandInspectSection::LineDragInput && current != CommandInspectSection::LineAxis) || showLineSettings);
			if (!visible)
			{
				continue;
			}

			if (current == section)
			{
				return index;
			}

			++index;
		}

		return -1;
	}

	// Inspect の共通 control 矩形を返す。
	inline RectF EditorCommandInspectControlRect(CommandInspectSection section, bool showCarrierSettings, bool showCarrierCapacity, bool showLineSettings, double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		const auto& spec = GetCommandInspectLayoutSpec();
		const int32 sectionIndex = Max(0, GetCommandInspectSectionIndex(section, showCarrierSettings, showCarrierCapacity, showLineSettings));
		const double infoBlockBottom = viewport.y + spec.topPadding + spec.infoRowStride * 5.0;
		const double sectionStartY = infoBlockBottom + sectionIndex * (spec.controlHeight + spec.sectionSpacing) - scroll;
		return RectF{ viewport.x + spec.insetX, sectionStartY + spec.sectionLabelGap + 14.0, Min(spec.controlWidth, viewport.w - spec.insetX * 2.0), spec.controlHeight };
	}

	// Inspect section ラベルの Y 座標を返す。
	inline double GetCommandInspectSectionLabelY(CommandInspectSection section, bool showCarrierSettings, bool showCarrierCapacity, bool showLineSettings, double scroll = 0.0)
	{
		return EditorCommandInspectControlRect(section, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll).y - GetCommandInspectLayoutSpec().sectionLabelGap - 14.0;
	}

	// 分割ボタン用 segment 矩形を返す。
	inline RectF EditorCommandInspectSegmentRect(const RectF& baseRect, int32 segmentIndex, int32 segmentCount)
	{
		const auto& spec = GetCommandInspectLayoutSpec();
		const int32 safeSegmentCount = Max(1, segmentCount);
		const double spacingTotal = spec.segmentedSpacing * (safeSegmentCount - 1);
		const double segmentWidth = (baseRect.w - spacingTotal) / safeSegmentCount;
		return RectF{ baseRect.x + segmentIndex * (segmentWidth + spec.segmentedSpacing), baseRect.y, segmentWidth, baseRect.h };
	}

	// Inspect コンテンツ全高を返す。
	inline double GetCommandInspectContentHeight(bool showCarrierSettings, bool showCarrierCapacity, bool showLineSettings)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		const RectF lastCostRow = EditorCommandInspectControlRect(CommandInspectSection::Costs, showCarrierSettings, showCarrierCapacity, showLineSettings);
		const auto& spec = GetCommandInspectLayoutSpec();
		return Max(viewport.h, (lastCostRow.y - viewport.y) + spec.controlHeight + spec.costRowSpacing * 2.0 + spec.bottomPadding);
	}

	// 配置トグル矩形を返す。
	inline RectF EditorCommandPlacementToggleRect(double scroll = 0.0)
	{
		return EditorCommandInspectControlRect(CommandInspectSection::Placement, true, true, scroll);
	}

	// 敵生産トグル矩形を返す。
	inline RectF EditorCommandEnemyCanProduceRect(double scroll = 0.0)
	{
		return EditorCommandInspectControlRect(CommandInspectSection::EnemyProduction, true, true, scroll);
	}

	// Spawn 数 row 矩形を返す。
	inline RectF EditorCommandSpawnCountRowRect(double scroll = 0.0)
	{
		const RectF viewport = EditorCommandInspectTopViewportRect();
		return RectF{ viewport.x + 12.0, viewport.y + 140.0 - scroll, Min(340.0, viewport.w - 24.0), 26.0 };
	}

	// Spawn 数値表示矩形を返す。
	inline RectF EditorCommandSpawnCountValueRect(const RectF& row)
	{
		const double valueWidth = 64.0;
		const double rightPadding = 88.0;
		return RectF{ row.x + row.w - rightPadding - valueWidth, row.y + 1.0, valueWidth, 24.0 };
	}

	// Spawn 数値ボタン矩形を返す。
	inline RectF EditorCommandSpawnCountButtonRect(const RectF& row, int32 buttonIndex)
	{
		const double buttonSize = 24.0;
		const double gap = 4.0;
		const double startX = row.x + row.w - (buttonSize * 3.0 + gap * 2.0) - 4.0;
		return RectF{ startX + buttonIndex * (buttonSize + gap), row.y + 1.0, buttonSize, buttonSize };
	}

	// Result Type の Unit 矩形を返す。
	inline RectF EditorCommandResultTypeUnitRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::ResultType, true, true, true, scroll), 0, 3);
	}

	// Result Type の Object 矩形を返す。
	inline RectF EditorCommandResultTypeObjectRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::ResultType, true, true, true, scroll), 1, 3);
	}

	// Result Type の Carrier 矩形を返す。
	inline RectF EditorCommandResultTypeCarrierRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::ResultType, true, true, true, scroll), 2, 3);
	}

	// Carrier Store 矩形を返す。
	inline RectF EditorCommandCarrierStoreRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::CarrierAction, true, true, true, scroll), 0, 2);
	}

	// Carrier Release 矩形を返す。
	inline RectF EditorCommandCarrierReleaseRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::CarrierAction, true, true, true, scroll), 1, 2);
	}

	// Carrier の半径 row 全体を返す。
	inline RectF EditorCommandCarrierRadiusRowRect(double scroll = 0.0)
	{
		const RectF baseRect = EditorCommandInspectControlRect(CommandInspectSection::CarrierRadius, true, true, true, scroll);
		return RectF{ baseRect.x, baseRect.y, baseRect.w, 26.0 };
	}

	// Carrier の半径 value 矩形を返す。
	inline RectF EditorCommandCarrierRadiusValueRect(const RectF& row)
	{
		const double valueWidth = 84.0;
		const double rightPadding = 108.0;
		return RectF{ row.x + row.w - rightPadding - valueWidth, row.y + 1.0, valueWidth, 24.0 };
	}

	// Carrier の半径ボタン矩形を返す。
	inline RectF EditorCommandCarrierRadiusButtonRect(const RectF& row, int32 buttonIndex)
	{
		const double buttonSize = 24.0;
		const double gap = 4.0;
		const double startX = row.x + row.w - (buttonSize * 3.0 + gap * 2.0) - 4.0;
		return RectF{ startX + buttonIndex * (buttonSize + gap), row.y + 1.0, buttonSize, buttonSize };
	}

	// Carrier の格納上限 row 全体を返す。
	inline RectF EditorCommandCarrierCapacityRowRect(double scroll = 0.0)
	{
		const RectF baseRect = EditorCommandInspectControlRect(CommandInspectSection::CarrierCapacity, true, true, true, scroll);
		return RectF{ baseRect.x, baseRect.y, baseRect.w, 26.0 };
	}

	// Carrier の格納上限 value 矩形を返す。
	inline RectF EditorCommandCarrierCapacityValueRect(const RectF& row)
	{
		const double valueWidth = 64.0;
		const double rightPadding = 88.0;
		return RectF{ row.x + row.w - rightPadding - valueWidth, row.y + 1.0, valueWidth, 24.0 };
	}

	// Carrier の格納上限ボタン矩形を返す。
	inline RectF EditorCommandCarrierCapacityButtonRect(const RectF& row, int32 buttonIndex)
	{
		const double buttonSize = 24.0;
		const double gap = 4.0;
		const double startX = row.x + row.w - (buttonSize * 3.0 + gap * 2.0) - 4.0;
		return RectF{ startX + buttonIndex * (buttonSize + gap), row.y + 1.0, buttonSize, buttonSize };
	}

	// 配置モード Point 矩形を返す。
	inline RectF EditorCommandPlacementModePointRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::PlacementMode, true, true, true, scroll), 0, 2);
	}

	// 配置モード Line 矩形を返す。
	inline RectF EditorCommandPlacementModeLineRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::PlacementMode, true, true, true, scroll), 1, 2);
	}

	// Line drag 配置トグル矩形を返す。
	inline RectF EditorCommandLineDragPlacementToggleRect(double scroll = 0.0)
	{
		return EditorCommandInspectControlRect(CommandInspectSection::LineDragInput, true, true, true, scroll);
	}

	// Line axis Auto 矩形を返す。
	inline RectF EditorCommandLineAxisAutoRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::LineAxis, true, true, true, scroll), 0, 3);
	}

	// Line axis Horizontal 矩形を返す。
	inline RectF EditorCommandLineAxisHorizontalRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::LineAxis, true, true, true, scroll), 1, 3);
	}

	// Line axis Vertical 矩形を返す。
	inline RectF EditorCommandLineAxisVerticalRect(double scroll = 0.0)
	{
		return EditorCommandInspectSegmentRect(EditorCommandInspectControlRect(CommandInspectSection::LineAxis, true, true, true, scroll), 2, 3);
	}

	// Cost row 矩形を返す。
	inline RectF EditorCommandCostRowRect(int32 index, double scroll = 0.0)
	{
		const RectF baseRect = EditorCommandInspectControlRect(CommandInspectSection::Costs, true, true, true, scroll);
		return RectF{ baseRect.x, baseRect.y + index * GetCommandInspectLayoutSpec().costRowSpacing, baseRect.w, 26.0 };
	}

	// Cost ボタン矩形を返す。
	inline RectF EditorCommandCostButtonRect(const RectF& row, int32 buttonIndex)
	{
		const double buttonSize = 24.0;
		const double gap = 4.0;
		const double startX = row.x + row.w - (buttonSize * 3.0 + gap * 2.0) - 4.0;
		return RectF{ startX + buttonIndex * (buttonSize + gap), row.y + 1.0, buttonSize, buttonSize };
	}

	// Command コンテキストメニュー矩形を返す。
	inline RectF EditorCommandContextMenuRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 156.0, 120.0 };
	}

	// Command コンテキストメニュー項目矩形を返す。
	inline RectF EditorCommandContextMenuItemRect(const Vec2& pos, int32 index)
	{
		const RectF menu = EditorCommandContextMenuRect(pos);
		return RectF{ menu.x + 4.0, menu.y + 4.0 + index * 30.0, menu.w - 8.0, 26.0 };
	}

	// Command 名変更オーバーレイ矩形を返す。
	inline RectF EditorCommandRenameOverlayRect(const RectF& row)
	{
		return RectF{ row.x + 56.0, row.y + 6.0, row.w - 64.0, 30.0 };
	}
}
