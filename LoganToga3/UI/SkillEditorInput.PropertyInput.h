#pragma once
# include "SkillEditorInput.AssetInput.h"
# include "SkillEditorNextInputHelpers.h"

namespace LT3
{
	/// <summary>
	/// スキル種別・挙動・フラグ入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorPropertyInput(MapEditorState& editor, DefinitionStores& defs, double scroll)
	{
		for (int32 i = 0; i < static_cast<int32>(SkillKindLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorKindButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.kind, static_cast<SkillKind>(i));
				});
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillMotionLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorMotionButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.projectileMotion, static_cast<SkillProjectileMotion>(i));
				});
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(SkillCenterLabels().size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorCenterButtonRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.projectileCenter, static_cast<SkillProjectileCenter>(i));
				});
				return true;
			}
		}

		if (HandleRectButtonClick(SkillEditorToggleButtonRect(0, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.projectileHoming);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(1, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.projectileD360);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(2, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.bom);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(3, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.bomFriendlyFire);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(4, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.allfunc);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorNextSkillButtonRect(scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				if (defs.skills.isEmpty())
				{
					return false;
				}

				int32 currentIndex = -1;
				if (!selected.nextSkillTag.isEmpty() && defs.skillByTag.contains(selected.nextSkillTag))
				{
					currentIndex = static_cast<int32>(defs.skillByTag.at(selected.nextSkillTag));
				}

				for (int32 offset = 1; offset <= static_cast<int32>(defs.skills.size()); ++offset)
				{
					const int32 nextIndex = (currentIndex + offset) % static_cast<int32>(defs.skills.size());
					if (defs.skills[nextIndex].tag != selected.tag)
					{
						selected.nextSkillTag = defs.skills[nextIndex].tag;
						selected.nextSkill = static_cast<SkillDefId>(nextIndex);
						return true;
					}
				}

				return false;
			});
			return true;
		}
		if (MouseR.down() && SkillEditorNextSkillButtonRect(scroll).mouseOver())
		{
			ApplySkillEditorNextSkillTag(editor, defs, U"");
			return true;
		}
		if (HandleRectButtonClick(SkillEditorNextSkillInputRect(scroll)))
		{
			editor.skillNextTagEditing = true;
			editor.skillNextTagEditingText = HasSelectedSkill(editor, defs) ? defs.skills[editor.selectedSkillIndex].nextSkillTag : U"";
			editor.skillNextTagFilterText = editor.skillNextTagEditingText;
			return true;
		}
		if (HandleRectButtonClick(SkillEditorNextSkillClearRect(scroll)))
		{
			editor.skillNextTagEditing = false;
			editor.skillNextTagEditingText.clear();
			editor.skillNextTagFilterText.clear();
			ApplySkillEditorNextSkillTag(editor, defs, U"");
			return true;
		}

		const Array<int32> nextCandidates = BuildSkillEditorNextSkillCandidateIndices(editor, defs);
		for (int32 i = 0; i < static_cast<int32>(nextCandidates.size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorNextSkillCandidateRect(i, scroll)))
			{
				const SkillDef& candidate = defs.skills[nextCandidates[i]];
				editor.skillNextTagEditing = false;
				editor.skillNextTagEditingText = candidate.tag;
				editor.skillNextTagFilterText = candidate.tag;
				ApplySkillEditorNextSkillTag(editor, defs, candidate.tag);
				return true;
			}
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(5, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.nextLast);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(6, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.jointSkill);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(7, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.sendTarget);
			});
			return true;
		}
		if (HandleRectButtonClick(SkillEditorToggleButtonRect(8, scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				return ToggleField(selected.sendImageDegree);
			});
			return true;
		}

		return false;
	}
}
