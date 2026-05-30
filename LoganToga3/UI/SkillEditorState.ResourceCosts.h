#pragma once
# include "SkillEditorState.DefinitionOps.h"

namespace LT3
{
	inline const Array<double>& SkillEditorResourceCostStepOptions()
	{
		static const Array<double> steps = { 1.0, 5.0, 10.0, 25.0, 50.0 };
		return steps;
	}

	inline void EnsureSkillEditorResourceCostSteps(MapEditorState& editor, const SkillDef& skill)
	{
		if (editor.skillResourceCostSteps.size() == skill.resourceCosts.size())
		{
			return;
		}

		editor.skillResourceCostSteps.assign(skill.resourceCosts.size(), 1.0);
	}

	inline double SkillEditorResourceCostStep(const MapEditorState& editor, int32 index)
	{
		if (0 <= index && index < static_cast<int32>(editor.skillResourceCostSteps.size()))
		{
			return editor.skillResourceCostSteps[index];
		}
		return 1.0;
	}

	inline void SetSkillEditorResourceCostStep(MapEditorState& editor, int32 index, double step)
	{
		if (0 <= index && index < static_cast<int32>(editor.skillResourceCostSteps.size()))
		{
			editor.skillResourceCostSteps[index] = step;
		}
	}

	inline void CycleSkillEditorResourceCostStep(MapEditorState& editor, int32 index)
	{
		const Array<double>& steps = SkillEditorResourceCostStepOptions();
		SetSkillEditorResourceCostStep(editor, index, NextCycledStepValue(steps, SkillEditorResourceCostStep(editor, index)));
	}

	inline bool ChangeSkillResourceCostAmount(SkillDef& skill, int32 index, int32 delta)
	{
		if (index < 0 || index >= static_cast<int32>(skill.resourceCosts.size()))
		{
			return false;
		}

		const int32 next = Max(1, skill.resourceCosts[index].amount + delta);
		if (skill.resourceCosts[index].amount == next)
		{
			return false;
		}

		skill.resourceCosts[index].amount = next;
		return true;
	}

	inline bool TryCommitSkillResourceCostAmountText(SkillDef& skill, int32 index, const String& text)
	{
		if (index < 0 || index >= static_cast<int32>(skill.resourceCosts.size()) || text.isEmpty())
		{
			return false;
		}

		if (const Optional<int32> value = ParseOpt<int32>(text))
		{
			const int32 bounded = Max(1, *value);
			if (skill.resourceCosts[index].amount == bounded)
			{
				return false;
			}

			skill.resourceCosts[index].amount = bounded;
			return true;
		}

		return false;
	}
}
