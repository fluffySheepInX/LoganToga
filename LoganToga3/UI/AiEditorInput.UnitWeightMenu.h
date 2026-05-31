#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.h"
# include "EditorMutationHelpers.h"

namespace LT3
{
	// unit weight 用の補助メニュー状態を閉じる。
	inline void CloseAiEditorUnitWeightMenu(MapEditorState& editor)
	{
		editor.aiUnitWeightMenuRow = none;
		editor.aiUnitWeightMenuKind = AiEditorUnitWeightMenuKind::None;
		editor.aiUnitWeightUnitPickerScroll = 0.0;
	}

	// weight step メニュー入力を処理する。
	inline bool HandleAiEditorUnitWeightStepMenuInput(MapEditorState& editor)
	{
		const Array<double>& steps = AiEditorUnitWeightStepOptions();
		const RectF menuRect = AiEditorUnitWeightStepMenuRect(editor.aiUnitWeightMenuPos, static_cast<int32>(steps.size()));
		for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
		{
			const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
			if (item.leftClicked())
			{
				editor.aiUnitWeightStep = steps[i];
				CloseAiEditorUnitWeightMenu(editor);
				editor.statusText = U"AI unit weight step set to {}"_fmt(steps[i]);
				return true;
			}
		}

		if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			return true;
		}

		return true;
	}

	// desired count step メニュー入力を処理する。
	inline bool HandleAiEditorUnitDesiredStepMenuInput(MapEditorState& editor)
	{
		const Array<int32>& steps = AiEditorUnitDesiredStepOptions();
		const RectF menuRect = AiEditorUnitWeightStepMenuRect(editor.aiUnitWeightMenuPos, static_cast<int32>(steps.size()));
		for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
		{
			const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
			if (item.leftClicked())
			{
				editor.aiUnitWeightDesiredStep = steps[i];
				CloseAiEditorUnitWeightMenu(editor);
				editor.statusText = U"AI desired count step set to {}"_fmt(steps[i]);
				return true;
			}
		}

		if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			return true;
		}

		return true;
	}

	// unit picker メニュー入力を処理する。
	inline bool HandleAiEditorUnitWeightPickerInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32 firstWeightRowIndex)
	{
		const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
		const int32 weightIndex = *editor.aiUnitWeightMenuRow - firstWeightRowIndex;
		if (tags.isEmpty() || !(0 <= weightIndex && weightIndex < static_cast<int32>(profile.unitWeights.size())))
		{
			CloseAiEditorUnitWeightMenu(editor);
			return true;
		}

		const int32 visibleRows = Min(8, static_cast<int32>(tags.size()));
		const RectF pickerRect = AiEditorUnitWeightPickerRect(editor.aiUnitWeightMenuPos, visibleRows);
		if (pickerRect.mouseOver())
		{
			const double maxScroll = Max(0.0, static_cast<double>(tags.size()) * 24.0 - visibleRows * 24.0);
			editor.aiUnitWeightUnitPickerScroll = Clamp(editor.aiUnitWeightUnitPickerScroll - Mouse::Wheel() * 24.0, 0.0, maxScroll);
		}

		for (int32 index = 0; index < static_cast<int32>(tags.size()); ++index)
		{
			const RectF item = AiEditorUnitWeightPickerItemRect(pickerRect, index, editor.aiUnitWeightUnitPickerScroll);
			if (item.y + item.h < pickerRect.y + 4.0 || pickerRect.y + pickerRect.h - 4.0 < item.y)
			{
				continue;
			}
			if (item.leftClicked())
			{
				const String pickedTag = tags[index];
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
					{
						return false;
					}
					return SetFieldIfChanged(selected.unitWeights[weightIndex].unitTag, pickedTag);
				});
				CloseAiEditorUnitWeightMenu(editor);
				return true;
			}
		}

		if (!pickerRect.mouseOver() && (MouseL.down() || MouseR.down()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			return true;
		}

		if (pickerRect.mouseOver())
		{
			return true;
		}

		return false;
	}

	// unit weight 関連メニュー入力をまとめて処理する。
	inline bool HandleAiEditorUnitWeightMenuInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32 firstWeightRowIndex)
	{
		if (!editor.aiUnitWeightMenuRow)
		{
			return false;
		}

		switch (editor.aiUnitWeightMenuKind)
		{
		case AiEditorUnitWeightMenuKind::WeightStep:
			return HandleAiEditorUnitWeightStepMenuInput(editor);
		case AiEditorUnitWeightMenuKind::DesiredStep:
			return HandleAiEditorUnitDesiredStepMenuInput(editor);
		case AiEditorUnitWeightMenuKind::UnitPicker:
			return HandleAiEditorUnitWeightPickerInput(editor, defs, profile, firstWeightRowIndex);
		default:
			return false;
		}
	}
}
