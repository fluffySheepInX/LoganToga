#pragma once
# include "MapEditorDrawBaseCommandEditorHelpers.h"
# include "MapEditorDrawBaseCommandEditorInspect.h"
# include "MapEditorDrawBaseCommandEditorListPanels.h"

namespace LT3
{
	// Command Editor 全体を描画する。
	inline void DrawCommandEditor(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showCommandEditor)
		{
			return;
		}

		const RectF panel = EditorCommandPanelRect();
		const RectF commandViewport = EditorCommandListViewportRect();
		const RectF unitViewport = EditorCommandUnitViewportRect();
		const RectF inspectTopViewport = EditorCommandInspectTopViewportRect();
		const RectF inspectBottomPanel = EditorCommandInspectBottomPanelRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Command Editor").draw(18, panel.x + 20.0, panel.y + 18.0, Palette::White);
		uiFont(U"左:Command  右:実行可能Unit").draw(12, panel.x + 220.0, panel.y + 22.0, Palette::Lightgray);

		const int32 commandCount = static_cast<int32>(defs.buildActions.size());
		if (commandCount <= 0)
		{
			uiFont(U"No commands").draw(14, panel.x + 20.0, panel.y + 52.0, Palette::Orange);
			return;
		}

		editor.selectedCommandActionIndex = Clamp(editor.selectedCommandActionIndex, 0, commandCount - 1);

		DrawCommandEditorCommandList(editor, defs, commandViewport, uiFont);
		DrawCommandEditorContextMenu(editor, uiFont);

		const BuildActionDef& selectedAction = defs.buildActions[editor.selectedCommandActionIndex];
		const Array<String> selectedSpawnTags = CommandEditorSpawnTags(selectedAction);
		const bool missingSpawnForUnitResult = (selectedAction.resultType == BuildActionResultType::Unit) && selectedSpawnTags.isEmpty();
		const bool allowMultipleSpawns = [&]() -> bool
		{
			for (const auto& entry : catalog.entries)
			{
				if (CommandEditorActionOwnedByEntry(selectedAction, entry) && IsCommandEditorFacilityUnit(entry))
				{
					return true;
				}
			}
			return false;
		}();
		DrawCommandEditorModeTabs(editor, uiFont);

		if (editor.commandEditorMode == 2)
		{
			DrawCommandEditorInspectPanel(editor, selectedAction, selectedSpawnTags, missingSpawnForUnitResult, inspectTopViewport, inspectBottomPanel, uiFont);
		}
		else
		{
			DrawCommandEditorUnitPanel(editor, catalog, defs, selectedAction, selectedSpawnTags, missingSpawnForUnitResult, allowMultipleSpawns, unitViewport, uiFont);
		}

		DrawCommandEditorFooter(editor, panel, uiFont);
	}
}
