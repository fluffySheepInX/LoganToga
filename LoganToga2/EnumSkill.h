#pragma once
enum class SkillType
{
	missile,
	missileAdd,
	sword,
	heal,
	summon,
	charge,
	status,
};
enum class MoveType
{
	line,
	arc,
	thr,
	drop,
	circle,
	swing
};
enum class SkillEasing { easeOutExpo };
enum class SkillCenter { on, off, end };
enum class SkillBomb { on, off };
enum class SkillD360 { on, off };
enum class SkillForceRay { on, off };
enum class SkillStrKind
{
	none, attack, magic, attack_magic, attack_dext, magic_dext, fix
};
enum class Visibility { Unseen, Seen, Visible };
enum class BattleStatus { Battle, Message, Event };
enum class BattleFormation { F, M, B };
enum class Gender { Neuter = 0, Male = 1, Female = 2, Androgynous = 3, infertile = 4 };
enum class MapTipObjectType { None, WALL2, GATE, HOME };
enum class BattleWhichIsThePlayer { Sortie, Def, None };
/// <summary>
/// 部隊チップの種別
/// flag = 0 なら「ユニットの識別名」として扱う。
/// flag = 1 なら「@文字変数」として扱う。
/// flag = 2 なら「特殊な文字列」として扱う。
/// 
/// ユニットの識別名 同名のunit/class構造体ユニットが配置されます
/// 
/// @文字変数 @が接頭辞の文字列は文字変数と見なされます。代入スクリプトで防衛施設を変化できます
/// 
/// 特殊な文字列
/// 「@」 防衛部隊の位置。
/// 「@@」 侵攻部隊の位置。
/// 「@ESC@」 城兵の退却位置になります。
/// 
/// </summary>
///
enum class FlagBattleMapUnit { Unit, Var, Spe };
