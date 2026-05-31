#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseUnitEditorHelpers.h"

namespace LT3
{
	// ownerTag 同士が同値か返す。
	inline bool CommandEditorOwnerTagEquals(StringView a, StringView b)
	{
		return !a.isEmpty() && !b.isEmpty() && String{ a }.lowercased() == String{ b }.lowercased();
	}

	// action の spawn tag 一覧を正規化して返す。
	inline Array<String> CommandEditorSpawnTags(const BuildActionDef& action)
	{
		Array<String> spawnTags = action.spawnTags;
		if (spawnTags.isEmpty() && !action.spawnTag.isEmpty())
		{
			spawnTags << action.spawnTag;
		}

		Array<String> normalized;
		for (const auto& spawnTag : spawnTags)
		{
			if (spawnTag.isEmpty())
			{
				continue;
			}

			bool exists = false;
			for (const auto& existing : normalized)
			{
				if (CommandEditorOwnerTagEquals(existing, spawnTag))
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				normalized << spawnTag;
			}
		}

		return normalized;
	}

	// 施設系 Unit か返す。
	inline bool IsCommandEditorFacilityUnit(const UnitCatalogEntry& entry)
	{
		const String kind = entry.kind.lowercased();
		const String buildingCategory = entry.building_category.lowercased();
		return (kind == U"building") || (buildingCategory == U"home");
	}

	// action が entry を owner として持つか返す。
	inline bool CommandEditorActionOwnedByEntry(const BuildActionDef& action, const UnitCatalogEntry& entry)
	{
		if (CommandEditorOwnerTagEquals(action.ownerTag, entry.unit_id))
		{
			return true;
		}

		for (const auto& ownerTag : action.ownerTags)
		{
			if (CommandEditorOwnerTagEquals(ownerTag, entry.unit_id))
			{
				return true;
			}
		}

		return false;
	}

	// entry が他 Unit を spawn するか返す。
	inline bool CommandEditorEntrySpawnsOtherUnits(const UnitCatalogEntry& entry, const DefinitionStores& defs)
	{
		for (const auto& action : defs.buildActions)
		{
			if (!CommandEditorActionOwnedByEntry(action, entry))
			{
				continue;
			}

			if (action.resultType != BuildActionResultType::Unit)
			{
				continue;
			}

			if (!CommandEditorSpawnTags(action).isEmpty())
			{
				return true;
			}
		}

		return false;
	}

	// action の icon path 一覧を返す。
	inline Array<FilePath> CommandEditorIconPaths(const BuildActionDef& action)
	{
		Array<FilePath> paths;
		if (!action.iconLayers.isEmpty())
		{
			for (const auto& icon : action.iconLayers)
			{
				if (icon.isEmpty())
				{
					continue;
				}

				paths << ResolveBuildIconPath(icon);
			}
		}
		else if (!action.icon.isEmpty())
		{
			paths << ResolveBuildIconPath(action.icon);
		}

		return paths;
	}

	// action のレイヤー icon を描画する。
	inline void DrawCommandEditorLayeredIcon(const BuildActionDef& action, const RectF& iconRect)
	{
		const Array<FilePath> iconPaths = CommandEditorIconPaths(action);
		for (const auto& iconPath : iconPaths)
		{
			if (!FileSystem::Exists(iconPath))
			{
				continue;
			}

			auto& cache = BuildingEditorTextureCache();
			if (!cache.contains(iconPath))
			{
				cache.emplace(iconPath, Texture{ iconPath });
			}

			const Texture& iconTexture = cache.at(iconPath);
			const double fitScale = Min((iconRect.w - 4.0) / Max(1.0, static_cast<double>(iconTexture.width())), (iconRect.h - 4.0) / Max(1.0, static_cast<double>(iconTexture.height())));
			iconTexture.scaled(Min(1.0, fitScale)).drawAt(iconRect.center());
		}
	}
}
