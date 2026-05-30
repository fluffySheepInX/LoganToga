#pragma once
# include "SkillEditorInput.Common.h"

namespace LT3
{
	/// <summary>
	/// リソースコスト編集テキストを確定します。
	/// </summary>
	inline bool CommitSkillEditorResourceCostText(MapEditorState& editor, DefinitionStores& defs, int32 costIndex, const String& text)
	{
		const bool committed = MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
		{
			return TryCommitSkillResourceCostAmountText(selected, costIndex, text);
		});
		if (!committed)
		{
			editor.statusText = U"Invalid resource cost: {}"_fmt(text);
		}
		return committed;
	}

	/// <summary>
	/// スキル詳細のテキスト編集入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorDetailTextInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (editor.skillValueEditingRow >= 0)
		{
			TextInput::UpdateText(editor.skillValueEditingText);
			if (KeyEscape.down())
			{
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				CommitSelectedSkillEditorValueText(editor, defs, editor.skillValueEditingRow, editor.skillValueEditingText);
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
		}

		if (editor.skillResourceCostEditingIndex >= 0)
		{
			TextInput::UpdateText(editor.skillResourceCostEditingText);
			if (KeyEscape.down())
			{
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				const int32 costIndex = editor.skillResourceCostEditingIndex;
				const String text = editor.skillResourceCostEditingText;
				CommitSkillEditorResourceCostText(editor, defs, costIndex, text);
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// スキル値ステップメニュー入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorValueStepMenuInput(MapEditorState& editor)
	{
		if (!editor.skillValueStepMenuRow)
		{
			return false;
		}

		const Array<double>& steps = SkillEditorDefaultValueSteps();
		const RectF menuRect = SkillEditorValueStepMenuRect(editor.skillValueStepMenuPos, static_cast<int32>(steps.size()));
		for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
		{
			if (SkillEditorValueStepMenuItemRect(editor.skillValueStepMenuPos, i).leftClicked())
			{
				SetSkillEditorValueStep(editor, *editor.skillValueStepMenuRow, steps[i]);
				editor.skillValueStepMenuRow = none;
				editor.statusText = U"Skill value step set to {}"_fmt(steps[i]);
				return true;
			}
		}

		if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			editor.skillValueStepMenuRow = none;
			return true;
		}

		return true;
	}

	/// <summary>
	/// リソースコスト関連入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorResourceCostInput(MapEditorState& editor, DefinitionStores& defs, SkillDef& skill, double scroll)
	{
		if (editor.skillResourceCostStepMenuIndex)
		{
			const Array<double>& steps = SkillEditorResourceCostStepOptions();
			const RectF menuRect = SkillEditorResourceCostStepMenuRect(editor.skillResourceCostStepMenuPos, static_cast<int32>(steps.size()));
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				if (SkillEditorResourceCostStepMenuItemRect(editor.skillResourceCostStepMenuPos, i).leftClicked())
				{
					SetSkillEditorResourceCostStep(editor, *editor.skillResourceCostStepMenuIndex, steps[i]);
					editor.skillResourceCostStepMenuIndex = none;
					editor.statusText = U"Resource cost step set to {}"_fmt(steps[i]);
					return true;
				}
			}

			if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.skillResourceCostStepMenuIndex = none;
				return true;
			}

			return true;
		}

		if (editor.skillResourceCostEditingIndex >= 0 && MouseL.down())
		{
			const int32 costIndex = editor.skillResourceCostEditingIndex;
			const RectF editingRect = SkillEditorResourceCostAmountStepperRects(costIndex, scroll).value;
			if (!editingRect.mouseOver())
			{
				const String text = editor.skillResourceCostEditingText;
				CommitSkillEditorResourceCostText(editor, defs, costIndex, text);
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				return true;
			}
		}

		for (int32 i = 0; i < static_cast<int32>(skill.resourceCosts.size()); ++i)
		{
			if (HandleRectButtonClick(SkillEditorResourceCostTagRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					if (i < 0 || i >= static_cast<int32>(selected.resourceCosts.size()) || defs.resources.isEmpty())
					{
						return false;
					}

					int32 currentIndex = 0;
					for (int32 r = 0; r < static_cast<int32>(defs.resources.size()); ++r)
					{
						if (defs.resources[r].tag == selected.resourceCosts[i].resourceTag)
						{
							currentIndex = r;
							break;
						}
					}

					const int32 nextIndex = (currentIndex + 1) % static_cast<int32>(defs.resources.size());
					return SetFieldIfChanged(selected.resourceCosts[i].resourceTag, defs.resources[nextIndex].tag);
				});
				return true;
			}

			if (HandleRectButtonClick(SkillEditorResourceCostRemoveRect(i, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					if (i < 0 || i >= static_cast<int32>(selected.resourceCosts.size()))
					{
						return false;
					}
					selected.resourceCosts.remove_at(i);
					return true;
				});
				editor.skillResourceCostEditingIndex = -1;
				editor.skillResourceCostEditingText.clear();
				editor.skillResourceCostStepMenuIndex = none;
				EnsureSkillEditorResourceCostSteps(editor, defs.skills[editor.selectedSkillIndex]);
				return true;
			}

			switch (DetectRectNumberStepperInput(SkillEditorResourceCostAmountStepperRects(i, scroll)))
			{
			case RectNumberStepperInputAction::StartValueEdit:
				editor.skillResourceCostEditingIndex = i;
				editor.skillResourceCostEditingText = U"{}"_fmt(skill.resourceCosts[i].amount);
				editor.skillResourceCostStepMenuIndex = none;
				return true;
			case RectNumberStepperInputAction::CycleStep:
				CycleSkillEditorResourceCostStep(editor, i);
				editor.statusText = U"Resource cost step set to {}"_fmt(SkillEditorResourceCostStep(editor, i));
				return true;
			case RectNumberStepperInputAction::OpenStepMenu:
				editor.skillResourceCostStepMenuIndex = i;
				editor.skillResourceCostStepMenuPos = Cursor::PosF();
				return true;
			case RectNumberStepperInputAction::Decrement:
			{
				const int32 delta = static_cast<int32>(ApplyTemporaryStepModifier(SkillEditorResourceCostStep(editor, i)));
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return ChangeSkillResourceCostAmount(selected, i, -Max(1, delta));
				});
				return true;
			}
			case RectNumberStepperInputAction::Increment:
			{
				const int32 delta = static_cast<int32>(ApplyTemporaryStepModifier(SkillEditorResourceCostStep(editor, i)));
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return ChangeSkillResourceCostAmount(selected, i, Max(1, delta));
				});
				return true;
			}
			default:
				break;
			}
		}

		if (HandleRectButtonClick(SkillEditorResourceCostAddRect(static_cast<int32>(skill.resourceCosts.size()), scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				if (defs.resources.isEmpty())
				{
					return false;
				}
				selected.resourceCosts << SkillResourceCostDef{ defs.resources.front().tag, 1 };
				return true;
			});
			EnsureSkillEditorResourceCostSteps(editor, defs.skills[editor.selectedSkillIndex]);
			return true;
		}

		return false;
	}

	/// <summary>
	/// スキル詳細パネル入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorDetailInput(MapEditorState& editor, DefinitionStores& defs, SkillDef& skill)
	{
		const RectF detailViewport = SkillEditorDetailViewportRect();
		const double detailMaxScroll = Max(0.0, SkillEditorDetailContentHeight() - detailViewport.h);
		if (detailViewport.mouseOver())
		{
			editor.skillDetailScroll = Clamp(editor.skillDetailScroll - Mouse::Wheel() * 42.0, 0.0, detailMaxScroll);
		}

		const double scroll = editor.skillDetailScroll;
		if (ProcessSkillEditorValueStepMenuInput(editor))
		{
			return true;
		}
		if (ProcessSkillEditorResourceCostInput(editor, defs, skill, scroll))
		{
			return true;
		}
		if (editor.skillValueEditingRow >= 0 && MouseL.down())
		{
			const RectF editingRect = SkillEditorValueFieldRect(editor.skillValueEditingRow, scroll);
			if (!editingRect.mouseOver())
			{
				CommitSelectedSkillEditorValueText(editor, defs, editor.skillValueEditingRow, editor.skillValueEditingText);
				editor.skillValueEditingRow = -1;
				editor.skillValueEditingText.clear();
				return true;
			}
		}

		for (int32 row = 0; row < 26; ++row)
		{
			if (IsSkillEditorValueRowLocked(skill, row))
			{
				continue;
			}

			const RectNumberStepperRects rects = SkillEditorValueStepperRects(row, scroll);
			if (HandleSkillEditorValueStepperAction(editor, defs, row, rects))
			{
				return true;
			}
		}

		return false;
	}
}
