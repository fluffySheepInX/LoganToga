#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.h"

namespace LT3
{
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
			editor.showAiEditor = false;
			editor.statusText = U"AI Editor OFF";
			return true;
		}

		if (HandleRectButtonClick(AiEditorApplyRect()))
		{
			ApplySelectedAiProfileTag(editor, defs);
			SaveMapEditorToml(editor, false);
			editor.statusText = U"AI profile applied to next battle: {}"_fmt(editor.selectedAiProfileTag);
			return true;
		}

		if (HandleRectButtonClick(AiEditorSaveRect()))
		{
			ApplySelectedAiProfileTag(editor, defs);
			String status;
			SaveAiProfileDefinitions(defs, status);
			SaveMapEditorToml(editor, false);
			editor.statusText = status;
			editor.aiProfilesDirty = false;
			return true;
		}

		const RectF list = AiEditorListViewportRect();
		if (list.mouseOver())
		{
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
					SetSelectedAiProfile(editor, defs.aiProfiles[profileIndex], profileIndex);
					editor.aiProfileDetailScroll = 0.0;
					SaveMapEditorToml(editor, false);
					return true;
				}
			}
			return true;
		}

		const RectF detail = AiEditorDetailRect();
		if (detail.mouseOver())
		{
			if (!HasSelectedAiProfile(editor, defs))
			{
				return true;
			}

			AiProfileDef& profile = SelectedAiProfile(editor, defs);
			editor.aiProfileDetailScroll = Clamp(editor.aiProfileDetailScroll - Mouse::Wheel() * 42.0, 0.0, AiEditorDetailMaxScroll(profile));
			const Array<double> deltas = { -10.0, -1.0, 1.0, 10.0 };
			for (int32 rowIndex = 0; rowIndex < 11; ++rowIndex)
			{
				const RectF row = AiEditorValueRowRect(rowIndex, editor.aiProfileDetailScroll);
				if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
				{
					continue;
				}
				if (const Optional<double> delta = FindClickedDeltaButton(deltas, [&](int32 index)
					{
						return AiEditorValueButtonRect(row, index);
					}))
				{
					ApplyAiEditorDelta(profile, rowIndex, *delta);
					editor.aiProfilesDirty = true;
					return true;
				}
			}

			const RectF freeSpawnRect = AiEditorValueRowRect(11, editor.aiProfileDetailScroll);
			const RectF toggleRect{ freeSpawnRect.x + freeSpawnRect.w - 112.0, freeSpawnRect.y + 4.0, 98.0, 26.0 };
			if (HandleToggleRectButton(toggleRect, profile.freeSpawnEnabled))
			{
				editor.aiProfilesDirty = true;
				return true;
			}

			const RectF contactBehaviorRect = AiEditorValueRowRect(12, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ contactBehaviorRect.x + contactBehaviorRect.w - 112.0, contactBehaviorRect.y + 4.0, 98.0, 26.0 }))
			{
				profile.contactBehavior = NextAiContactBehavior(profile.contactBehavior);
				editor.aiProfilesDirty = true;
				return true;
			}

			int32 rowIndex = 13;
			const RectF addWeightRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addWeightRow.x + addWeightRow.w - 124.0, addWeightRow.y + 4.0, 110.0, 26.0 }))
			{
				profile.unitWeights << AiUnitWeightDef{ FindNextAiUnitWeightTag(profile, defs), 1.0 };
				editor.aiProfilesDirty = true;
				return true;
			}

			const Array<double> weightDeltas = { -0.5, -0.1, 0.1, 0.5 };
			for (int32 weightIndex = 0; weightIndex < static_cast<int32>(profile.unitWeights.size()); ++weightIndex)
			{
				const RectF row = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
				if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
				{
					continue;
				}

				if (const Optional<double> delta = FindClickedDeltaButton(weightDeltas, [&](int32 index)
					{
						return AiEditorInlineButtonRect(row, index, 5);
					}))
				{
					profile.unitWeights[weightIndex].weight = Max(0.0, profile.unitWeights[weightIndex].weight + *delta);
					editor.aiProfilesDirty = true;
					return true;
				}

				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 4, 5)))
				{
					profile.unitWeights.erase(profile.unitWeights.begin() + weightIndex);
					editor.aiProfilesDirty = true;
					return true;
				}
			}

			const RectF addTargetRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addTargetRow.x + addTargetRow.w - 124.0, addTargetRow.y + 4.0, 110.0, 26.0 }))
			{
				profile.targetPriority << U"base";
				editor.aiProfilesDirty = true;
				return true;
			}

			for (int32 targetIndex = 0; targetIndex < static_cast<int32>(profile.targetPriority.size()); ++targetIndex)
			{
				const RectF row = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
				if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
				{
					continue;
				}

				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 0, 3)))
				{
					profile.targetPriority[targetIndex] = PreviousAiTargetPriority(profile.targetPriority[targetIndex]);
					editor.aiProfilesDirty = true;
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 1, 3)))
				{
					profile.targetPriority[targetIndex] = NextAiTargetPriority(profile.targetPriority[targetIndex]);
					editor.aiProfilesDirty = true;
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 2, 3)))
				{
					profile.targetPriority.erase(profile.targetPriority.begin() + targetIndex);
					editor.aiProfilesDirty = true;
					return true;
				}
			}
			return true;
		}

		return true;
	}
}
