#pragma once
# include "MapEditorToolbarLayoutRects.h"
# include "RectLayoutPrimitives.h"
# include "RectNumberStepperTypes.h"
# include "RectValueRowPrimitives.h"

namespace LT3
{
	inline RectF EditorUnitBuildingTabBarRect()
	{
		return RectF{ 600, 60, 420, 32 };
	}

	inline RectF EditorUnitBuildingTabBarRect(const MapEditorState& editor)
	{
		const Vec2 panelPos = editor.showBuildingEditor ? editor.uiBuildingEditorPos : editor.uiParamEditorPos;
		const bool topAnchor = editor.showBuildingEditor ? editor.uiBuildingEditorTopAnchor : editor.uiParamEditorTopAnchor;
		return RectF{ panelPos.x, panelPos.y - 32.0 + EditorBarTopAnchorOffset(editor, topAnchor), 420.0, 32.0 };
	}

	inline RectF EditorUnitBuildingTabRect(int32 index)
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		const double tabW = 140.0;
		return RectF{ bar.x + 8.0 + index * (tabW + 4.0), bar.y + 4.0, tabW, bar.h - 8.0 };
	}

	inline RectF EditorUnitBuildingTabRect(const MapEditorState& editor, int32 index)
	{
		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		const double tabW = 132.0;
		return RectF{ bar.x + 8.0 + index * (tabW + 4.0), bar.y + 4.0, tabW, bar.h - 8.0 };
	}

	inline RectF EditorUnitBuildingCloseRect()
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		return RectCloseButton(bar, 36.0, 4.0, SizeF{ 28.0, bar.h - 8.0 });
	}

	inline RectF EditorUnitBuildingCloseRect(const MapEditorState& editor)
	{
		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		return RectCloseButton(bar, 36.0, 4.0, SizeF{ 28.0, bar.h - 8.0 });
	}

	inline RectF EditorUnitParameterPanelRect()
	{
		const RectF bar = EditorUnitBuildingTabBarRect();
		return RectF{ bar.x, bar.y + bar.h, bar.w, 520.0 };
	}

	inline RectF EditorUnitParameterPanelRect(const MapEditorState& editor)
	{
		return RectF{ editor.uiParamEditorPos.x, editor.uiParamEditorPos.y + EditorBarTopAnchorOffset(editor, editor.uiParamEditorTopAnchor), 420.0, 520.0 };
	}

	inline RectF EditorUnitParamInnerTabRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 16.0 + index * 96.0, panel.y + 12.0, 88.0, 26.0 };
	}

	inline RectF EditorUnitParamHeaderDividerRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 86.0, panel.w - 28.0, 1.0 };
	}

	inline RectF EditorUniquePanelRect(const MapEditorState& editor)
	{
		return EditorUnitParameterPanelRect(editor);
	}

	inline RectF EditorUniquePortraitButtonRect(const MapEditorState& editor);
	inline RectF EditorUniquePortraitPreviewRect(const MapEditorState& editor);
	inline RectF EditorUniquePortraitClearRect(const MapEditorState& editor);
	inline RectF EditorUniqueRespawnCheckRect(const MapEditorState& editor);

	inline double EditorUniqueTopControlsBottomY(const MapEditorState& editor)
	{
		const RectF portraitButton = EditorUniquePortraitButtonRect(editor);
		const RectF portraitPreview = EditorUniquePortraitPreviewRect(editor);
		const RectF portraitClear = EditorUniquePortraitClearRect(editor);
		const RectF respawnCheck = EditorUniqueRespawnCheckRect(editor);
		return Max(Max(portraitButton.y + portraitButton.h, portraitPreview.y + portraitPreview.h), Max(portraitClear.y + portraitClear.h, respawnCheck.y + respawnCheck.h));
	}

	inline RectF EditorUniqueHeaderDividerRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 14.0, EditorUniqueTopControlsBottomY(editor) + 10.0, panel.w - 28.0, 1.0 };
	}

	inline RectF EditorUniqueCheckRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 18.0, panel.y + 52.0, 20.0, 20.0 };
	}

	inline RectF EditorUniquePortraitButtonRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 18.0, panel.y + 80.0, 90.0, 24.0 };
	}

	inline RectF EditorUniquePortraitPreviewRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 118.0, panel.y + 54.0, 46.0, 46.0 };
	}

	inline RectF EditorUniquePortraitClearRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 172.0, panel.y + 80.0, 34.0, 22.0 };
	}

	inline RectF EditorUniqueRespawnCheckRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		return RectF{ panel.x + 222.0, panel.y + 80.0, 20.0, 20.0 };
	}

	inline RectF EditorUniqueValueViewportRect(const MapEditorState& editor)
	{
		const RectF divider = EditorUniqueHeaderDividerRect(editor);
		return RectF{ divider.x, divider.y + divider.h + 10.0, divider.w, 154.0 };
	}

	inline RectValueRowLayoutSpec EditorUniqueValueRowLayoutSpec()
	{
		return RectValueRowLayoutSpec{};
	}

	inline RectF EditorUniqueValueRowRect(const RectF& viewport, int32 index)
	{
		return RectValueRow(viewport, index, EditorUniqueValueRowLayoutSpec());
	}

	inline RectF EditorUniqueValueRowNameRect(const RectF& row)
	{
		return RectValueRowName(row, EditorUniqueValueRowLayoutSpec());
	}

	inline RectF EditorUniqueValueRowValueRect(const RectF& row)
	{
		return RectValueRowValue(row, EditorUniqueValueRowLayoutSpec());
	}

	inline RectNumberStepperRects EditorUniqueValueRowStepperRects(const RectF& row)
	{
		return RectValueRowStepper(row, EditorUniqueValueRowLayoutSpec());
	}

	inline RectF EditorUniqueValueRowButtonRect(const RectF& row)
	{
		return RectValueRowButton(row, 0, EditorUniqueValueRowLayoutSpec());
	}

	inline RectF EditorUniqueValueStepMenuRect(const Vec2& pos, int32 itemCount)
	{
		return RectValueRowStepMenu(pos, itemCount);
	}

	inline RectF EditorUniqueValueStepMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectValueRowStepMenuItem(pos, index);
	}

	inline RectF EditorUniqueSpeechHeaderRect(const MapEditorState& editor)
	{
		const RectF valueViewport = EditorUniqueValueViewportRect(editor);
		return RectF{ valueViewport.x, valueViewport.y + valueViewport.h + 10.0, valueViewport.w, 30.0 };
	}

	inline RectF EditorUniqueSpeechAddRect(const MapEditorState& editor)
	{
		const RectF header = EditorUniqueSpeechHeaderRect(editor);
		return RectF{ header.x + header.w - 64.0, header.y + 4.0, 56.0, 22.0 };
	}

	inline RectF EditorUniqueSpeechViewportRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUniquePanelRect(editor);
		const RectF speechHeader = EditorUniqueSpeechHeaderRect(editor);
		return RectF{ speechHeader.x, speechHeader.y + speechHeader.h + 4.0, speechHeader.w, panel.y + panel.h - (speechHeader.y + speechHeader.h + 16.0) };
	}

	inline RectF EditorUniqueSpeechRowRect(const RectF& viewport, int32 index)
	{
		return RectRow(viewport, index, 46.0, 54.0);
	}

	inline RectF EditorUniqueSpeechTextRect(const RectF& row)
	{
		return RectF{ row.x + 8.0, row.y + 6.0, row.w - 116.0, row.h - 12.0 };
	}

	inline RectF EditorUniqueSpeechDeleteRect(const RectF& row)
	{
		return RectF{ row.x + row.w - 38.0, row.y + 10.0, 28.0, 24.0 };
	}

	inline RectF EditorUnitParamUniqueCheckRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 282.0, panel.y + 50.0, 20.0, 20.0 };
	}

	inline RectF EditorUnitParamPortraitButtonRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 310.0, panel.y + 48.0, 88.0, 24.0 };
	}

	inline RectF EditorUnitParamPortraitPreviewRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 334.0, panel.y + 74.0, 26.0, 26.0 };
	}

	inline RectF EditorUnitParamPortraitClearRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 370.0, panel.y + 74.0, 28.0, 20.0 };
	}

	inline RectF EditorUnitParamListHeaderRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 92.0, panel.w - 28.0, 34.0 };
	}

	inline RectF EditorUnitParamListViewportRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 14.0, panel.y + 128.0, panel.w - 28.0, panel.h - 146.0 };
	}

	inline RectValueRowLayoutSpec EditorUnitParamRowLayoutSpec()
	{
		RectValueRowLayoutSpec spec;
		spec.rowHeight = 50.0;
		spec.rowStride = 56.0;
		spec.fieldY = 7.0;
		spec.fieldHeightInset = 14.0;
		spec.nameX = 6.0;
		spec.nameW = 104.0;
		spec.valueX = 118.0;
		spec.valueW = 70.0;
		spec.minusX = 194.0;
		spec.plusX = 224.0;
		spec.stepX = 254.0;
		spec.stepW = 46.0;
		spec.buttonX = 304.0;
		spec.buttonW = 24.0;
		spec.buttonStride = 28.0;
		return spec;
	}

	inline RectF EditorUnitParamRowRect(const RectF& viewport, int32 index)
	{
		return RectValueRow(viewport, index, EditorUnitParamRowLayoutSpec());
	}

	inline RectF EditorUnitParamRowNameRect(const RectF& row)
	{
		return RectValueRowName(row, EditorUnitParamRowLayoutSpec());
	}

	inline RectF EditorUnitParamRowValueRect(const RectF& row)
	{
		return RectValueRowValue(row, EditorUnitParamRowLayoutSpec());
	}

	inline RectNumberStepperRects EditorUnitParamRowStepperRects(const RectF& row)
	{
		return RectValueRowStepper(row, EditorUnitParamRowLayoutSpec());
	}

	inline RectF EditorUnitParamRowButtonRect(const RectF& row, int32 buttonIndex)
	{
		return RectValueRowButton(row, buttonIndex, EditorUnitParamRowLayoutSpec());
	}

	inline RectF EditorUnitParamStepMenuRect(const Vec2& pos, int32 itemCount)
	{
		return RectValueRowStepMenu(pos, itemCount);
	}

	inline RectF EditorUnitParamStepMenuItemRect(const Vec2& pos, int32 index)
	{
		return RectValueRowStepMenuItem(pos, index);
	}

	inline RectF EditorUnitRowMoveUpRect(const RectF& row)
	{
		return RectF{ row.x + row.w - 66.0, row.y + 10.0, 24.0, 24.0 };
	}

	inline RectF EditorUnitRowMoveDownRect(const RectF& row)
	{
		return RectF{ row.x + row.w - 36.0, row.y + 10.0, 24.0, 24.0 };
	}

	inline RectF EditorUnitScaleDecrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleIncrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 128.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitScaleResetRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 132.0, panel.y + 176.0, 96.0, 34.0 };
	}

	inline RectF EditorUnitScaleResetRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 132.0, panel.y + 176.0, 96.0, 34.0 };
	}

	inline RectF EditorUnitMoveDecrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 24.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveIncrementRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 288.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 228.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitMoveUseSpeedRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectF{ panel.x + 116.0, panel.y + 276.0, 128.0, 28.0 };
	}

	inline RectF EditorUnitMoveUseSpeedRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 116.0, panel.y + 276.0, 128.0, 28.0 };
	}

	inline RectF EditorUnitVisionDecrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 338.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitVisionIncrementRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 288.0, panel.y + 338.0, 48.0, 40.0 };
	}

	inline RectF EditorUnitVisionResetRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectF{ panel.x + 132.0, panel.y + 386.0, 96.0, 28.0 };
	}

	inline RectF EditorUnitParameterCloseRect()
	{
		const RectF panel = EditorUnitParameterPanelRect();
		return RectCloseButton(panel);
	}

	inline RectF EditorUnitParameterCloseRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelRect(editor);
		return RectCloseButton(panel);
	}

	inline RectF EditorUnitParameterPanelWithPosRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 420.0, 430.0 };
	}

	inline RectF EditorUnitParameterPanelWithPosRect(const MapEditorState& editor)
	{
		return EditorUnitParameterPanelRect(editor);
	}

	inline RectF EditorUnitParameterDragHandleRect(const Vec2& pos)
	{
		const RectF panel = EditorUnitParameterPanelWithPosRect(pos);
		return RectDragHandle(panel);
	}

	inline RectF EditorUnitParameterDragHandleRect(const MapEditorState& editor)
	{
		const RectF panel = EditorUnitParameterPanelWithPosRect(editor);
		return RectDragHandle(panel);
	}

	inline RectF BuildingEditorPanelWithPosRect(const Vec2& pos)
	{
		return RectF{ pos.x, pos.y, 420.0, 680.0 };
	}

	inline RectF BuildingEditorPanelWithPosRect(const MapEditorState& editor)
	{
		return RectF{ editor.uiBuildingEditorPos.x, editor.uiBuildingEditorPos.y + EditorBarTopAnchorOffset(editor, editor.uiBuildingEditorTopAnchor), 420.0, 680.0 };
	}

	inline RectF EditorBuildingEditorDragHandleRect(const Vec2& pos)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(pos);
		return RectDragHandle(panel);
	}

	inline RectF EditorBuildingEditorDragHandleRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectDragHandle(panel);
	}
}
