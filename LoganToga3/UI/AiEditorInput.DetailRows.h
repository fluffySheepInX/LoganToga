#pragma once
# include <Siv3D.hpp>
# include "AiEditorInput.UnitWeightMenu.h"

namespace LT3
{
	// 基本パラメータ行の入力を処理する。
	inline bool HandleAiEditorBasicValueRowsInput(MapEditorState& editor, DefinitionStores& defs)
	{
		const RectF detail = AiEditorDetailRect();
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
		const RectF toggleRect{ AiEditorActionRightX(freeSpawnRect) - 98.0, freeSpawnRect.y + 4.0, 98.0, 26.0 };
		if (HandleRectButtonClick(toggleRect))
		{
			CloseAiEditorUnitWeightMenu(editor);
			MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
			{
				return ToggleField(selected.freeSpawnEnabled);
			});
			return true;
		}

		const RectF contactBehaviorRect = AiEditorValueRowRect(13, editor.aiProfileDetailScroll);
		if (HandleRectButtonClick(RectF{ AiEditorActionRightX(contactBehaviorRect) - 98.0, contactBehaviorRect.y + 4.0, 98.0, 26.0 }))
		{
			CloseAiEditorUnitWeightMenu(editor);
			MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
			{
				return SetFieldIfChanged(selected.contactBehavior, NextAiContactBehavior(selected.contactBehavior));
			});
			return true;
		}

		return false;
	}

	// 初期ユニット行の入力を処理する。
	inline bool HandleAiEditorInitialUnitRowsInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32& rowIndex)
	{
		const RectF detail = AiEditorDetailRect();
		const RectF addInitialUnitRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
		if (HandleRectButtonClick(RectF{ AiEditorActionRightX(addInitialUnitRow) - 110.0, addInitialUnitRow.y + 4.0, 110.0, 26.0 }))
		{
			CloseAiEditorUnitWeightMenu(editor);
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

		return false;
	}

	// unit weight 行の入力を処理する。
	inline bool HandleAiEditorUnitWeightRowsInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32& rowIndex, int32 firstWeightRowIndex)
	{
		const RectF detail = AiEditorDetailRect();
		const RectF addWeightRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
		if (HandleRectButtonClick(RectF{ AiEditorActionRightX(addWeightRow) - 110.0, addWeightRow.y + 4.0, 110.0, 26.0 }))
		{
			CloseAiEditorUnitWeightMenu(editor);
			MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
			{
				selected.unitWeights << AiUnitWeightDef{ FindNextAiUnitWeightTag(selected, defs), 1.0 };
				return true;
			});
			return true;
		}

		for (int32 weightIndex = 0; weightIndex < static_cast<int32>(profile.unitWeights.size()); ++weightIndex)
		{
			const RectF row = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
			if (row.y + row.h < detail.y || detail.y + detail.h < row.y)
			{
				continue;
			}

			const int32 absoluteRowIndex = firstWeightRowIndex + weightIndex;
			const RectF unitButtonRect = AiEditorUnitWeightUnitButtonRect(row);
			const RectNumberStepperRects weightStepper = AiEditorUnitWeightValueStepperRects(row);
			const RectNumberStepperRects desiredStepper = AiEditorUnitWeightDesiredStepperRects(row);
			if (unitButtonRect.leftClicked())
			{
				editor.aiUnitWeightMenuRow = absoluteRowIndex;
				editor.aiUnitWeightMenuKind = AiEditorUnitWeightMenuKind::UnitPicker;
				editor.aiUnitWeightMenuPos = unitButtonRect.bl() + Vec2{ 0.0, 4.0 };
				editor.aiUnitWeightUnitPickerScroll = 0.0;
				return true;
			}

			switch (DetectRectNumberStepperInput(weightStepper))
			{
			case RectNumberStepperInputAction::CycleStep:
				editor.aiUnitWeightStep = NextCycledStepValue(AiEditorUnitWeightStepOptions(), editor.aiUnitWeightStep);
				editor.statusText = U"AI unit weight step set to {}"_fmt(editor.aiUnitWeightStep);
				return true;
			case RectNumberStepperInputAction::OpenStepMenu:
				editor.aiUnitWeightMenuRow = absoluteRowIndex;
				editor.aiUnitWeightMenuKind = AiEditorUnitWeightMenuKind::WeightStep;
				editor.aiUnitWeightMenuPos = Cursor::PosF();
				return true;
			case RectNumberStepperInputAction::Decrement:
				{
					const double delta = ApplyTemporaryStepModifier(editor.aiUnitWeightStep);
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return AdjustField(selected.unitWeights[weightIndex].weight, -delta, [](double value)
						{
							return Max(0.0, value);
						});
					});
					return true;
				}
			case RectNumberStepperInputAction::Increment:
				{
					const double delta = ApplyTemporaryStepModifier(editor.aiUnitWeightStep);
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return AdjustField(selected.unitWeights[weightIndex].weight, delta, [](double value)
						{
							return Max(0.0, value);
						});
					});
					return true;
				}
			default:
				break;
			}

			switch (DetectRectNumberStepperInput(desiredStepper))
			{
			case RectNumberStepperInputAction::CycleStep:
				for (int32 i = 0; i < static_cast<int32>(AiEditorUnitDesiredStepOptions().size()); ++i)
				{
					if (AiEditorUnitDesiredStepOptions()[i] == editor.aiUnitWeightDesiredStep)
					{
						editor.aiUnitWeightDesiredStep = AiEditorUnitDesiredStepOptions()[(i + 1) % static_cast<int32>(AiEditorUnitDesiredStepOptions().size())];
						editor.statusText = U"AI desired count step set to {}"_fmt(editor.aiUnitWeightDesiredStep);
						return true;
					}
				}
				editor.aiUnitWeightDesiredStep = AiEditorUnitDesiredStepOptions().front();
				editor.statusText = U"AI desired count step set to {}"_fmt(editor.aiUnitWeightDesiredStep);
				return true;
			case RectNumberStepperInputAction::OpenStepMenu:
				editor.aiUnitWeightMenuRow = absoluteRowIndex;
				editor.aiUnitWeightMenuKind = AiEditorUnitWeightMenuKind::DesiredStep;
				editor.aiUnitWeightMenuPos = Cursor::PosF();
				return true;
			case RectNumberStepperInputAction::Decrement:
				{
					const int32 delta = Max(1, static_cast<int32>(Round(ApplyTemporaryStepModifier(static_cast<double>(editor.aiUnitWeightDesiredStep)))));
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						return AdjustField(selected.unitWeights[weightIndex].desiredCount, -delta, [](int32 value)
						{
							return Max(0, value);
						});
					});
					return true;
				}
			case RectNumberStepperInputAction::Increment:
				{
					const int32 delta = Max(1, static_cast<int32>(Round(ApplyTemporaryStepModifier(static_cast<double>(editor.aiUnitWeightDesiredStep)))));
					MutateSelectedAiProfile(editor, defs, [&](AiProfileDef& selected)
					{
						if (!(0 <= weightIndex && weightIndex < static_cast<int32>(selected.unitWeights.size())))
						{
							return false;
						}
						++selected.unitWeights[weightIndex].desiredCount;
						selected.unitWeights[weightIndex].desiredCount += (delta - 1);
						return true;
					});
					return true;
				}
			default:
				break;
			}

			if (HandleRectButtonClick(AiEditorUnitWeightDeleteRect(row)))
			{
				CloseAiEditorUnitWeightMenu(editor);
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

		return false;
	}

	// build priority 行の入力を処理する。
	inline bool HandleAiEditorBuildPriorityRowsInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32& rowIndex)
	{
		const RectF detail = AiEditorDetailRect();
		const RectF addBuildRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
		if (HandleRectButtonClick(RectF{ AiEditorActionRightX(addBuildRow) - 110.0, addBuildRow.y + 4.0, 110.0, 26.0 }))
		{
			CloseAiEditorUnitWeightMenu(editor);
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

		return false;
	}

	// target priority 行の入力を処理する。
	inline bool HandleAiEditorTargetPriorityRowsInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile, int32& rowIndex)
	{
		const RectF detail = AiEditorDetailRect();
		const RectF addTargetRow = AiEditorValueRowRect(rowIndex++, editor.aiProfileDetailScroll);
		if (HandleRectButtonClick(RectF{ AiEditorActionRightX(addTargetRow) - 110.0, addTargetRow.y + 4.0, 110.0, 26.0 }))
		{
			CloseAiEditorUnitWeightMenu(editor);
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

		return false;
	}

	// 詳細パネル内の各編集行入力をまとめて処理する。
	inline bool HandleAiEditorDetailRowsInput(MapEditorState& editor, DefinitionStores& defs, AiProfileDef& profile)
	{
		if (HandleAiEditorBasicValueRowsInput(editor, defs))
		{
			return true;
		}

		int32 rowIndex = 14;
		if (HandleAiEditorInitialUnitRowsInput(editor, defs, profile, rowIndex))
		{
			return true;
		}

		const int32 firstWeightRowIndex = 16 + static_cast<int32>(profile.initialUnits.size());
		if (HandleAiEditorUnitWeightRowsInput(editor, defs, profile, rowIndex, firstWeightRowIndex))
		{
			return true;
		}

		if (HandleAiEditorBuildPriorityRowsInput(editor, defs, profile, rowIndex))
		{
			return true;
		}

		if (HandleAiEditorTargetPriorityRowsInput(editor, defs, profile, rowIndex))
		{
			return true;
		}

		return false;
	}
}
