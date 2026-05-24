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
		switch (row)
		{
		case 0:
			skill.range = Max(0.0, skill.range + delta);
			break;
		case 1:
			skill.cooldownSec = Max(0.05, skill.cooldownSec + delta);
			break;
		case 2:
			skill.damage = Clamp(skill.damage + static_cast<int32>(delta), -999, 999);
			break;
		case 3:
			skill.projectileSpeed = Max(1.0, skill.projectileSpeed + delta);
			break;
		case 4:
			skill.burstCount = Clamp(skill.burstCount + static_cast<int32>(delta), 1, 32);
			break;
		case 5:
			skill.spreadDeg = Clamp(skill.spreadDeg + delta, 0.0, 180.0);
			break;
		case 6:
			skill.arcHeight = Max(0.0, skill.arcHeight + delta);
			break;
		case 7:
			skill.orbitRadius = Max(1.0, skill.orbitRadius + delta);
			break;
		case 8:
			skill.orbitAngularSpeedDeg += delta;
			break;
		case 9:
			skill.orbitDurationSec = Max(0.05, skill.orbitDurationSec + delta);
			break;
		case 10:
			skill.projectileStartDegree = Clamp(skill.projectileStartDegree + delta, -360.0, 360.0);
			break;
		case 11:
			skill.projectileStartDegreeType = Clamp(skill.projectileStartDegreeType + static_cast<int32>(delta), 0, 9);
			break;
		case 12:
			skill.projectileWidth = Max(1.0, skill.projectileWidth + delta);
			break;
		case 13:
			skill.projectileHeight = Max(1.0, skill.projectileHeight + delta);
			break;
		case 14:
			skill.swingRadius = Max(0.0, skill.swingRadius + delta);
			break;
		case 15:
			skill.swingAngleDeg = Clamp(skill.swingAngleDeg + delta, -720.0, 720.0);
			break;
		default:
			break;
		}
	}
}
