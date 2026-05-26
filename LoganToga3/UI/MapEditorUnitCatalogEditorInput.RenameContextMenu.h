#pragma once
# include "MapEditorUnitCatalogEditorInput.IdOps.h"

namespace LT3
{
	inline bool ProcessUnitCatalogRenameInput(MapEditorState& editor, UnitCatalog& catalog)
	{
		if (!editor.unitRenameTargetIndex)
		{
			return false;
		}

		TextInput::UpdateText(editor.unitRenameEditText);
		editor.unitRenameEditText.remove_if([](char32 c)
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
				editor.unitRenameEditText += clip;
			}
		}
		if (KeyEnter.down())
		{
			const int32 idx = *editor.unitRenameTargetIndex;
			if (0 <= idx && idx < static_cast<int32>(catalog.entries.size()))
			{
				ApplyUnitCatalogMutation(editor, catalog, [&]()
				{
					catalog.entries[idx].name = editor.unitRenameEditText.isEmpty()
						? catalog.entries[idx].name
						: editor.unitRenameEditText;
					return true;
				});
				editor.statusText = U"Renamed unit: {}"_fmt(catalog.entries[idx].unit_id);
			}
			editor.unitRenameTargetIndex = none;
			editor.unitRenameEditText = U"";
		}
		else if (KeyEscape.down())
		{
			if (editor.unitRenameIsDuplicate)
			{
				const int32 idx = *editor.unitRenameTargetIndex;
				if (0 <= idx && idx < static_cast<int32>(catalog.entries.size()))
				{
					ApplyUnitCatalogMutation(editor, catalog, [&]()
					{
						catalog.entries.remove_at(idx);
						return true;
					});
					if (editor.selectedUnitCatalogIndex >= static_cast<int32>(catalog.entries.size()))
					{
						editor.selectedUnitCatalogIndex = static_cast<int32>(catalog.entries.size()) - 1;
					}
				}
			}
			editor.unitRenameTargetIndex = none;
			editor.unitRenameEditText = U"";
			editor.unitRenameIsDuplicate = false;
		}

		return true;
	}

	inline bool ProcessUnitCatalogContextMenuInput(MapEditorState& editor, UnitCatalog& catalog)
	{
		if (!editor.unitContextMenuTargetIndex)
		{
			return false;
		}

		const RectF menuRect = EditorUnitContextMenuRect(editor.unitContextMenuPos);
		const RectF dupItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 0);
		if (dupItem.leftClicked())
		{
			const int32 srcIdx = *editor.unitContextMenuTargetIndex;
			editor.unitContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(catalog.entries.size()))
			{
				UnitCatalogEntry newEntry = catalog.entries[srcIdx];
				String baseUnitId = newEntry.unit_id;
				String candidateUnitId = baseUnitId + U"_copy";
				int32 suffix = 2;
				while (catalog.entries.any([&](const UnitCatalogEntry& e) { return e.unit_id == candidateUnitId; }))
				{
					candidateUnitId = baseUnitId + U"_copy" + Format(suffix++);
				}
				newEntry.unit_id = candidateUnitId;
				catalog.entries << newEntry;
				const int32 newIdx = static_cast<int32>(catalog.entries.size()) - 1;
				editor.selectedUnitCatalogIndex = newIdx;
				editor.showUnitParameterEditor = true;
				const RectF viewport = EditorUnitListViewportRect();
				const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
				editor.unitListScroll = maxScroll;
				editor.unitRenameTargetIndex = newIdx;
				editor.unitRenameEditText = newEntry.name;
				editor.unitRenameIsDuplicate = true;
			}
			return true;
		}
		const RectF renameItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 1);
		if (renameItem.leftClicked())
		{
			const int32 srcIdx = *editor.unitContextMenuTargetIndex;
			editor.unitContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(catalog.entries.size()))
			{
				editor.selectedUnitCatalogIndex = srcIdx;
				editor.showUnitParameterEditor = true;
				editor.unitRenameTargetIndex = srcIdx;
				editor.unitRenameEditText = catalog.entries[srcIdx].name;
				editor.unitRenameIsDuplicate = false;
			}
			return true;
		}
		const RectF descriptionItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 2);
		if (descriptionItem.leftClicked())
		{
			const int32 srcIdx = *editor.unitContextMenuTargetIndex;
			editor.unitContextMenuTargetIndex = none;
			if (0 <= srcIdx && srcIdx < static_cast<int32>(catalog.entries.size()))
			{
				editor.selectedUnitCatalogIndex = srcIdx;
				editor.showUnitParameterEditor = true;
				const UnitCatalogEntry& entry = catalog.entries[srcIdx];
				OpenDescriptionEditor(editor, DescriptionEditorTargetKind::Unit, srcIdx, U"Unit: {}"_fmt(entry.name.isEmpty() ? entry.unit_id : entry.name), entry.description);
			}
			return true;
		}
		else if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			editor.unitContextMenuTargetIndex = none;
			return false;
		}

		return true;
	}
}
