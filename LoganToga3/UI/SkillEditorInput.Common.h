#pragma once
# include "EditorMutationHelpers.h"
# include "SkillEditorCommon.h"

namespace LT3
{
	/// <summary>
	/// 選択中スキルへの変更を保存付きで適用します。
	/// </summary>
	template <class Mutator>
	inline bool MutateSelectedSkillDefinition(MapEditorState& editor, DefinitionStores& defs, Mutator&& mutator)
	{
		if (!HasSelectedSkill(editor, defs))
		{
			return false;
		}

		SkillDef& skill = defs.skills[editor.selectedSkillIndex];
		return ApplyEditorMutation([&]()
		{
			return mutator(skill);
		}, [&]()
		{
			SaveSkillEditorDefinitions(editor, defs);
		});
	}

	/// <summary>
	/// 数値テキスト入力中のスキル値を確定します。
	/// </summary>
	inline bool CommitSelectedSkillEditorValueText(MapEditorState& editor, DefinitionStores& defs, int32 row, const String& text)
	{
		const bool committed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
		{
			return TryCommitSkillEditorValueText(selected, row, text);
		});

		if (!committed)
		{
			editor.statusText = U"Invalid skill value: {}"_fmt(text);
		}

		return committed;
	}

	/// <summary>
	/// スキル値ステッパーの入力を処理します。
	/// </summary>
	inline bool HandleSkillEditorValueStepperAction(MapEditorState& editor, DefinitionStores& defs, int32 row, RectNumberStepperRects rects)
	{
		switch (DetectRectNumberStepperInput(rects))
		{
		case RectNumberStepperInputAction::StartValueEdit:
			editor.skillValueEditingRow = row;
			editor.skillValueEditingText = U"{}"_fmt(GetSkillEditorValue(defs.skills[editor.selectedSkillIndex], row));
			editor.skillValueStepMenuRow = none;
			return true;
		case RectNumberStepperInputAction::CycleStep:
			CycleSkillEditorValueStep(editor, row);
			editor.statusText = U"Skill value step set to {}"_fmt(SkillEditorValueStep(editor, row));
			return true;
		case RectNumberStepperInputAction::OpenStepMenu:
			editor.skillValueStepMenuRow = row;
			editor.skillValueStepMenuPos = Cursor::PosF();
			return true;
		case RectNumberStepperInputAction::Decrement:
		{
			const double step = ApplyTemporaryStepModifier(SkillEditorValueStep(editor, row));
			return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				ChangeSkillValue(selected, row, -step);
				return true;
			});
		}
		case RectNumberStepperInputAction::Increment:
		{
			const double step = ApplyTemporaryStepModifier(SkillEditorValueStep(editor, row));
			return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				ChangeSkillValue(selected, row, step);
				return true;
			});
		}
		default:
			return false;
		}
	}
}
