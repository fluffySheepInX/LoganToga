#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.h"
# include "EditorMutationHelpers.h"

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
			for (int32 rowIndex = 0; rowIndex < 12; ++rowIndex)
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
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						ApplyAiEditorDelta(selected, rowIndex, *delta);
						return true;
					});
					return true;
				}
			}

			const RectF freeSpawnRect = AiEditorValueRowRect(12, editor.aiProfileDetailScroll);
			const RectF toggleRect{ freeSpawnRect.x + freeSpawnRect.w - 112.0, freeSpawnRect.y + 4.0, 98.0, 26.0 };
			if (HandleRectButtonClick(toggleRect))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					return ToggleField(selected.freeSpawnEnabled);
				});
				return true;
			}

			const RectF contactBehaviorRect = AiEditorValueRowRect(13, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ contactBehaviorRect.x + contactBehaviorRect.w - 112.0, contactBehaviorRect.y + 4.0, 98.0, 26.0 }))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					return SetFieldIfChanged(selected.contactBehavior, NextAiContactBehavior(selected.contactBehavior));
				});
				return true;
			}

			int32 rowIndex = 14;
			const RectF addInitialUnitRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addInitialUnitRow.x + addInitialUnitRow.w - 124.0, addInitialUnitRow.y + 4.0, 110.0, 26.0 }))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					const String nextTag = FindNextAiInitialUnitTag(selected, defs);
					if (nextTag.isEmpty())
					{
						return false;
					}
					selected.initialUnits << nextTag;
					return true;
				});
				return true;
			}

			for (int32 initialUnitIndex = 0; initialUnitIndex < static_cast<int32>(profile.initialUnits.size()); ++initialUnitIndex)
			{
				const RectF row = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
				if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
				{
					continue;
				}

				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 0, 3)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= initialUnitIndex && initialUnitIndex < static_cast<int32>(selected.initialUnits.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.initialUnits[initialUnitIndex], NextAiInitialUnitTag(selected.initialUnits[initialUnitIndex], defs, -1));
					});
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 1, 3)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= initialUnitIndex && initialUnitIndex < static_cast<int32>(selected.initialUnits.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.initialUnits[initialUnitIndex], NextAiInitialUnitTag(selected.initialUnits[initialUnitIndex], defs, 1));
					});
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 2, 3)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= initialUnitIndex && initialUnitIndex < static_cast<int32>(selected.initialUnits.size())))
						{
							return false;
						}
						selected.initialUnits.erase(selected.initialUnits.begin() + initialUnitIndex);
						return true;
					});
					return true;
				}
			}

			const RectF addWeightRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addWeightRow.x + addWeightRow.w - 124.0, addWeightRow.y + 4.0, 110.0, 26.0 }))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					selected.unitWeights << AiUnitWeightDef{ FindNextAiUnitWeightTag(selected, defs), 1.0 };
					return true;
				});
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

				if (HandleRectButtonClick(AiEditorTinyInlineButtonRect(row, 0, 8)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.unitWeights[weightIndex].unitTag, NextAiUnitWeightTag(selected.unitWeights[weightIndex].unitTag, defs, -1));
					});
					return true;
				}

				if (HandleRectButtonClick(AiEditorTinyInlineButtonRect(row, 1, 8)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.unitWeights[weightIndex].unitTag, NextAiUnitWeightTag(selected.unitWeights[weightIndex].unitTag, defs, 1));
					});
					return true;
				}

				if (const Optional<double> delta = FindClickedDeltaButton(weightDeltas, [&](int32 index)
					{
						return AiEditorTinyInlineButtonRect(row, index + 2, 8);
					}))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return AdjustField(selected.unitWeights[weightIndex].weight, *delta, [](double value)
						{
							return Max(0.0, value);
						});
					});
					return true;
				}

				if (HandleRectButtonClick(AiEditorTinyInlineButtonRect(row, 6, 8)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						++selected.unitWeights[weightIndex].desiredCount;
						return true;
					});
					return true;
				}

				if (HandleRectButtonClick(AiEditorTinyInlineButtonRect(row, 7, 8)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						selected.unitWeights.erase(selected.unitWeights.begin() + weightIndex);
						return true;
					});
					return true;
				}
			}

			const RectF addBuildRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addBuildRow.x + addBuildRow.w - 124.0, addBuildRow.y + 4.0, 110.0, 26.0 }))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					const String nextTag = FindNextAiBuildPriorityTag(selected, defs);
					if (nextTag.isEmpty())
					{
						return false;
					}
					selected.buildPriorities << AiBuildPriorityDef{ nextTag, 1.0, 1 };
					return true;
				});
				return true;
			}

			const Array<double> buildWeightDeltas = { -0.5, -0.1, 0.1, 0.5 };
			for (int32 buildIndex = 0; buildIndex < static_cast<int32>(profile.buildPriorities.size()); ++buildIndex)
			{
				const RectF row = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
				if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
				{
					continue;
				}

				if (const Optional<double> delta = FindClickedDeltaButton(buildWeightDeltas, [&](int32 index)
					{
						return AiEditorCompactInlineButtonRect(row, index, 6);
					}))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= buildIndex && buildIndex < static_cast<int32>(selected.buildPriorities.size())))
						{
							return false;
						}
						return AdjustField(selected.buildPriorities[buildIndex].weight, *delta, [](double value)
						{
							return Max(0.0, value);
						});
					});
					return true;
				}

				if (HandleRectButtonClick(AiEditorCompactInlineButtonRect(row, 4, 6)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= buildIndex && buildIndex < static_cast<int32>(selected.buildPriorities.size())))
						{
							return false;
						}
						++selected.buildPriorities[buildIndex].desiredCount;
						return true;
					});
					return true;
				}

				if (HandleRectButtonClick(AiEditorCompactInlineButtonRect(row, 5, 6)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= buildIndex && buildIndex < static_cast<int32>(selected.buildPriorities.size())))
						{
							return false;
						}
						selected.buildPriorities.erase(selected.buildPriorities.begin() + buildIndex);
						return true;
					});
					return true;
				}
			}

			const RectF addTargetRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (HandleRectButtonClick(RectF{ addTargetRow.x + addTargetRow.w - 124.0, addTargetRow.y + 4.0, 110.0, 26.0 }))
			{
				MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
				{
					selected.targetPriority << U"base";
					return true;
				});
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
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= targetIndex && targetIndex < static_cast<int32>(selected.targetPriority.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.targetPriority[targetIndex], PreviousAiTargetPriority(selected.targetPriority[targetIndex]));
					});
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 1, 3)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= targetIndex && targetIndex < static_cast<int32>(selected.targetPriority.size())))
						{
							return false;
						}
						return SetFieldIfChanged(selected.targetPriority[targetIndex], NextAiTargetPriority(selected.targetPriority[targetIndex]));
					});
					return true;
				}
				if (HandleRectButtonClick(AiEditorInlineButtonRect(row, 2, 3)))
				{
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= targetIndex && targetIndex < static_cast<int32>(selected.targetPriority.size())))
						{
							return false;
						}
						selected.targetPriority.erase(selected.targetPriority.begin() + targetIndex);
						return true;
					});
					return true;
				}
			}
			return true;
		}

		return true;
	}
}
