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
				&& !entry.tag.isEmpty()
				&& entry.tag.lowercased() == ownerTag.lowercased();
		}

	inline bool DoesCatalogEntryMatchOwnerTag(const UnitCatalogEntry& entry, const String& ownerTag)
	{
		if (ownerTag.isEmpty())
		{
			return false;
		}

		const String ownerLower = ownerTag.lowercased();
		return (!entry.tag.isEmpty() && entry.tag.lowercased() == ownerLower)
			|| (!entry.classBuild.isEmpty() && entry.classBuild.lowercased() == ownerLower)
			|| (!entry.classTag.isEmpty() && entry.classTag.lowercased() == ownerLower);
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
