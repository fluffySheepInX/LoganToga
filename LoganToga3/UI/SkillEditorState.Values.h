#pragma once
# include "SkillEditorState.Metadata.h"

namespace LT3
{
	inline bool IsSkillEditorValueRowLocked(const SkillDef& skill, int32 row)
	{
		return row == 0 && skill.projectileMotion == SkillProjectileMotion::Swing;
	}

	inline String SkillEditorValueRowLockNote(const SkillDef& skill, int32 row)
	{
		if (row == 0 && skill.projectileMotion == SkillProjectileMotion::Swing)
		{
			return U"Swingではrangeは直接編集せず、実効射程はswingRとHのワールド長から決まります。center=endでは到達長はおおむねswingR+Hです。";
		}
		if (row == 21 && skill.projectileMotion == SkillProjectileMotion::Swing)
		{
			return U"swingRは射程ではなく、画像の根元をユニット中心から離す距離です。射程ではなく根元オフセットとして扱われます。";
		}

		return U"";
	}

	inline const Array<double>& SkillEditorDefaultValueSteps()
	{
		static const Array<double> steps = { 0.1, 0.5, 1.0, 5.0, 10.0 };
		return steps;
	}

	inline double SkillEditorDefaultValueStep(int32 row)
	{
		switch (row)
		{
		case 1:
		case 11:
		case 16:
			return 0.1;
		case 9:
			return 0.5;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 10:
		case 18:
			return 1.0;
		default:
			return 1.0;
		}
	}

	inline void EnsureSkillEditorValueSteps(MapEditorState& editor)
	{
		if (editor.skillValueSteps.size() == 26)
		{
			return;
		}

		editor.skillValueSteps.resize(26);
		for (int32 row = 0; row < 26; ++row)
		{
			editor.skillValueSteps[row] = SkillEditorDefaultValueStep(row);
		}
	}

	inline double SkillEditorValueStep(const MapEditorState& editor, int32 row)
	{
		if (0 <= row && row < static_cast<int32>(editor.skillValueSteps.size()))
		{
			return editor.skillValueSteps[row];
		}

		return SkillEditorDefaultValueStep(row);
	}

	inline void SetSkillEditorValueStep(MapEditorState& editor, int32 row, double step)
	{
		EnsureSkillEditorValueSteps(editor);
		if (0 <= row && row < static_cast<int32>(editor.skillValueSteps.size()))
		{
			editor.skillValueSteps[row] = step;
		}
	}

	inline void CycleSkillEditorValueStep(MapEditorState& editor, int32 row)
	{
		const Array<double>& steps = SkillEditorDefaultValueSteps();
		SetSkillEditorValueStep(editor, row, NextCycledStepValue(steps, SkillEditorValueStep(editor, row)));
	}

	inline double ApplySkillEditorValueBounds(int32 row, double value)
	{
		switch (row)
		{
		case 0:
			return Max(0.0, value);
		case 1:
			return Max(0.0, value);
		case 2:
			return Max(0.05, value);
		case 3:
			return Clamp(value, 0.01, 100.0);
		case 4:
			return Max(0.0, value);
		case 5:
			return Max(1.0, value);
		case 6:
			return static_cast<double>(Clamp(static_cast<int32>(value), 1, 32));
		case 7:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 1));
		case 8:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 1));
		case 9:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 2));
		case 10:
			return Max(0.1, value);
		case 11:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 1));
		case 12:
			return Max(0.0, value);
		case 13:
			return Clamp(value, 0.0, 180.0);
		case 14:
			return Max(0.0, value);
		case 15:
			return Max(1.0, value);
		case 16:
			return value;
		case 17:
			return Max(0.05, value);
		case 18:
			return Clamp(value, -360.0, 360.0);
		case 19:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 9));
		case 20:
		case 21:
			return Max(1.0, value);
		case 22:
			return Max(0.0, value);
		case 23:
			return Clamp(value, -720.0, 720.0);
		case 24:
			return Max(0.0, value);
		case 25:
			return Max(0.0, value);
		default:
			return value;
		}
	}

	inline double GetSkillEditorValue(const SkillDef& skill, int32 row)
	{
		switch (row)
		{
		case 0: return skill.range;
		case 1: return skill.rangeMin;
		case 2: return skill.cooldownSec;
		case 3: return skill.damage;
		case 4: return skill.selfDamageOnHit;
		case 5: return skill.projectileSpeed;
		case 6: return static_cast<double>(skill.burstCount);
		case 7: return static_cast<double>(skill.burstFireMode == SkillBurstFireMode::Staggered ? 1 : 0);
		case 8: return static_cast<double>(skill.burstOrderMode == SkillBurstOrderMode::Random ? 1 : 0);
		case 9: return static_cast<double>(skill.rayMode == SkillRayMode::Image ? 1 : (skill.rayMode == SkillRayMode::Line ? 2 : 0));
		case 10: return skill.rayLength;
		case 11: return static_cast<double>(skill.rayLockToCaster ? 1 : 0);
		case 12: return skill.burstIntervalSec;
		case 13: return skill.spreadDeg;
		case 14: return skill.arcHeight;
		case 15: return skill.orbitRadius;
		case 16: return skill.orbitAngularSpeedDeg;
		case 17: return skill.orbitDurationSec;
		case 18: return skill.projectileStartDegree;
		case 19: return static_cast<double>(skill.projectileStartDegreeType);
		case 20: return skill.projectileWidth;
		case 21: return skill.projectileHeight;
		case 22: return skill.swingRadius;
		case 23: return skill.swingAngleDeg;
		case 24: return skill.bomRadius;
		case 25: return skill.bomSelfDamageScale;
		default: return 0.0;
		}
	}

	inline void SetSkillEditorValue(SkillDef& skill, int32 row, double value)
	{
		const double bounded = ApplySkillEditorValueBounds(row, value);
		switch (row)
		{
		case 0: skill.range = bounded; break;
		case 1: skill.rangeMin = Min(skill.range, bounded); break;
		case 2: skill.cooldownSec = bounded; break;
		case 3: skill.damage = bounded; break;
		case 4: skill.selfDamageOnHit = bounded; break;
		case 5: skill.projectileSpeed = bounded; break;
		case 6: skill.burstCount = static_cast<int32>(bounded); break;
		case 7: skill.burstFireMode = (static_cast<int32>(bounded) == 1) ? SkillBurstFireMode::Staggered : SkillBurstFireMode::Simultaneous; break;
		case 8: skill.burstOrderMode = (static_cast<int32>(bounded) == 1) ? SkillBurstOrderMode::Random : SkillBurstOrderMode::Sequential; break;
		case 9:
			switch (static_cast<int32>(bounded))
			{
			case 1: skill.rayMode = SkillRayMode::Image; break;
			case 2: skill.rayMode = SkillRayMode::Line; break;
			default: skill.rayMode = SkillRayMode::None; break;
			}
			break;
		case 10: skill.rayLength = bounded; break;
		case 11: skill.rayLockToCaster = (static_cast<int32>(bounded) == 1); break;
		case 12: skill.burstIntervalSec = bounded; break;
		case 13: skill.spreadDeg = bounded; break;
		case 14: skill.arcHeight = bounded; break;
		case 15: skill.orbitRadius = bounded; break;
		case 16: skill.orbitAngularSpeedDeg = bounded; break;
		case 17: skill.orbitDurationSec = bounded; break;
		case 18: skill.projectileStartDegree = bounded; break;
		case 19: skill.projectileStartDegreeType = static_cast<int32>(bounded); break;
		case 20: skill.projectileWidth = bounded; break;
		case 21: skill.projectileHeight = bounded; break;
		case 22: skill.swingRadius = bounded; break;
		case 23: skill.swingAngleDeg = bounded; break;
		case 24: skill.bomRadius = bounded; break;
		case 25: skill.bomSelfDamageScale = bounded; break;
		default: break;
		}
		skill.rangeMin = Min(skill.rangeMin, skill.range);
	}

	inline bool TryCommitSkillEditorValueText(SkillDef& skill, int32 row, const String& text)
	{
		if (text.isEmpty())
		{
			return false;
		}

		if (const Optional<double> value = ParseOpt<double>(text))
		{
			SetSkillEditorValue(skill, row, *value);
			return true;
		}

		return false;
	}

	inline void ChangeSkillValue(SkillDef& skill, int32 row, double delta)
	{
		SetSkillEditorValue(skill, row, GetSkillEditorValue(skill, row) + delta);
	}
}
