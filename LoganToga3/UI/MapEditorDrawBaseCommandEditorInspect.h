#pragma once
# include <Siv3D.hpp>
# include "MapEditorCommandEditorHelpers.h"
# include "MapEditorDrawBaseCommandEditorHelpers.h"

namespace LT3
{
	// Command Editor のトグルボタンを描画する。
	inline void DrawCommandEditorToggleButton(const RectF& rect, bool active, StringView label, const Font& uiFont, const ColorF& activeFrame = ColorF{ 0.45, 0.90, 1.0, 0.85 })
	{
		rect.draw(active ? ColorF{ 0.12, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : (active ? activeFrame : ColorF{ 1, 1, 1, 0.16 }));
		uiFont(label).drawAt(11, rect.center(), active ? Palette::Aqua : Palette::Lightgray);
	}

	// Command Editor の Inspect パネルを描画する。
	inline void DrawCommandEditorInspectPanel(MapEditorState& editor, const BuildActionDef& selectedAction, const Array<String>& selectedSpawnTags, bool missingSpawnForUnitResult, const RectF& inspectTopViewport, const RectF& inspectBottomPanel, const Font& uiFont)
	{
		const bool showCarrierSettings = (selectedAction.resultType == BuildActionResultType::Carrier);
		const bool showCarrierCapacity = IsCommandCarrierCapacityVisible(selectedAction);
		const bool showLineSettings = (selectedAction.placementMode == BuildPlacementMode::Line);
		const double inspectContentHeight = GetCommandInspectContentHeight(showCarrierSettings, showCarrierCapacity, showLineSettings);
		const double inspectMaxScroll = Max(0.0, inspectContentHeight - inspectTopViewport.h + 8.0);
		editor.commandInspectScroll = Clamp(editor.commandInspectScroll, 0.0, inspectMaxScroll);

		inspectTopViewport.draw(ColorF{ 0.0, 0.0, 0.0, 0.14 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		const double scroll = editor.commandInspectScroll;
		uiFont(U"Owner").draw(12, inspectTopViewport.x + 12.0, inspectTopViewport.y + 12.0 - scroll, Palette::Lightgray);
		uiFont(selectedAction.ownerTag).draw(12, inspectTopViewport.x + 120.0, inspectTopViewport.y + 12.0 - scroll, Palette::White);
		uiFont(U"ID").draw(12, inspectTopViewport.x + 12.0, inspectTopViewport.y + 40.0 - scroll, Palette::Lightgray);
		uiFont(selectedAction.id).draw(12, inspectTopViewport.x + 120.0, inspectTopViewport.y + 40.0 - scroll, Palette::White);
		uiFont(U"Result Type").draw(12, inspectTopViewport.x + 12.0, inspectTopViewport.y + 68.0 - scroll, Palette::Lightgray);
		uiFont(BuildActionResultTypeToTomlValue(selectedAction.resultType)).draw(12, inspectTopViewport.x + 120.0, inspectTopViewport.y + 68.0 - scroll, Palette::White);
		uiFont(U"Spawn").draw(12, inspectTopViewport.x + 12.0, inspectTopViewport.y + 96.0 - scroll, Palette::Lightgray);
		uiFont(selectedSpawnTags.isEmpty() ? U"(none)" : selectedSpawnTags.join(U", ")).draw(12, inspectTopViewport.x + 120.0, inspectTopViewport.y + 96.0 - scroll, Palette::White);
		uiFont(U"Spawn Count").draw(12, inspectTopViewport.x + 12.0, inspectTopViewport.y + 124.0 - scroll, Palette::Lightgray);
		const RectF spawnCountRow = EditorCommandSpawnCountRowRect(scroll);
		const RectF spawnCountValueRect = EditorCommandSpawnCountValueRect(spawnCountRow);
		const bool spawnCountEditable = (selectedAction.resultType == BuildActionResultType::Unit);
		spawnCountRow.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		spawnCountValueRect.draw(spawnCountEditable ? ColorF{ 0.09, 0.10, 0.12, 0.96 } : ColorF{ 0.05, 0.06, 0.08, 0.80 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
		uiFont(U"{}"_fmt(Max(1, selectedAction.createCount))).drawAt(12, spawnCountValueRect.center(), spawnCountEditable ? Palette::White : Palette::Gray);
		for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
		{
			const RectF buttonRect = EditorCommandSpawnCountButtonRect(spawnCountRow, buttonIndex);
			buttonRect.draw(spawnCountEditable ? ColorF{ 0.08, 0.09, 0.11, 0.92 } : ColorF{ 0.05, 0.06, 0.08, 0.72 })
				.drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			if (buttonIndex == 0)
			{
				uiFont(U"-").drawAt(14, buttonRect.center(), spawnCountEditable ? Palette::White : Palette::Gray);
			}
			else if (buttonIndex == 1)
			{
				uiFont(U"+").drawAt(14, buttonRect.center(), spawnCountEditable ? Palette::White : Palette::Gray);
			}
			else
			{
				uiFont(U"R").drawAt(14, buttonRect.center(), spawnCountEditable ? Palette::White : Palette::Gray);
			}
		}

		uiFont(U"Result Type").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::ResultType, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
		DrawCommandEditorToggleButton(EditorCommandResultTypeUnitRect(scroll), selectedAction.resultType == BuildActionResultType::Unit, U"Unit", uiFont);
		DrawCommandEditorToggleButton(EditorCommandResultTypeObjectRect(scroll), selectedAction.resultType == BuildActionResultType::Object, U"Object", uiFont);
		DrawCommandEditorToggleButton(EditorCommandResultTypeCarrierRect(scroll), selectedAction.resultType == BuildActionResultType::Carrier, U"Carrier", uiFont);

		uiFont(U"Placement").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::Placement, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
		DrawCommandEditorToggleButton(EditorCommandPlacementToggleRect(scroll), selectedAction.isMove, selectedAction.isMove ? U"[ON] 場所指定して実行" : U"[OFF] 場所指定して実行", uiFont);

		uiFont(U"Enemy Production").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::EnemyProduction, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
		DrawCommandEditorToggleButton(EditorCommandEnemyCanProduceRect(scroll), selectedAction.enemyCanProduce, selectedAction.enemyCanProduce ? U"[ON] 敵陣営も生産可能" : U"[OFF] 敵陣営は生産不可", uiFont);

		if (showCarrierSettings)
		{
			uiFont(U"Carrier Action").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::CarrierAction, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
			DrawCommandEditorToggleButton(EditorCommandCarrierStoreRect(scroll), selectedAction.carrierAction == CarrierActionKind::Store, U"Store", uiFont);
			DrawCommandEditorToggleButton(EditorCommandCarrierReleaseRect(scroll), selectedAction.carrierAction == CarrierActionKind::Release, U"Release", uiFont);

			uiFont(U"Carrier Radius").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::CarrierRadius, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
			const RectF radiusRow = EditorCommandCarrierRadiusRowRect(scroll);
			const RectF radiusValueRect = EditorCommandCarrierRadiusValueRect(radiusRow);
			radiusRow.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			radiusValueRect.draw(ColorF{ 0.09, 0.10, 0.12, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
			uiFont(U"{} px"_fmt(Round(selectedAction.carrierRadiusPx))).drawAt(12, radiusValueRect.center(), Palette::White);
			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorCommandCarrierRadiusButtonRect(radiusRow, buttonIndex);
				buttonRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
					.drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				if (buttonIndex == 0)
				{
					uiFont(U"-").drawAt(14, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 1)
				{
					uiFont(U"+").drawAt(14, buttonRect.center(), Palette::White);
				}
				else
				{
					uiFont(U"R").drawAt(14, buttonRect.center(), Palette::White);
				}
			}
		}

		if (showCarrierCapacity)
		{
			uiFont(U"Carrier Capacity").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::CarrierCapacity, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
			const RectF capacityRow = EditorCommandCarrierCapacityRowRect(scroll);
			const RectF capacityValueRect = EditorCommandCarrierCapacityValueRect(capacityRow);
			capacityRow.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			capacityValueRect.draw(ColorF{ 0.09, 0.10, 0.12, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
			const String capacityLabel = (selectedAction.carrierMaxUnits <= 0) ? U"∞ (0)" : U"{}"_fmt(selectedAction.carrierMaxUnits);
			uiFont(capacityLabel).drawAt(12, capacityValueRect.center(), Palette::White);
			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorCommandCarrierCapacityButtonRect(capacityRow, buttonIndex);
				buttonRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
					.drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				if (buttonIndex == 0)
				{
					uiFont(U"-").drawAt(14, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 1)
				{
					uiFont(U"+").drawAt(14, buttonRect.center(), Palette::White);
				}
				else
				{
					uiFont(U"R").drawAt(14, buttonRect.center(), Palette::White);
				}
			}
		}

		uiFont(U"Placement Mode").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::PlacementMode, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
		DrawCommandEditorToggleButton(EditorCommandPlacementModePointRect(scroll), selectedAction.placementMode == BuildPlacementMode::Point, U"Point", uiFont);
		DrawCommandEditorToggleButton(EditorCommandPlacementModeLineRect(scroll), selectedAction.placementMode == BuildPlacementMode::Line, U"Line", uiFont);

		if (showLineSettings)
		{
			uiFont(U"Line Drag Input").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::LineDragInput, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
			DrawCommandEditorToggleButton(EditorCommandLineDragPlacementToggleRect(scroll), selectedAction.useRightDragPlacement, selectedAction.useRightDragPlacement ? U"[ON] 右ドラッグで範囲指定" : U"[OFF] 左クリック配置のみ", uiFont);

			uiFont(U"Line Axis").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::LineAxis, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
			DrawCommandEditorToggleButton(EditorCommandLineAxisAutoRect(scroll), selectedAction.lineAxisMode == BuildLineAxisMode::Auto, U"Auto", uiFont);
			DrawCommandEditorToggleButton(EditorCommandLineAxisHorizontalRect(scroll), selectedAction.lineAxisMode == BuildLineAxisMode::HorizontalOnly, U"Horizontal", uiFont);
			DrawCommandEditorToggleButton(EditorCommandLineAxisVerticalRect(scroll), selectedAction.lineAxisMode == BuildLineAxisMode::VerticalOnly, U"Vertical", uiFont);
		}

		const Array<std::pair<String, int32>> costRows = {
			{ U"Gold", selectedAction.costGold },
			{ U"Trust", selectedAction.costTrust },
			{ U"Food", selectedAction.costFood }
		};
		uiFont(U"Cost").draw(12, inspectTopViewport.x + 12.0, GetCommandInspectSectionLabelY(CommandInspectSection::Costs, showCarrierSettings, showCarrierCapacity, showLineSettings, scroll), Palette::Lightgray);
		for (int32 i = 0; i < static_cast<int32>(costRows.size()); ++i)
		{
			const RectF row = EditorCommandCostRowRect(i, scroll);
			row.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			uiFont(costRows[i].first).draw(11, row.x + 8.0, row.y + 4.0, Palette::Lightgray);
			uiFont(U"{}"_fmt(costRows[i].second)).draw(11, row.x + 92.0, row.y + 4.0, Palette::Gold);
			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorCommandCostButtonRect(row, buttonIndex);
				buttonRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				if (buttonIndex == 0)
				{
					uiFont(U"-").drawAt(14, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 1)
				{
					uiFont(U"+").drawAt(14, buttonRect.center(), Palette::White);
				}
				else
				{
					uiFont(U"R").drawAt(14, buttonRect.center(), Palette::White);
				}
			}
		}

		if (inspectMaxScroll > 0.0)
		{
			const double scrollRate = editor.commandInspectScroll / inspectMaxScroll;
			const double handleHeight = Max(28.0, inspectTopViewport.h * inspectTopViewport.h / Max(inspectTopViewport.h, inspectContentHeight));
			const double handleY = inspectTopViewport.y + (inspectTopViewport.h - handleHeight) * scrollRate;
			RectF{ inspectTopViewport.x + inspectTopViewport.w - 6.0, inspectTopViewport.y, 6.0, inspectTopViewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
			RectF{ inspectTopViewport.x + inspectTopViewport.w - 6.0, handleY, 6.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
		}

		inspectBottomPanel.draw(ColorF{ 0.03, 0.05, 0.07, 0.95 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
		if (missingSpawnForUnitResult)
		{
			uiFont(U"WARN: Unit result requires spawn target").draw(12, inspectBottomPanel.x + 12.0, inspectBottomPanel.y + 10.0, Palette::Orange);
		}
		else if (selectedAction.resultType == BuildActionResultType::Unit)
		{
			uiFont(U"Spawn Count: 1回の実行で生成する人数").draw(12, inspectBottomPanel.x + 12.0, inspectBottomPanel.y + 10.0, Palette::Aqua);
		}
		uiFont(U"Inspect: placement / line設定 + コスト編集").draw(12, inspectBottomPanel.x + 12.0, inspectBottomPanel.y + 34.0, Palette::Aqua);
		uiFont(U"固定設置系Unitは現在1体のみ配置").draw(11, inspectBottomPanel.x + 12.0, inspectBottomPanel.y + 54.0, Palette::Lightgray);
	}
}
