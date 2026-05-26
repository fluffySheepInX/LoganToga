#pragma once
# include "MapEditorUnitCatalogEditorInput.ListInput.h"
# include "EditorMutationHelpers.h"
# include "RectUiHelpers.h"

namespace LT3
{
	enum class UniqueEditorValueKind
	{
		Interval,
		Visible,
		BubbleWidth,
		BubbleHeight,
	};

	inline const Array<UniqueEditorValueKind>& UniqueEditorValueKinds()
	{
		static const Array<UniqueEditorValueKind> kinds = {
			UniqueEditorValueKind::Interval,
			UniqueEditorValueKind::Visible,
			UniqueEditorValueKind::BubbleWidth,
			UniqueEditorValueKind::BubbleHeight,
		};
		return kinds;
	}

	inline UniqueEditorValueKind UniqueEditorValueKindAt(int32 row)
	{
		return UniqueEditorValueKinds()[row];
	}

	inline const Array<double>& UniqueEditorDefaultSteps()
	{
		static const Array<double> steps = { 0.1, 0.5, 1.0, 5.0, 10.0 };
		return steps;
	}

	inline double UniqueEditorValueDefaultStep(UniqueEditorValueKind kind)
	{
		switch (kind)
		{
		case UniqueEditorValueKind::Interval:
		case UniqueEditorValueKind::Visible:
			return 0.1;
		case UniqueEditorValueKind::BubbleWidth:
		case UniqueEditorValueKind::BubbleHeight:
		default:
			return 10.0;
		}
	}

	inline void EnsureUniqueEditorSteps(MapEditorState& editor)
	{
		if (editor.uniqueEditorValueSteps.size() == UniqueEditorValueKinds().size())
		{
			return;
		}

		editor.uniqueEditorValueSteps.resize(UniqueEditorValueKinds().size());
		for (int32 row = 0; row < static_cast<int32>(UniqueEditorValueKinds().size()); ++row)
		{
			editor.uniqueEditorValueSteps[row] = UniqueEditorValueDefaultStep(UniqueEditorValueKindAt(row));
		}
	}

	inline double UniqueEditorStep(const MapEditorState& editor, int32 row)
	{
		return (0 <= row && row < static_cast<int32>(editor.uniqueEditorValueSteps.size())) ? editor.uniqueEditorValueSteps[row] : 1.0;
	}

	inline void CycleUniqueEditorStep(MapEditorState& editor, int32 row)
	{
		EnsureUniqueEditorSteps(editor);
		const Array<double>& steps = UniqueEditorDefaultSteps();
		editor.uniqueEditorValueSteps[row] = NextCycledStepValue(steps, UniqueEditorStep(editor, row));
	}

	inline double GetUniqueEditorValue(const UnitCatalogEntry& entry, UniqueEditorValueKind kind)
	{
		switch (kind)
		{
		case UniqueEditorValueKind::Interval: return entry.uniqueSpeechIntervalSec;
		case UniqueEditorValueKind::Visible: return entry.uniqueSpeechVisibleSec;
		case UniqueEditorValueKind::BubbleWidth: return entry.uniqueSpeechBubbleWidth;
		case UniqueEditorValueKind::BubbleHeight: return entry.uniqueSpeechBubbleHeight;
		default: return 0.0;
		}
	}

	inline double ApplyUniqueEditorValueBounds(UniqueEditorValueKind kind, double value)
	{
		switch (kind)
		{
		case UniqueEditorValueKind::Interval: return Math::Round(Clamp(value, 0.5, 60.0) * 10.0) / 10.0;
		case UniqueEditorValueKind::Visible: return Math::Round(Clamp(value, 0.2, 20.0) * 10.0) / 10.0;
		case UniqueEditorValueKind::BubbleWidth: return static_cast<double>(Clamp(static_cast<int32>(value), 80, 420));
		case UniqueEditorValueKind::BubbleHeight: return static_cast<double>(Clamp(static_cast<int32>(value), 36, 220));
		default: return value;
		}
	}

	inline double UniqueEditorValueResetValue(UniqueEditorValueKind kind)
	{
		switch (kind)
		{
		case UniqueEditorValueKind::Interval: return 6.0;
		case UniqueEditorValueKind::Visible: return 2.2;
		case UniqueEditorValueKind::BubbleWidth: return 180.0;
		case UniqueEditorValueKind::BubbleHeight: return 54.0;
		default: return 0.0;
		}
	}

	inline void SetUniqueEditorValue(UnitCatalogEntry& entry, UniqueEditorValueKind kind, double value)
	{
		const double bounded = ApplyUniqueEditorValueBounds(kind, value);
		switch (kind)
		{
		case UniqueEditorValueKind::Interval: entry.uniqueSpeechIntervalSec = bounded; break;
		case UniqueEditorValueKind::Visible: entry.uniqueSpeechVisibleSec = bounded; break;
		case UniqueEditorValueKind::BubbleWidth: entry.uniqueSpeechBubbleWidth = bounded; break;
		case UniqueEditorValueKind::BubbleHeight: entry.uniqueSpeechBubbleHeight = bounded; break;
		default: break;
		}
	}

	inline bool SetUniqueEditorValueIfChanged(UnitCatalogEntry& entry, UniqueEditorValueKind kind, double value)
	{
		const double next = ApplyUniqueEditorValueBounds(kind, value);
		if (GetUniqueEditorValue(entry, kind) == next)
		{
			return false;
		}

		SetUniqueEditorValue(entry, kind, next);
		return true;
	}

	inline bool AdjustUniqueEditorValue(UnitCatalogEntry& entry, UniqueEditorValueKind kind, double delta)
	{
		return SetUniqueEditorValueIfChanged(entry, kind, GetUniqueEditorValue(entry, kind) + delta);
	}

	inline String UniqueEditorValueEditText(const UnitCatalogEntry& entry, UniqueEditorValueKind kind)
	{
		return U"{}"_fmt(GetUniqueEditorValue(entry, kind));
	}

	inline bool TryCommitUniqueEditorValue(UnitCatalogEntry& entry, int32 row, const String& text)
	{
		if (text.isEmpty() || !(0 <= row && row < static_cast<int32>(UniqueEditorValueKinds().size())))
		{
			return false;
		}

		if (const Optional<double> value = ParseOpt<double>(text))
		{
			return SetUniqueEditorValueIfChanged(entry, UniqueEditorValueKindAt(row), *value);
		}
		return false;
	}

	inline bool ProcessUnitCatalogUniqueEditorInput(MapEditorState& editor, UnitCatalog& catalog, bool& consumed)
	{
		if (!(editor.showUniqueEditor && EditorUniquePanelRect(editor).mouseOver()))
		{
			return false;
		}

		EnsureUniqueEditorSteps(editor);
		consumed = true;
		if (!(0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size())))
		{
			return true;
		}

		UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];

		const RectF speechViewport = EditorUniqueSpeechViewportRect(editor);
		if (speechViewport.mouseOver())
		{
			const double contentHeight = Max(0.0, static_cast<double>(entry.uniqueSpeechLines.size()) * 54.0);
			const double maxScroll = Max(0.0, contentHeight - speechViewport.h);
			editor.uniqueSpeechScroll = Clamp(editor.uniqueSpeechScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
			if (Mouse::Wheel() != 0.0)
			{
				return true;
			}
		}

		if (EditorUniqueCheckRect(editor).leftClicked())
		{
			MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
			{
				return ToggleField(selected.unique);
			});
			return true;
		}
		if (EditorUniquePortraitButtonRect(editor).leftClicked())
		{
			return ChangeSelectedUnitPortraitFromDialog(editor, catalog);
		}
		if (EditorUniquePortraitClearRect(editor).leftClicked())
		{
			MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
			{
				return SetFieldIfChanged(selected.portraitImage, String{});
			});
			return true;
		}
		if (EditorUniqueRespawnCheckRect(editor).leftClicked())
		{
			MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
			{
				return ToggleField(selected.uniqueRespawnAllowed);
			});
			return true;
		}

		if (editor.uniqueEditorValueEditingRow >= 0)
		{
			TextInput::UpdateText(editor.uniqueEditorValueEditingText);
			if (KeyEscape.down())
			{
				editor.uniqueEditorValueEditingRow = -1;
				editor.uniqueEditorValueEditingText.clear();
				return true;
			}
			if (KeyEnter.down())
			{
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return TryCommitUniqueEditorValue(selected, editor.uniqueEditorValueEditingRow, editor.uniqueEditorValueEditingText);
				});
				editor.uniqueEditorValueEditingRow = -1;
				editor.uniqueEditorValueEditingText.clear();
				return true;
			}
		}

		if (editor.uniqueSpeechEditingIndex >= 0)
		{
			TextInput::UpdateText(editor.uniqueSpeechEditingText);
			if (KeyEscape.down())
			{
				editor.uniqueSpeechEditingIndex = -1;
				editor.uniqueSpeechEditingText.clear();
				return true;
			}
			if (KeyEnter.down() && editor.uniqueSpeechEditingIndex < static_cast<int32>(entry.uniqueSpeechLines.size()))
			{
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return SetFieldIfChanged(selected.uniqueSpeechLines[editor.uniqueSpeechEditingIndex], editor.uniqueSpeechEditingText);
				});
				editor.uniqueSpeechEditingIndex = -1;
				editor.uniqueSpeechEditingText.clear();
				return true;
			}
		}

		const RectF valueViewport = EditorUniqueValueViewportRect(editor);
		for (int32 rowIndex = 0; rowIndex < static_cast<int32>(UniqueEditorValueKinds().size()); ++rowIndex)
		{
			const UniqueEditorValueKind kind = UniqueEditorValueKindAt(rowIndex);
			const RectF row = EditorUniqueValueRowRect(valueViewport, rowIndex);
			const RectNumberStepperRects rects = EditorUniqueValueRowStepperRects(row);
			switch (DetectRectNumberStepperInput(rects))
			{
			case RectNumberStepperInputAction::StartValueEdit:
				editor.uniqueEditorValueEditingRow = rowIndex;
				editor.uniqueEditorValueEditingText = UniqueEditorValueEditText(entry, kind);
				return true;
			case RectNumberStepperInputAction::CycleStep:
				CycleUniqueEditorStep(editor, rowIndex);
				return true;
			case RectNumberStepperInputAction::Decrement:
			{
				const double step = ApplyTemporaryStepModifier(UniqueEditorStep(editor, rowIndex));
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return AdjustUniqueEditorValue(selected, kind, -step);
				});
				return true;
			}
			case RectNumberStepperInputAction::Increment:
			{
				const double step = ApplyTemporaryStepModifier(UniqueEditorStep(editor, rowIndex));
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return AdjustUniqueEditorValue(selected, kind, step);
				});
				return true;
			}
			default:
				break;
			}
			if (EditorUniqueValueRowButtonRect(row).leftClicked())
			{
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					return SetUniqueEditorValueIfChanged(selected, kind, UniqueEditorValueResetValue(kind));
				});
				return true;
			}
		}

		if (EditorUniqueSpeechAddRect(editor).leftClicked())
		{
			MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
			{
				selected.uniqueSpeechLines << U"New line";
				return true;
			});
			return true;
		}

		for (int32 i = 0; i < static_cast<int32>(entry.uniqueSpeechLines.size()); ++i)
		{
			const RectF row = EditorUniqueSpeechRowRect(speechViewport, i).movedBy(0.0, -editor.uniqueSpeechScroll);
			if (EditorUniqueSpeechDeleteRect(row).leftClicked())
			{
				MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& selected)
				{
					selected.uniqueSpeechLines.erase(selected.uniqueSpeechLines.begin() + i);
					const double contentHeight = Max(0.0, static_cast<double>(selected.uniqueSpeechLines.size()) * 54.0);
					const double maxScroll = Max(0.0, contentHeight - speechViewport.h);
					editor.uniqueSpeechScroll = Clamp(editor.uniqueSpeechScroll, 0.0, maxScroll);
					if (editor.uniqueSpeechEditingIndex == i)
					{
						editor.uniqueSpeechEditingIndex = -1;
						editor.uniqueSpeechEditingText.clear();
					}
					return true;
				});
				return true;
			}
			if (!speechViewport.intersects(row))
			{
				continue;
			}
			if (EditorUniqueSpeechTextRect(row).leftClicked())
			{
				editor.uniqueSpeechEditingIndex = i;
				editor.uniqueSpeechEditingText = entry.uniqueSpeechLines[i];
				return true;
			}
		}

		return true;
	}
}
