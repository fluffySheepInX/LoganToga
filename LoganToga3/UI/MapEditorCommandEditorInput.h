#pragma once
# include "MapEditorCommandEditorInput.DetailInput.h"

namespace LT3
{
	inline bool ProcessCommandEditorInput(MapEditorState& editor, UnitCatalog& catalog, DefinitionStores& defs)
	{
		if (!editor.showCommandEditor)
		{
			return false;
		}

		if (ProcessCommandRenameInput(editor, defs))
		{
			return true;
		}

		if (ProcessCommandContextMenuInput(editor, defs))
		{
			return true;
		}

		bool consumed = false;
		const RectF panel = EditorCommandPanelRect();
		const RectF commandViewport = EditorCommandListViewportRect();
		const RectF unitViewport = EditorCommandUnitViewportRect();
		const Array<RectF> modeRects = { EditorCommandModeTabRect(0), EditorCommandModeTabRect(1), EditorCommandModeTabRect(2) };
		const RectF normalizeIdsRect = EditorCommandNormalizeIdsRect();
		const RectF saveRect = EditorCommandSaveRect();
		const RectF closeRect = EditorCommandCloseRect();
		const RectF inspectTopViewport = EditorCommandInspectTopViewportRect();

		const int32 commandCount = static_cast<int32>(defs.buildActions.size());
		if (commandCount > 0)
		{
			editor.selectedCommandActionIndex = Clamp(editor.selectedCommandActionIndex, 0, commandCount - 1);
		}
		else
		{
			editor.selectedCommandActionIndex = -1;
		}

		for (int32 modeIndex = 0; modeIndex < static_cast<int32>(modeRects.size()); ++modeIndex)
		{
			if (modeRects[modeIndex].leftClicked())
			{
				editor.commandEditorMode = modeIndex;
				consumed = true;
			}
		}

		if (normalizeIdsRect.leftClicked())
		{
			if (0 <= editor.selectedCommandActionIndex && editor.selectedCommandActionIndex < commandCount)
			{
				const BuildActionDef& selectedAction = defs.buildActions[editor.selectedCommandActionIndex];
				if (NormalizeCommandIdsForOwnerTags(editor, defs, selectedAction.ownerTags.isEmpty()
					? Array<String>{ selectedAction.ownerTag }
					: selectedAction.ownerTags))
				{
					consumed = true;
				}
			}
			else
			{
				editor.statusText = U"Select a command before normalizing ids";
				consumed = true;
			}
		}

		if (saveRect.leftClicked())
		{
			SaveBuildActionDefinitions(defs, editor.statusText);
			editor.commandBindingsDirty = false;
			consumed = true;
		}
		if (closeRect.leftClicked())
		{
			editor.showCommandEditor = false;
			consumed = true;
		}

		if (commandViewport.mouseOver())
		{
			const double contentHeight = Max(0.0, commandCount * 66.0);
			const double maxScroll = Max(0.0, contentHeight - commandViewport.h);
			editor.commandListScroll = Clamp(editor.commandListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
			consumed = true;
		}

		const double commandViewportBottom = commandViewport.y + commandViewport.h;
		for (int32 i = 0; i < commandCount; ++i)
		{
			const RectF row = EditorCommandRowRect(commandViewport, i, editor.commandListScroll);
			if (!((commandViewport.y <= row.y) && ((row.y + row.h) <= commandViewportBottom)))
			{
				continue;
			}

			if (row.leftClicked())
			{
				editor.selectedCommandActionIndex = i;
				consumed = true;
				break;
			}
			if (row.rightClicked())
			{
				editor.commandContextMenuTargetIndex = i;
				editor.commandContextMenuPos = Cursor::PosF();
				consumed = true;
				break;
			}
		}

		if (editor.selectedCommandActionIndex >= 0 && editor.selectedCommandActionIndex < commandCount)
		{
			BuildActionDef& action = defs.buildActions[editor.selectedCommandActionIndex];
			if (editor.commandEditorMode == 2)
			{
				if (ProcessCommandInspectInput(editor, action, inspectTopViewport))
				{
					return true;
				}
			}

			if (ProcessCommandUnitSelectionInput(editor, catalog, defs, action, unitViewport))
			{
				return true;
			}
		}

		if (panel.mouseOver())
		{
			consumed = true;
		}

		return consumed;
	}
}
