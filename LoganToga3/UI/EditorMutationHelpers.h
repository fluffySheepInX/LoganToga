#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "../Data/DefinitionStores.h"
# include "../Data/UnitCatalog.h"

namespace LT3
{
		inline void CommitUnitCatalogChanges(MapEditorState& editor, UnitCatalog& catalog)
		{
			SaveUnitCatalogToml(catalog, editor.statusText);
			editor.unitCatalogDirty = true;
		}

	template <class Mutator, class Commit>
	inline bool ApplyEditorMutation(Mutator&& mutator, Commit&& commit)
	{
		if (!mutator())
		{
			return false;
		}

		commit();
		return true;
	}

	template <class T>
	inline bool SetFieldIfChanged(T& target, const T& value)
	{
		if (target == value)
		{
			return false;
		}

		target = value;
		return true;
	}

	template <class T, class Delta, class ClampFn>
	inline bool AdjustField(T& target, Delta delta, ClampFn&& clampFn)
	{
		const T next = static_cast<T>(clampFn(target + delta));
		if (next == target)
		{
			return false;
		}

		target = next;
		return true;
	}

	inline bool ToggleField(bool& value)
	{
		value = !value;
		return true;
	}

		template <class Mutator>
		inline bool ApplyUnitCatalogMutation(MapEditorState& editor, UnitCatalog& catalog, Mutator&& mutator)
		{
			return ApplyEditorMutation([&]()
			{
				return mutator();
			}, [&]()
			{
				CommitUnitCatalogChanges(editor, catalog);
			});
		}

	template <class Mutator>
	inline bool MutateSelectedCatalogEntry(MapEditorState& editor, UnitCatalog& catalog, Mutator&& mutator)
	{
		if (!(0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size())))
		{
			return false;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		return ApplyEditorMutation([&]()
		{
			return mutator(entry);
		}, [&]()
		{
				CommitUnitCatalogChanges(editor, catalog);
		});
	}

	template <class Mutator>
	inline bool MutateSelectedAiProfile(MapEditorState& editor, DefinitionStores& defs, Mutator&& mutator)
	{
		if (!(0 <= editor.selectedAiProfileIndex && editor.selectedAiProfileIndex < static_cast<int32>(defs.aiProfiles.size())))
		{
			return false;
		}

		AiProfileDef& profile = defs.aiProfiles[editor.selectedAiProfileIndex];
		return ApplyEditorMutation([&]()
		{
			return mutator(profile);
		}, [&]()
		{
			editor.aiProfilesDirty = true;
		});
	}
}
