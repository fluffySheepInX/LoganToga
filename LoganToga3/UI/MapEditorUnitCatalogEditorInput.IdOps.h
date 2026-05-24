#pragma once
# include "MapEditorUnitCatalogEditorInputCommon.h"

namespace LT3
{
	inline bool NormalizeVisibleUnitCatalogIds(MapEditorState& editor, UnitCatalog& catalog)
	{
		for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
		{
			catalog.entries[i].id = i;
		}

		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
		editor.statusText = U"Renumbered unit ids from 0 in visible order";
		return true;
	}

	inline bool StoreUnitCatalogIdsToUnitIds(MapEditorState& editor, UnitCatalog& catalog)
	{
		for (auto& entry : catalog.entries)
		{
			entry.unit_id = ToString(entry.id);
		}

		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
		editor.statusText = U"Stored ids into unit_id";
		return true;
	}
}
