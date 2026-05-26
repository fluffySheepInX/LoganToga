#pragma once
# include <Siv3D.hpp>
# include "../Data/UnitCatalog.h"
# include "MapEditorTypes.h"

namespace LT3
{
	enum class UnitParamRowKind
	{
		Hp,
		BuildingHp,
		Mp,
		Attack,
		Defense,
		Speed,
		Magic,
		MagicDefense,
		Move,
		Vision,
		VisualScale,
		MaintainRange,
	};

	struct UnitParamRowSpec
	{
		UnitParamRowKind kind;
		String label;
		String helpText;
		bool useIcon = false;
	};

	inline Array<UnitParamRowSpec> UnitParamRowSpecs(int32 tab)
	{
		switch (tab)
		{
		case 0:
			return {
				{ UnitParamRowKind::Hp, U"HP", U"HP を増減します" },
				{ UnitParamRowKind::BuildingHp, U"BHP", U"Building HP を増減します" },
				{ UnitParamRowKind::Mp, U"MP", U"MP を増減します" },
			};
		case 1:
			return {
				{ UnitParamRowKind::Attack, U"ATK", U"攻撃力を増減します" },
				{ UnitParamRowKind::Defense, U"DEF", U"防御力を増減します" },
				{ UnitParamRowKind::Speed, U"SPD", U"速度を増減します" },
				{ UnitParamRowKind::Magic, U"MAG", U"魔力を増減します" },
				{ UnitParamRowKind::MagicDefense, U"MDEF", U"魔法防御を増減します" },
			};
		case 2:
			return {
				{ UnitParamRowKind::Move, U"MOVE", U"移動力。Uで move=0(Use SPD)", true },
				{ UnitParamRowKind::Vision, U"VISION", U"視界半径（セル）" },
				{ UnitParamRowKind::VisualScale, U"SCALE", U"見た目サイズ" },
			};
		default:
			return {
				{ UnitParamRowKind::MaintainRange, U"RANGE", U"maintain_range を増減します" },
			};
		}
	}

	inline const Array<double>& UnitParamDefaultSteps()
	{
		static const Array<double> steps = { 0.1, 0.5, 1.0, 5.0, 10.0 };
		return steps;
	}

	inline double UnitParamDefaultStep(UnitParamRowKind kind)
	{
		return (kind == UnitParamRowKind::VisualScale) ? 0.05 : 1.0;
	}

	inline int32 UnitParamStepIndex(int32 tab, int32 rowIndex)
	{
		return tab * 8 + rowIndex;
	}

	inline void EnsureUnitParamSteps(MapEditorState& editor)
	{
		if (editor.unitParamSteps.size() == 32)
		{
			return;
		}

		editor.unitParamSteps.resize(32);
		for (int32 tab = 0; tab < 4; ++tab)
		{
			const Array<UnitParamRowSpec> rows = UnitParamRowSpecs(tab);
			for (int32 rowIndex = 0; rowIndex < static_cast<int32>(rows.size()); ++rowIndex)
			{
				editor.unitParamSteps[UnitParamStepIndex(tab, rowIndex)] = UnitParamDefaultStep(rows[rowIndex].kind);
			}
		}
	}

	inline double UnitParamStep(const MapEditorState& editor, int32 tab, int32 rowIndex)
	{
		const int32 index = UnitParamStepIndex(tab, rowIndex);
		if (0 <= index && index < static_cast<int32>(editor.unitParamSteps.size()))
		{
			return editor.unitParamSteps[index];
		}

		const Array<UnitParamRowSpec> rows = UnitParamRowSpecs(tab);
		if (0 <= rowIndex && rowIndex < static_cast<int32>(rows.size()))
		{
			return UnitParamDefaultStep(rows[rowIndex].kind);
		}

		return 1.0;
	}

	inline void SetUnitParamStep(MapEditorState& editor, int32 tab, int32 rowIndex, double step)
	{
		EnsureUnitParamSteps(editor);
		const int32 index = UnitParamStepIndex(tab, rowIndex);
		if (0 <= index && index < static_cast<int32>(editor.unitParamSteps.size()))
		{
			editor.unitParamSteps[index] = step;
		}
	}

	inline void CycleUnitParamStep(MapEditorState& editor, int32 tab, int32 rowIndex)
	{
		const Array<double>& steps = UnitParamDefaultSteps();
		SetUnitParamStep(editor, tab, rowIndex, NextCycledStepValue(steps, UnitParamStep(editor, tab, rowIndex)));
	}

	inline double GetUnitParamValue(const UnitCatalogEntry& entry, UnitParamRowKind kind)
	{
		switch (kind)
		{
		case UnitParamRowKind::Hp: return static_cast<double>(entry.hp);
		case UnitParamRowKind::BuildingHp: return static_cast<double>(entry.buildingHp);
		case UnitParamRowKind::Mp: return static_cast<double>(entry.mp);
		case UnitParamRowKind::Attack: return static_cast<double>(entry.attack);
		case UnitParamRowKind::Defense: return static_cast<double>(entry.defense);
		case UnitParamRowKind::Speed: return static_cast<double>(entry.speed);
		case UnitParamRowKind::Magic: return static_cast<double>(entry.magic);
		case UnitParamRowKind::MagicDefense: return static_cast<double>(entry.magicDefense);
		case UnitParamRowKind::Move: return static_cast<double>(entry.move);
		case UnitParamRowKind::Vision: return static_cast<double>(entry.visionRadius);
		case UnitParamRowKind::VisualScale: return entry.visualScale;
		case UnitParamRowKind::MaintainRange: return static_cast<double>(entry.maintainRange);
		default: return 0.0;
		}
	}

	inline String UnitParamValueText(const UnitCatalogEntry& entry, UnitParamRowKind kind)
	{
		if (kind == UnitParamRowKind::VisualScale)
		{
			return U"{:.2f}"_fmt(entry.visualScale);
		}

		return U"{}"_fmt(static_cast<int32>(GetUnitParamValue(entry, kind)));
	}

	inline String UnitParamEditValueText(const UnitCatalogEntry& entry, UnitParamRowKind kind)
	{
		return U"{}"_fmt(GetUnitParamValue(entry, kind));
	}

	inline double ApplyUnitParamBounds(UnitParamRowKind kind, double value)
	{
		switch (kind)
		{
		case UnitParamRowKind::Move:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 2000));
		case UnitParamRowKind::Vision:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 40));
		case UnitParamRowKind::VisualScale:
			return Math::Round(Clamp(value, 0.25, 3.0) * 100.0) / 100.0;
		default:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 99999));
		}
	}

	inline void SetUnitParamValue(UnitCatalogEntry& entry, UnitParamRowKind kind, double value)
	{
		const double bounded = ApplyUnitParamBounds(kind, value);
		switch (kind)
		{
		case UnitParamRowKind::Hp: entry.hp = static_cast<int32>(bounded); break;
		case UnitParamRowKind::BuildingHp: entry.buildingHp = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Mp: entry.mp = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Attack: entry.attack = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Defense: entry.defense = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Speed: entry.speed = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Magic: entry.magic = static_cast<int32>(bounded); break;
		case UnitParamRowKind::MagicDefense: entry.magicDefense = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Move: entry.move = static_cast<int32>(bounded); break;
		case UnitParamRowKind::Vision: entry.visionRadius = static_cast<int32>(bounded); break;
		case UnitParamRowKind::VisualScale: entry.visualScale = bounded; break;
		case UnitParamRowKind::MaintainRange: entry.maintainRange = static_cast<int32>(bounded); break;
		default: break;
		}
	}

	inline bool SetUnitParamValueIfChanged(UnitCatalogEntry& entry, UnitParamRowKind kind, double value)
	{
		const double next = ApplyUnitParamBounds(kind, value);
		if (GetUnitParamValue(entry, kind) == next)
		{
			return false;
		}

		SetUnitParamValue(entry, kind, next);
		return true;
	}

	inline void ChangeUnitParamValue(UnitCatalogEntry& entry, UnitParamRowKind kind, double delta)
	{
		SetUnitParamValue(entry, kind, GetUnitParamValue(entry, kind) + delta);
	}

	inline bool AdjustUnitParamValue(UnitCatalogEntry& entry, UnitParamRowKind kind, double delta)
	{
		return SetUnitParamValueIfChanged(entry, kind, GetUnitParamValue(entry, kind) + delta);
	}

	inline double UnitParamResetValue(UnitParamRowKind kind)
	{
		switch (kind)
		{
		case UnitParamRowKind::Vision:
			return 6.0;
		case UnitParamRowKind::VisualScale:
			return 1.0;
		default:
			return 0.0;
		}
	}

	inline bool UnitParamHasSpecialAction(UnitParamRowKind kind)
	{
		return (kind == UnitParamRowKind::Move);
	}

	inline bool ApplyUnitParamSpecialAction(UnitCatalogEntry& entry, UnitParamRowKind kind)
	{
		if (kind != UnitParamRowKind::Move)
		{
			return false;
		}

		if (entry.move == 0)
		{
			return false;
		}

		entry.move = 0;
		return true;
	}

	inline bool TryCommitUnitParamText(UnitCatalogEntry& entry, UnitParamRowKind kind, const String& text)
	{
		if (text.isEmpty())
		{
			return false;
		}

		if (const Optional<double> value = ParseOpt<double>(text))
		{
			SetUnitParamValue(entry, kind, *value);
			return true;
		}

		return false;
	}
}
