#pragma once
# include "SkillEditorState.Values.h"

namespace LT3
{
	inline const Array<String>* FindSkillIconWarnings(const DefinitionStores& defs, StringView skillTag)
	{
		if (defs.skillIconWarningsByTag.contains(String{ skillTag }))
		{
			return &defs.skillIconWarningsByTag.at(String{ skillTag });
		}

		return nullptr;
	}

	inline void SaveSkillEditorDefinitions(MapEditorState& editor, const DefinitionStores& defs)
	{
		String status;
		if (SaveSkillDefinitionsToml(defs.skills, &status))
		{
			editor.skillDefsDirty = true;
		}
		editor.statusText = status;
	}

	inline String NormalizeSkillTagForEditor(String text)
	{
		text = text.lowercased();
		text.remove_if([](char32 ch)
		{
			return ch == U'\n' || ch == U'\r' || ch == U'\t'
				|| ch == U'"' || ch == U'\\' || ch < U' ';
		});
		for (auto& ch : text)
		{
			if (ch == U' ')
			{
				ch = U'_';
			}
		}
		return text;
	}

	inline void ReplaceSkillTagReferences(UnitCatalog& catalog, StringView beforeTag, StringView afterTag)
	{
		for (auto& entry : catalog.entries)
		{
			for (auto& skillTag : entry.skills)
			{
				if (skillTag == beforeTag)
				{
					skillTag = String{ afterTag };
				}
			}
		}
	}

	inline bool RenameSkillTag(MapEditorState& editor, DefinitionStores& defs, UnitCatalog& catalog, int32 skillIndex, StringView requestedTag)
	{
		if (skillIndex < 0 || skillIndex >= static_cast<int32>(defs.skills.size()))
		{
			return false;
		}

		const String normalized = NormalizeSkillTagForEditor(String{ requestedTag });
		if (normalized.isEmpty())
		{
			editor.statusText = U"Skill tag cannot be empty";
			return false;
		}

		const String oldTag = defs.skills[skillIndex].tag;
		if (oldTag == normalized)
		{
			return true;
		}
		if (const auto it = defs.skillByTag.find(normalized); it != defs.skillByTag.end() && it->second != skillIndex)
		{
			editor.statusText = U"Skill tag already exists: {}"_fmt(normalized);
			return false;
		}

		defs.skills[skillIndex].tag = normalized;
		defs.skillByTag.erase(oldTag);
		defs.skillByTag[normalized] = skillIndex;
		if (const auto warningIt = defs.skillIconWarningsByTag.find(oldTag); warningIt != defs.skillIconWarningsByTag.end())
		{
			defs.skillIconWarningsByTag[normalized] = warningIt->second;
			defs.skillIconWarningsByTag.erase(warningIt);
		}

		ReplaceSkillTagReferences(catalog, oldTag, normalized);
		SaveSkillEditorDefinitions(editor, defs);
		SaveUnitCatalogToml(catalog, editor.statusText);
		editor.unitCatalogDirty = true;
		editor.statusText = U"Renamed skill tag: {} -> {}"_fmt(oldTag, normalized);
		return true;
	}

	inline bool RenameSkillName(MapEditorState& editor, DefinitionStores& defs, int32 skillIndex, StringView requestedName)
	{
		if (skillIndex < 0 || skillIndex >= static_cast<int32>(defs.skills.size()))
		{
			return false;
		}

		String normalized = String{ requestedName };
		normalized.remove_if([](char32 ch)
		{
			return ch == U'\n' || ch == U'\r' || ch == U'\t' || ch < U' ';
		});
		if (normalized.isEmpty())
		{
			editor.statusText = U"Skill name cannot be empty";
			return false;
		}
		if (defs.skills[skillIndex].name == normalized)
		{
			return true;
		}

		defs.skills[skillIndex].name = normalized;
		SaveSkillEditorDefinitions(editor, defs);
		editor.statusText = U"Renamed skill name: {}"_fmt(normalized);
		return true;
	}

	inline String MakeDuplicatedSkillTag(const DefinitionStores& defs, StringView sourceTag)
	{
		const String baseTag = sourceTag.isEmpty() ? U"skill" : String{ sourceTag };
		String candidate = U"{}_copy"_fmt(baseTag).lowercased();
		if (!defs.skillByTag.contains(candidate))
		{
			return candidate;
		}

		for (int32 index = 2; index < 10000; ++index)
		{
			candidate = U"{}_copy{}"_fmt(baseTag, index).lowercased();
			if (!defs.skillByTag.contains(candidate))
			{
				return candidate;
			}
		}

		return U"{}_copy_final"_fmt(baseTag).lowercased();
	}

	inline bool DuplicateSkillDefinition(MapEditorState& editor, DefinitionStores& defs, int32 sourceIndex)
	{
		if (sourceIndex < 0 || sourceIndex >= static_cast<int32>(defs.skills.size()))
		{
			return false;
		}

		SkillDef copy = defs.skills[sourceIndex];
		copy.tag = MakeDuplicatedSkillTag(defs, copy.tag);
		copy.name = copy.name.isEmpty() ? copy.tag : U"{} Copy"_fmt(copy.name);
		const SkillDefId duplicatedId = defs.addSkill(copy);
		if (const Array<String>* warnings = FindSkillIconWarnings(defs, defs.skills[sourceIndex].tag))
		{
			defs.skillIconWarningsByTag[copy.tag] = *warnings;
		}
		editor.selectedSkillIndex = duplicatedId;
		SaveSkillEditorDefinitions(editor, defs);
		editor.statusText = U"Duplicated skill: {} -> {}"_fmt(defs.skills[sourceIndex].tag, copy.tag);
		return true;
	}
}
