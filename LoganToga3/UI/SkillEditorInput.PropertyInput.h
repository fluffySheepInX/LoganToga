#pragma once
# include "SkillEditorInput.AssetInput.h"

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

		return false;
	}
}
