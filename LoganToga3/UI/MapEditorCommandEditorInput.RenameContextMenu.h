#pragma once
# include "MapEditorCommandEditorInput.ImageOps.h"

namespace LT3
{
	inline bool ProcessCommandRenameInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (!editor.commandRenameTargetIndex)
		{
			return false;
		}

		TextInput::UpdateText(editor.commandRenameEditText);
		editor.commandRenameEditText.remove_if([](char32 c)
		{
			return c == U'\n' || c == U'\r' || c == U'\t'
				|| c == U'"' || c == U'\\' || c < U' ';
		});
		if ((KeyControl | KeyCommand).pressed() && KeyV.down())
		{
			String clip;
			if (Clipboard::GetText(clip) && !clip.isEmpty())
			{
				clip.remove_if([](char32 c)
				{
					return c == U'\n' || c == U'\r' || c == U'\t'
						|| c == U'"' || c == U'\\' || c < U' ';
				});
				editor.commandRenameEditText += clip;
			}
		}

		if (KeyEnter.down())
		{
			const int32 idx = *editor.commandRenameTargetIndex;
			if (0 <= idx && idx < static_cast<int32>(defs.buildActions.size()))
			{
				defs.buildActions[idx].name = editor.commandRenameEditText.isEmpty()
					? defs.buildActions[idx].name
					: editor.commandRenameEditText;
				editor.commandBindingsDirty = true;
				editor.statusText = U"Renamed command: {}"_fmt(defs.buildActions[idx].id);
			}
			editor.commandRenameTargetIndex = none;
			editor.commandRenameEditText = U"";
			editor.commandRenameIsDuplicate = false;
		}
		else if (KeyEscape.down())
		{
			if (editor.commandRenameIsDuplicate)
			{
				const int32 idx = *editor.commandRenameTargetIndex;
				if (0 <= idx && idx < static_cast<int32>(defs.buildActions.size()))
				{
					defs.buildActions.remove_at(idx);
					if (editor.selectedCommandActionIndex >= static_cast<int32>(defs.buildActions.size()))
					{
						editor.selectedCommandActionIndex = static_cast<int32>(defs.buildActions.size()) - 1;
					}
					editor.commandBindingsDirty = true;
				}
			}
			editor.commandRenameTargetIndex = none;
			editor.commandRenameEditText = U"";
			editor.commandRenameIsDuplicate = false;
		}

		return true;
	}

	inline bool ProcessCommandContextMenuInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (!editor.commandContextMenuTargetIndex)
		{
			return false;
		}

		const RectF menuRect = EditorCommandContextMenuRect(editor.commandContextMenuPos);
		const RectF dupItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 0);
		if (dupItem.leftClicked())
		{
			const int32 srcIdx = *editor.commandContextMenuTargetIndex;
			editor.commandContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
			{
				BuildActionDef duplicated = defs.buildActions[srcIdx];
				duplicated.id = duplicated.id + U"_copy";
				duplicated.tag = U"{}:{}"_fmt(duplicated.ownerTag, duplicated.id);
				duplicated.name += U" copy";
				defs.buildActions << duplicated;

				const int32 newIdx = static_cast<int32>(defs.buildActions.size()) - 1;
				editor.selectedCommandActionIndex = newIdx;
				const double contentHeight = defs.buildActions.size() * 66.0;
				const double maxScroll = Max(0.0, contentHeight - EditorCommandListViewportRect().h);
				editor.commandListScroll = maxScroll;

				editor.commandRenameTargetIndex = newIdx;
				editor.commandRenameEditText = duplicated.name;
				editor.commandRenameIsDuplicate = true;
				editor.commandBindingsDirty = true;
			}
			return true;
		}

		const RectF renameItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 1);
		if (renameItem.leftClicked())
		{
			const int32 srcIdx = *editor.commandContextMenuTargetIndex;
			editor.commandContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
			{
				editor.selectedCommandActionIndex = srcIdx;
				editor.commandRenameTargetIndex = srcIdx;
				editor.commandRenameEditText = defs.buildActions[srcIdx].name;
				editor.commandRenameIsDuplicate = false;
			}
			return true;
		}

		const RectF imageItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 2);
		if (imageItem.leftClicked())
		{
			const int32 srcIdx = *editor.commandContextMenuTargetIndex;
			editor.commandContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
			{
				editor.selectedCommandActionIndex = srcIdx;
				return ChangeSelectedCommandImageFromDialog(editor, defs, srcIdx);
			}
			return true;
		}

		const RectF descriptionItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 3);
		if (descriptionItem.leftClicked())
		{
			const int32 srcIdx = *editor.commandContextMenuTargetIndex;
			editor.commandContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
			{
				editor.selectedCommandActionIndex = srcIdx;
				const BuildActionDef& action = defs.buildActions[srcIdx];
				OpenDescriptionEditor(editor, DescriptionEditorTargetKind::Command, srcIdx, U"Command: {}"_fmt(action.name.isEmpty() ? action.id : action.name), action.description);
			}
			return true;
		}
		else if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			editor.commandContextMenuTargetIndex = none;
			return false;
		}

		return true;
	}
}
