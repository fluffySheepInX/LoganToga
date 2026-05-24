#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "../Data/Loaders/SkillDefLoader.h"

namespace LT3
{
	inline const Array<String>& SkillKindLabels()
	{
		static const Array<String> labels = { U"missile", U"sword", U"heal", U"summon", U"charge", U"status" };
		return labels;
	}

	inline const Array<String>& SkillCenterLabels()
	{
		static const Array<String> labels = { U"off", U"on", U"end" };
		return labels;
	}

	inline const Array<String>& SkillMotionLabels()
	{
		static const Array<String> labels = { U"direct", U"static", U"arc", U"throw", U"drop", U"circle", U"swing" };
		return labels;
	}

	inline const Array<String>& SkillEditorValueHelpTexts()
	{
		static const Array<String> helpTexts = {
			U"射程。対象を探してスキルを使える距離です。",
			U"クールダウン秒。次に同じスキルを使えるまでの待ち時間です。",
			U"ダメージ量。負数は回復などの特殊用途に使えます。",
			U"弾や効果の移動速度です。motion によっては影響が小さい場合があります。",
			U"1回の発動で出す弾や効果の数です。",
			U"複数発射時の広がり角度です。",
			U"arc / throw / drop 系の山なり高さです。画像サイズには使いません。",
			U"circle 系の回転半径です。swing の半径は swingR を使います。",
			U"orbit / circle 系の角速度（度/秒）です。",
			U"軌道・持続系の効果が残る秒数です。",
			U"発射開始角度の補正値です。",
			U"開始角度の解釈タイプです。0～9で処理側の方式を切り替えます。",
			U"画像の描画幅です。Swingでは見た目の太さとして扱います。",
			U"画像の高さです。Swing + center=end ではワールド上の剣の長さ・到達長として扱います。",
			U"swing 専用の回転半径です。0 なら術者中心を基準に振ります。",
			U"swing 専用の旋回角度です。負数にすると逆方向へ振ります。",
		};
		return helpTexts;
	}

	inline int32 SkillKindIndex(SkillKind kind)
	{
		return static_cast<int32>(kind);
	}

	inline int32 SkillMotionIndex(SkillProjectileMotion motion)
	{
		return static_cast<int32>(motion);
	}

	inline int32 SkillCenterIndex(SkillProjectileCenter center)
	{
		return static_cast<int32>(center);
	}

	inline bool HasSelectedSkill(const MapEditorState& editor, const DefinitionStores& defs)
	{
		return 0 <= editor.selectedSkillIndex && editor.selectedSkillIndex < static_cast<int32>(defs.skills.size());
	}

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
		if (row == 14 && skill.projectileMotion == SkillProjectileMotion::Swing)
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
		case 9:
			return 0.1;
		case 4:
		case 11:
			return 1.0;
		default:
			return 1.0;
		}
	}

	inline void EnsureSkillEditorValueSteps(MapEditorState& editor)
	{
		if (editor.skillValueSteps.size() == 16)
		{
			return;
		}

		editor.skillValueSteps.resize(16);
		for (int32 row = 0; row < 16; ++row)
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
		const double current = SkillEditorValueStep(editor, row);
		int32 nextIndex = 0;
		for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
		{
			if (Math::Abs(steps[i] - current) < 0.0001)
			{
				nextIndex = (i + 1) % static_cast<int32>(steps.size());
				break;
			}
		}

		SetSkillEditorValueStep(editor, row, steps[nextIndex]);
	}

	inline double ApplySkillEditorValueBounds(int32 row, double value)
	{
		switch (row)
		{
		case 0:
			return Max(0.0, value);
		case 1:
			return Max(0.05, value);
		case 2:
			return Clamp(value, 0.01, 100.0);
		case 3:
			return Max(1.0, value);
		case 4:
			return static_cast<double>(Clamp(static_cast<int32>(value), 1, 32));
		case 5:
			return Clamp(value, 0.0, 180.0);
		case 6:
			return Max(0.0, value);
		case 7:
			return Max(1.0, value);
		case 9:
			return Max(0.05, value);
		case 10:
			return Clamp(value, -360.0, 360.0);
		case 11:
			return static_cast<double>(Clamp(static_cast<int32>(value), 0, 9));
		case 12:
		case 13:
			return Max(1.0, value);
		case 14:
			return Max(0.0, value);
		case 15:
			return Clamp(value, -720.0, 720.0);
		default:
			return value;
		}
	}

	inline double GetSkillEditorValue(const SkillDef& skill, int32 row)
	{
		switch (row)
		{
		case 0: return skill.range;
		case 1: return skill.cooldownSec;
		case 2: return skill.damage;
		case 3: return skill.projectileSpeed;
		case 4: return static_cast<double>(skill.burstCount);
		case 5: return skill.spreadDeg;
		case 6: return skill.arcHeight;
		case 7: return skill.orbitRadius;
		case 8: return skill.orbitAngularSpeedDeg;
		case 9: return skill.orbitDurationSec;
		case 10: return skill.projectileStartDegree;
		case 11: return static_cast<double>(skill.projectileStartDegreeType);
		case 12: return skill.projectileWidth;
		case 13: return skill.projectileHeight;
		case 14: return skill.swingRadius;
		case 15: return skill.swingAngleDeg;
		default: return 0.0;
		}
	}

	inline void SetSkillEditorValue(SkillDef& skill, int32 row, double value)
	{
		const double bounded = ApplySkillEditorValueBounds(row, value);
		switch (row)
		{
		case 0: skill.range = bounded; break;
		case 1: skill.cooldownSec = bounded; break;
		case 2: skill.damage = bounded; break;
		case 3: skill.projectileSpeed = bounded; break;
		case 4: skill.burstCount = static_cast<int32>(bounded); break;
		case 5: skill.spreadDeg = bounded; break;
		case 6: skill.arcHeight = bounded; break;
		case 7: skill.orbitRadius = bounded; break;
		case 8: skill.orbitAngularSpeedDeg = bounded; break;
		case 9: skill.orbitDurationSec = bounded; break;
		case 10: skill.projectileStartDegree = bounded; break;
		case 11: skill.projectileStartDegreeType = static_cast<int32>(bounded); break;
		case 12: skill.projectileWidth = bounded; break;
		case 13: skill.projectileHeight = bounded; break;
		case 14: skill.swingRadius = bounded; break;
		case 15: skill.swingAngleDeg = bounded; break;
		default: break;
		}
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

	inline const Array<String>* FindSkillIconWarnings(const DefinitionStores& defs, StringView skillTag)
	{
		if (defs.skillIconWarningsByTag.contains(String{ skillTag }))
		{
			return &defs.skillIconWarningsByTag.at(String{ skillTag });
		}

		return nullptr;
	}

	inline void SaveSkillEditorDefinitions(MapEditorState& editor, const DefinitionStores& defs)
	{
		String status;
		if (SaveSkillDefinitionsToml(defs.skills, &status))
		{
			editor.skillDefsDirty = true;
		}
		editor.statusText = status;
	}

	inline void ChangeSkillValue(SkillDef& skill, int32 row, double delta)
	{
		SetSkillEditorValue(skill, row, GetSkillEditorValue(skill, row) + delta);
	}
}
