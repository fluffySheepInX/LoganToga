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

	inline const Array<String>& SkillBomVisualLabels()
	{
		static const Array<String> labels = { U"circle", U"image" };
		return labels;
	}

	inline const Array<String>& SkillEditorValueHelpTexts()
	{
		static const Array<String> helpTexts = {
			U"射程。対象を探してスキルを使える距離です。",
			U"最小射程。この距離より内側に対象がいる間は発射しません。0なら制限なしです。",
			U"クールダウン秒。次に同じスキルを使えるまでの待ち時間です。",
			U"ダメージ量。負数は回復などの特殊用途に使えます。",
			U"命中時に術者自身へ入る固定ダメージです。地雷や自爆スキル用です。",
			U"弾や効果の移動速度です。motion によっては影響が小さい場合があります。",
			U"1回の発動で出す弾や効果の数です。",
			U"burst発射方式。simulは同時発射、staggerは時間差発射です。",
			U"burstの発射順。seqは左端から順、randomは毎burstで順序をシャッフルします。",
			U"missile専用の残像表現。none / image / line を切り替えます。",
			U"rayの長さ倍率です。line/imageの伸び方を調整します。",
			U"line rayの末尾を術者中心へ固定します。レーザーのように途切れにくくなります。",
			U"burstを時間差発射する時の弾間隔（秒）です。0なら同時相当です。",
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
			U"爆風半径です。bom=on の時に着弾地点からこの半径内へ同じダメージを与えます。距離減衰はありません。",
			U"爆風による自爆倍率です。爆風の基準ダメージへ倍率を掛けて術者自身へ与えます。",
			U"swing の命中方式です。stop は最初の接触で終了、multi_hit_once は振り切りつつ同一対象への再ヒットを防ぎます。",
			U"bom の見た目モードです。0=circle, 1=image。ダメージ判定は bom_radius 側で変わりません。",
			U"bom 画像の描画倍率です。円の代わりに画像/GIF を表示する時の見た目倍率です。",
			U"bom 画像/GIF の表示秒数です。GIF でもこの秒数を超えたら演出を終了します。",
		};
		return helpTexts;
	}

	inline String SkillEditorToggleHelpText(int32 index)
	{
		switch (index)
		{
		case 0:
			return U"homing。飛翔中に対象位置へ進行方向を補正し続けます。";
		case 1:
			return U"d360。全方位向き画像として扱う補助フラグです。";
		case 2:
			return U"bom。命中時に bom_radius 内へ距離減衰なしの範囲ダメージを与えます。";
		case 3:
			return U"ff。bom 時に味方も爆風対象へ含めます。";
		case 4:
			return U"allfunc。射程範囲内の有効対象すべてに向けて同時に処理します。heal は味方、通常は敵を対象にします。";
		case 5:
			return U"next_last。ヒットしなくても寿命切れや対象喪失で消滅した時に next を発生させます。";
		case 6:
			return U"joint_skill。next へのつなぎ用スキルであることを示す編集用フラグです。";
		case 7:
			return U"send_target。親スキルの発射時標的位置を next スキルへ渡します。進行方向は next 側で再計算します。";
		case 8:
			return U"send_image_degree。親スキル画像の向き角度を next スキルの画像向きへ渡します。";
		default:
			return U"";
		}
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
}
