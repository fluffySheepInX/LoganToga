#pragma once
# include "MapEditorUnitCatalogEditorInput.ListInput.h"
# include "MapEditorUnitParamEditorCommon.h"
# include "EditorMutationHelpers.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline bool ProcessUnitCatalogParamEditorInput(MapEditorState& editor, UnitCatalog& catalog, bool& consumed)
	{
		if (!(editor.showUnitParameterEditor && EditorUnitParameterPanelRect(editor).mouseOver()))
		{
			return false;
		}

		EnsureUnitParamSteps(editor);
		consumed = true;
		if (EditorUnitParameterCloseRect(editor).leftClicked() || EditorUnitBuildingCloseRect(editor).leftClicked())
		{
			editor.showUnitParameterEditor = false;
			editor.showBuildingEditor = false;
		}

		for (int32 tabIndex = 0; tabIndex < 4; ++tabIndex)
		{
			if (EditorUnitParamInnerTabRect(editor, tabIndex).leftClicked())
			{
				editor.unitParamEditorTab = tabIndex;
				return true;
			}
		}

		if (!(0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size())))
		{
			return true;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];

		const Array<UnitParamRowSpec> rows = UnitParamRowSpecs(editor.unitParamEditorTab);
		auto commitEditingText = [&]()
			{
				if (!MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return TryCommitUnitParamText(selected, rows[editor.unitParamEditingRow].kind, editor.unitParamEditingText);
				}))
				{
					editor.statusText = U"Invalid unit param value: {}"_fmt(editor.unitParamEditingText);
				}
			};

		if (editor.unitParamEditingRow >= 0)
		{
			TextInput::UpdateText(editor.unitParamEditingText);
			if (KeyEscape.down())
			{
				editor.unitParamEditingRow = -1;
				editor.unitParamEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				if (0 <= editor.unitParamEditingRow && editor.unitParamEditingRow < static_cast<int32>(rows.size()))
				{
					commitEditingText();
				}
				editor.unitParamEditingRow = -1;
				editor.unitParamEditingText.clear();
				return true;
			}
		}

		if (editor.unitParamStepMenuRow)
		{
			const Array<double>& steps = UnitParamDefaultSteps();
			const RectF menuRect = EditorUnitParamStepMenuRect(editor.unitParamStepMenuPos, static_cast<int32>(steps.size()));
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				if (EditorUnitParamStepMenuItemRect(editor.unitParamStepMenuPos, i).leftClicked())
				{
					SetUnitParamStep(editor, editor.unitParamEditorTab, *editor.unitParamStepMenuRow, steps[i]);
					editor.unitParamStepMenuRow = none;
					editor.statusText = U"Unit param step set to {}"_fmt(steps[i]);
					return true;
				}
			}

			if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.unitParamStepMenuRow = none;
				return true;
			}

			return true;
		}

		const RectF viewport = EditorUnitParamListViewportRect(editor);
		if (editor.unitParamEditingRow >= 0 && MouseL.down())
		{
			const RectF editingRow = EditorUnitParamRowRect(viewport, editor.unitParamEditingRow);
			const RectF editingRect = EditorUnitParamRowValueRect(editingRow);
			if (!editingRect.mouseOver())
			{
				if (0 <= editor.unitParamEditingRow && editor.unitParamEditingRow < static_cast<int32>(rows.size()))
				{
					commitEditingText();
				}
				editor.unitParamEditingRow = -1;
				editor.unitParamEditingText.clear();
				return true;
			}
		}

		for (int32 rowIndex = 0; rowIndex < static_cast<int32>(rows.size()); ++rowIndex)
		{
			const RectF row = EditorUnitParamRowRect(viewport, rowIndex);
			if (!viewport.intersects(row))
			{
				continue;
			}

			const RectNumberStepperRects rects = EditorUnitParamRowStepperRects(row);
			switch (DetectRectNumberStepperInput(rects))
			{
			case RectNumberStepperInputAction::StartValueEdit:
				editor.unitParamEditingRow = rowIndex;
				editor.unitParamEditingText = UnitParamEditValueText(entry, rows[rowIndex].kind);
				editor.unitParamStepMenuRow = none;
				return true;
			case RectNumberStepperInputAction::CycleStep:
				CycleUnitParamStep(editor, editor.unitParamEditorTab, rowIndex);
				editor.statusText = U"Unit param step set to {}"_fmt(UnitParamStep(editor, editor.unitParamEditorTab, rowIndex));
				return true;
			case RectNumberStepperInputAction::OpenStepMenu:
				editor.unitParamStepMenuRow = rowIndex;
				editor.unitParamStepMenuPos = Cursor::PosF();
				return true;
			case RectNumberStepperInputAction::Decrement:
			{
				const double step = ApplyTemporaryStepModifier(UnitParamStep(editor, editor.unitParamEditorTab, rowIndex));
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return AdjustUnitParamValue(selected, rows[rowIndex].kind, -step);
				});
				return true;
			}
			case RectNumberStepperInputAction::Increment:
			{
				const double step = ApplyTemporaryStepModifier(UnitParamStep(editor, editor.unitParamEditorTab, rowIndex));
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return AdjustUnitParamValue(selected, rows[rowIndex].kind, step);
				});
				return true;
			}
			default:
				break;
			}

			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorUnitParamRowButtonRect(row, buttonIndex);
				if (!buttonRect.leftClicked())
				{
					continue;
				}

				if (buttonIndex == 2)
				{
					editor.statusText = rows[rowIndex].helpText;
					return true;
				}

				if (buttonIndex == 0)
				{
					MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
					{
						return SetUnitParamValueIfChanged(selected, rows[rowIndex].kind, UnitParamResetValue(rows[rowIndex].kind));
					});
				}
				else if (buttonIndex == 1 && UnitParamHasSpecialAction(rows[rowIndex].kind))
				{
					MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
					{
						return ApplyUnitParamSpecialAction(selected, rows[rowIndex].kind);
					});
					editor.statusText = U"MOVE: Use SPD (move=0)";
				}
				else if (buttonIndex == 1)
				{
					editor.statusText = U"U action はこの項目では未割当です";
				}

				return true;
			}
		}

		return false;
	}
}
