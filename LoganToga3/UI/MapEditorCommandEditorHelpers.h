#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "BuildingEditorCommon.h"
# include "../Data/Loaders/BuildActionDefLoader.h"

namespace LT3
{
	inline bool NormalizeCommandIdsContainsOwnerTag(const Array<String>& tags, StringView target)
	{
		for (const auto& tag : tags)
		{
			if (!tag.isEmpty() && !target.isEmpty() && tag.lowercased() == String{ target }.lowercased())
			{
				return true;
			}
		}

		return false;
	}

	inline Array<String> NormalizeCommandIdsOwnerTags(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& tag : source)
		{
			if (tag.isEmpty())
			{
				continue;
			}

			if (!NormalizeCommandIdsContainsOwnerTag(normalized, tag))
			{
				normalized << tag;
			}
		}

		return normalized;
	}

	inline Array<String> NormalizeSpawnTagsForEditor(const Array<String>& source)
	{
		return NormalizeCommandIdsOwnerTags(source);
	}

	inline String BuildNormalizedCommandId(int32 serial)
	{
		return U"cmd_{:03d}"_fmt(serial);
	}

	inline bool HasBuildActionId(const Array<BuildActionDef>& actions, StringView id, Optional<size_t> ignoredIndex = none)
	{
		for (size_t actionIndex = 0; actionIndex < actions.size(); ++actionIndex)
		{
			if (ignoredIndex && *ignoredIndex == actionIndex)
			{
				continue;
			}

			if (!actions[actionIndex].id.isEmpty() && actions[actionIndex].id == id)
			{
				return true;
			}
		}

		return false;
	}

	inline bool IsFacilityUnitCatalogEntry(const UnitCatalogEntry& entry)
	{
		const String kind = entry.kind.lowercased();
		const String buildingCategory = entry.building_category.lowercased();
		return (kind == U"building") || (buildingCategory == U"home");
	}

	inline bool ActionOwnerIncludesFacilityUnit(const BuildActionDef& action, const UnitCatalog& catalog)
	{
		const Array<String> ownerTags = action.ownerTags.isEmpty()
			? Array<String>{ action.ownerTag }
			: NormalizeCommandIdsOwnerTags(action.ownerTags);

		for (const auto& ownerTag : ownerTags)
		{
			for (const auto& entry : catalog.entries)
			{
				if (EqualsIgnoreCaseOwnerTag(entry.unit_id, ownerTag) && IsFacilityUnitCatalogEntry(entry))
				{
					return true;
				}
			}
		}

		return false;
	}

	inline void RefreshActionSpawnSelection(BuildActionDef& action, const DefinitionStores& defs)
	{
		action.spawnTags = NormalizeSpawnTagsForEditor(action.spawnTags);
		action.spawnUnits.clear();
		for (const auto& spawnTag : action.spawnTags)
		{
			if (defs.unitByTag.contains(spawnTag))
			{
				action.spawnUnits << defs.unitByTag.at(spawnTag);
			}
		}

		action.spawnTag = action.spawnTags.isEmpty() ? U"" : action.spawnTags.front();
		action.resultTag = action.spawnTag;
		action.spawnUnit = action.spawnUnits.isEmpty() ? InvalidUnitDefId : action.spawnUnits.front();
	}

	inline void ClearActionSpawnSelection(BuildActionDef& action)
	{
		action.spawnTag.clear();
		action.resultTag.clear();
		action.spawnTags.clear();
		action.spawnUnits.clear();
		action.spawnUnit = InvalidUnitDefId;
	}

	// Carrier の格納数 UI を表示する条件を返す。
	inline bool IsCommandCarrierCapacityVisible(const BuildActionDef& action)
	{
		return (action.resultType == BuildActionResultType::Carrier)
			&& (action.carrierAction == CarrierActionKind::Store);
	}

	inline bool NormalizeCommandIdsForOwnerTags(MapEditorState& editor, DefinitionStores& defs, const Array<String>& targetOwnerTags)
	{
		Array<String> normalizedOwnerTags = NormalizeCommandIdsOwnerTags(targetOwnerTags);
		if (normalizedOwnerTags.isEmpty())
		{
			return false;
		}

		int32 serial = 0;
		int32 updatedCount = 0;
		for (size_t actionIndex = 0; actionIndex < defs.buildActions.size(); ++actionIndex)
		{
			auto& action = defs.buildActions[actionIndex];
			Array<String> actionOwnerTags = NormalizeCommandIdsOwnerTags(action.ownerTags);
			if (actionOwnerTags.isEmpty() && !action.ownerTag.isEmpty())
			{
				actionOwnerTags << action.ownerTag;
			}

			bool ownerMatched = false;
			for (const auto& ownerTag : actionOwnerTags)
			{
				if (NormalizeCommandIdsContainsOwnerTag(normalizedOwnerTags, ownerTag))
				{
					ownerMatched = true;
					break;
				}
			}

			if (!ownerMatched)
			{
				continue;
			}

			action.ownerTags = actionOwnerTags;
			action.ownerTag = actionOwnerTags.isEmpty() ? U"" : actionOwnerTags.front();

			String nextId;
			do
			{
				nextId = BuildNormalizedCommandId(serial++);
			}
			while (HasBuildActionId(defs.buildActions, nextId, actionIndex));

			action.id = nextId;
			action.tag = action.id;
			++updatedCount;
		}

		if (updatedCount <= 0)
		{
			return false;
		}

		editor.commandBindingsDirty = true;
		editor.statusText = U"Normalized command ids: {}"_fmt(updatedCount);
		return true;
	}

	inline bool EqualsOwnerTagIgnoreCase(StringView a, StringView b)
	{
		return !a.isEmpty() && !b.isEmpty() && String{ a }.lowercased() == String{ b }.lowercased();
	}

	inline bool ContainsOwnerTag(const Array<String>& tags, StringView target)
	{
		for (const auto& tag : tags)
		{
			if (EqualsOwnerTagIgnoreCase(tag, target))
			{
				return true;
			}
		}

		return false;
	}

	inline Array<String> NormalizeOwnerTagsForEditor(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& tag : source)
		{
			if (tag.isEmpty())
			{
				continue;
			}

			if (!ContainsOwnerTag(normalized, tag))
			{
				normalized << tag;
			}
		}

		return normalized;
	}

	inline bool IsCommandSelectableImageFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"png" || extension == U"gif";
	}

	inline Array<String> NormalizeCommandIconLayers(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& icon : source)
		{
			if (icon.isEmpty())
			{
				continue;
			}

			normalized << icon;
		}

		return normalized;
	}
}
