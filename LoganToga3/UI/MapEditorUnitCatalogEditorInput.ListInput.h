#pragma once
# include "MapEditorUnitCatalogEditorInput.RenameContextMenu.h"

namespace LT3
{
	inline bool ProcessUnitCatalogListInput(MapEditorState& editor, UnitCatalog& catalog, bool& consumed)
	{
		if (editor.showUnitList && EditorUnitListPanelRect().mouseOver())
		{
			const RectF normalizeIdsRect = EditorUnitNormalizeIdsRect();
			if (normalizeIdsRect.leftClicked())
			{
				return NormalizeVisibleUnitCatalogIds(editor, catalog);
			}
			const RectF storeIdToTagRect = EditorUnitStoreIdToTagRect();
			if (storeIdToTagRect.leftClicked())
			{
				return StoreUnitCatalogIdsToUnitIds(editor, catalog);
			}

			const RectF viewport = EditorUnitListViewportRect();
			const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
			editor.unitListScroll = Clamp(editor.unitListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
			consumed = true;

			for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
			{
				const RectF row = EditorUnitListRowRect(viewport, i, editor.unitListScroll);
				if (!viewport.intersects(row))
				{
					continue;
				}

				const RectF moveUpRect = EditorUnitRowMoveUpRect(row);
				const RectF moveDownRect = EditorUnitRowMoveDownRect(row);
				if ((i > 0) && moveUpRect.leftClicked())
				{
					ApplyUnitCatalogMutation(editor, catalog, [&]()
					{
						const UnitCatalogEntry movedEntry = catalog.entries[i];
						catalog.entries[i] = catalog.entries[i - 1];
						catalog.entries[i - 1] = movedEntry;
						return true;
					});
					editor.selectedUnitCatalogIndex = i - 1;
					editor.statusText = U"Moved unit up: {}"_fmt(catalog.entries[i - 1].unit_id);
					return true;
				}
				if ((i + 1 < static_cast<int32>(catalog.entries.size())) && moveDownRect.leftClicked())
				{
					ApplyUnitCatalogMutation(editor, catalog, [&]()
					{
						const UnitCatalogEntry movedEntry = catalog.entries[i];
						catalog.entries[i] = catalog.entries[i + 1];
						catalog.entries[i + 1] = movedEntry;
						return true;
					});
					editor.selectedUnitCatalogIndex = i + 1;
					editor.statusText = U"Moved unit down: {}"_fmt(catalog.entries[i + 1].unit_id);
					return true;
				}

				const RectF previewRect = EditorUnitListPreviewRect(row);
				if (previewRect.leftClicked())
				{
					editor.selectedUnitCatalogIndex = i;
					editor.showUnitParameterEditor = true;
					if (!ChangeSelectedUnitImageFromDialog(editor, catalog))
					{
						editor.statusText = U"Editing unit: {}"_fmt(catalog.entries[i].unit_id);
					}
				}
				else if (row.leftClicked())
				{
					editor.selectedUnitCatalogIndex = i;
					editor.showUnitParameterEditor = true;
					editor.statusText = U"Editing unit: {}"_fmt(catalog.entries[i].unit_id);
				}
				else if (row.rightClicked())
				{
					editor.unitContextMenuTargetIndex = i;
					editor.unitContextMenuPos = Cursor::PosF();
				}
			}
		}
		else if (editor.showUnitList && EditorUnitNormalizeIdsRect().leftClicked())
		{
			return NormalizeVisibleUnitCatalogIds(editor, catalog);
		}
		else if (editor.showUnitList && EditorUnitStoreIdToTagRect().leftClicked())
		{
			return StoreUnitCatalogIdsToUnitIds(editor, catalog);
		}

		return false;
	}
}
