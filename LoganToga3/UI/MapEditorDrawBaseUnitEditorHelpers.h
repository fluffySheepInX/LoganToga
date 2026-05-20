#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseAssetHelpers.h"
# include "MapEditorResourceDraw.h"
# include "BuildingEditor.h"

namespace LT3
{
		inline bool DoesCatalogEntryMatchStoredOwnerTag(const UnitCatalogEntry& entry, const String& ownerTag)
		{
			return !ownerTag.isEmpty()
				&& !entry.unit_id.isEmpty()
				&& entry.unit_id.lowercased() == ownerTag.lowercased();
		}

	inline bool DoesCatalogEntryMatchOwnerTag(const UnitCatalogEntry& entry, const String& ownerTag)
	{
		if (ownerTag.isEmpty())
		{
			return false;
		}

		const String ownerLower = ownerTag.lowercased();
		return (!entry.unit_id.isEmpty() && entry.unit_id.lowercased() == ownerLower)
			|| (!entry.building_category.isEmpty() && entry.building_category.lowercased() == ownerLower)
			|| (!entry.unit_family.isEmpty() && entry.unit_family.lowercased() == ownerLower);
	}

	inline bool IsCatalogEntryBoundToAction(const UnitCatalogEntry& entry, const BuildActionDef& action)
	{
		if (!action.ownerTags.isEmpty())
		{
			for (const auto& ownerTag : action.ownerTags)
			{
					if (DoesCatalogEntryMatchStoredOwnerTag(entry, ownerTag))
				{
					return true;
				}
			}
			return false;
		}

			return DoesCatalogEntryMatchStoredOwnerTag(entry, action.ownerTag);
	}
}
