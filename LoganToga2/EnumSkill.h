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
