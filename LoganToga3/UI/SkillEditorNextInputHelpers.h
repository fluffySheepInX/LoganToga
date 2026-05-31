#pragma once
# include "SkillEditorInput.Common.h"

namespace LT3
{
	inline Array<int32> BuildSkillEditorNextSkillCandidateIndices(const MapEditorState& editor, const DefinitionStores& defs)
	{
		Array<int32> indices;
		if (!HasSelectedSkill(editor, defs))
		{
			return indices;
		}

		const String filter = editor.skillNextTagFilterText.lowercased();
		for (int32 i = 0; i < static_cast<int32>(defs.skills.size()); ++i)
		{
			if (i == editor.selectedSkillIndex)
			{
				continue;
			}

			const SkillDef& candidate = defs.skills[i];
			if (!filter.isEmpty() && !candidate.tag.includes(filter) && !candidate.name.lowercased().includes(filter))
			{
				continue;
			}

			indices << i;
			if (indices.size() >= 3)
			{
				break;
			}
		}

		return indices;
	}

	inline bool ApplySkillEditorNextSkillTag(MapEditorState& editor, DefinitionStores& defs, String nextTag)
	{
		nextTag = nextTag.lowercased().trimmed();
		if (nextTag.isEmpty())
		{
			const bool changed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				const bool wasChanged = !selected.nextSkillTag.isEmpty() || selected.nextSkill != InvalidSkillDefId;
				selected.nextSkillTag.clear();
				selected.nextSkill = InvalidSkillDefId;
				return wasChanged;
			});
			if (changed)
			{
				editor.statusText = U"Next cleared";
			}
			return changed;
		}

		if (!defs.skillByTag.contains(nextTag))
		{
			editor.statusText = U"Next skill not found: {}"_fmt(nextTag);
			return false;
		}

		const SkillDefId nextId = defs.skillByTag.at(nextTag);
		if (nextId == editor.selectedSkillIndex)
		{
			editor.statusText = U"Next cannot target itself";
			return false;
		}

		const bool changed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
		{
			selected.nextSkillTag = nextTag;
			selected.nextSkill = nextId;
			return true;
		});
		if (changed)
		{
			editor.statusText = U"Next set: {}"_fmt(nextTag);
		}
		return changed;
	}
}
