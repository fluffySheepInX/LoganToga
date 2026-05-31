#pragma once
# include <Siv3D.hpp>
# include "AiEditorInput.DetailRows.h"

namespace LT3
{
	// AI Editor の一覧パネル入力を処理する。
	inline bool HandleAiEditorListInput(MapEditorState& editor, DefinitionStores& defs)
	{
		const RectF list = AiEditorListViewportRect();
		if (!list.mouseOver())
		{
			return false;
		}

		const double maxScroll = Max(0.0, static_cast<double>(defs.aiProfiles.size()) * 58.0 - list.h);
		editor.aiProfileListScroll = Clamp(editor.aiProfileListScroll - Mouse::Wheel() * 58.0, 0.0, maxScroll);
		const int32 firstIndex = Max(0, static_cast<int32>(editor.aiProfileListScroll / 58.0));
		const int32 visibleRows = static_cast<int32>(list.h / 58.0) + 1;
		for (int32 visible = 0; visible < visibleRows; ++visible)
		{
			const int32 profileIndex = firstIndex + visible;
			if (profileIndex >= static_cast<int32>(defs.aiProfiles.size()))
			{
				break;
			}
			if (HandleRectButtonClick(AiEditorProfileRowRect(list, visible, 0.0)))
			{
				CloseAiEditorUnitWeightMenu(editor);
				SetSelectedAiProfile(editor, defs.aiProfiles[profileIndex], profileIndex);
				editor.aiProfileDetailScroll = 0.0;
				SaveMapEditorToml(editor, false);
				return true;
			}
		}
		return true;
	}

	// AI Editor の詳細パネル入力を処理する。
	inline bool HandleAiEditorDetailInput(MapEditorState& editor, DefinitionStores& defs)
	{
		const RectF detail = AiEditorDetailRect();
		if (!detail.mouseOver())
		{
			return false;
		}

		if (!HasSelectedAiProfile(editor, defs))
		{
			return true;
		}

		AiProfileDef& profile = SelectedAiProfile(editor, defs);
		const int32 firstWeightRowIndex = 16 + static_cast<int32>(profile.initialUnits.size());
		bool unitPickerOpen = false;
		if (editor.aiUnitWeightMenuRow && editor.aiUnitWeightMenuKind == AiEditorUnitWeightMenuKind::UnitPicker)
		{
			const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
			if (!tags.isEmpty())
			{
				const int32 visibleRows = Min(8, static_cast<int32>(tags.size()));
				const RectF pickerRect = AiEditorUnitWeightPickerRect(editor.aiUnitWeightMenuPos, visibleRows);
				unitPickerOpen = pickerRect.mouseOver();
			}
		}
		if (!unitPickerOpen)
		{
			editor.aiProfileDetailScroll = Clamp(editor.aiProfileDetailScroll - Mouse::Wheel() * 42.0, 0.0, AiEditorDetailMaxScroll(profile));
		}

		if (HandleAiEditorUnitWeightMenuInput(editor, defs, profile, firstWeightRowIndex))
		{
			return true;
		}

		if (HandleAiEditorDetailRowsInput(editor, defs, profile))
		{
			return true;
		}

		return true;
	}

	// AI Editor 全体の入力を処理する。
	inline bool ProcessAiEditorInput(MapEditorState& editor, DefinitionStores& defs)
	{
		if (!editor.showAiEditor)
		{
			return false;
		}

		const RectF panel = AiEditorPanelRect();
		if (!panel.mouseOver())
		{
			return false;
		}

		if (HandleRectButtonClick(AiEditorCloseRect()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			editor.showAiEditor = false;
			editor.statusText = U"AI Editor OFF";
			return true;
		}

		if (HandleRectButtonClick(AiEditorApplyRect()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			ApplySelectedAiProfileTag(editor, defs);
			SaveMapEditorToml(editor, false);
			editor.statusText = U"AI profile applied to next battle: {}"_fmt(editor.selectedAiProfileTag);
			return true;
		}

		if (HandleRectButtonClick(AiEditorSaveRect()))
		{
			CloseAiEditorUnitWeightMenu(editor);
			ApplySelectedAiProfileTag(editor, defs);
			String status;
			SaveAiProfileDefinitions(defs, status);
			SaveMapEditorToml(editor, false);
			editor.statusText = status;
			editor.aiProfilesDirty = false;
			return true;
		}

		if (HandleAiEditorListInput(editor, defs))
		{
			return true;
		}

		if (HandleAiEditorDetailInput(editor, defs))
		{
			return true;
		}

		return true;
	}
}
