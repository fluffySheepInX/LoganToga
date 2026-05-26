#pragma once
# include "SkillDefIconRules.h"

namespace LT3
{
	inline String SkillKindToTag(SkillKind kind)
	{
		switch (kind)
		{
		case SkillKind::Sword:
			return U"sword";
		case SkillKind::Heal:
			return U"heal";
		case SkillKind::Summon:
			return U"summon";
		case SkillKind::Charge:
			return U"charge";
		case SkillKind::Status:
			return U"status";
		default:
			return U"missile";
		}
	}

	inline SkillKind ParseSkillKind(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"sword")
		{
			return SkillKind::Sword;
		}
		if (lowered == U"heal")
		{
			return SkillKind::Heal;
		}
		if (lowered == U"summon")
		{
			return SkillKind::Summon;
		}
		if (lowered == U"charge")
		{
			return SkillKind::Charge;
		}
		if (lowered == U"status")
		{
			return SkillKind::Status;
		}
		return SkillKind::Missile;
	}

	inline String SkillProjectileMotionToTag(SkillProjectileMotion motion)
	{
		switch (motion)
		{
		case SkillProjectileMotion::Static:
			return U"static";
		case SkillProjectileMotion::Arc:
			return U"arc";
		case SkillProjectileMotion::Parabola:
			return U"throw";
		case SkillProjectileMotion::Drop:
			return U"drop";
		case SkillProjectileMotion::Orbit:
			return U"circle";
		case SkillProjectileMotion::Swing:
			return U"swing";
		default:
			return U"direct";
		}
	}

	inline String SkillProjectileCenterToTag(SkillProjectileCenter center)
	{
		switch (center)
		{
		case SkillProjectileCenter::On:
			return U"on";
		case SkillProjectileCenter::End:
			return U"end";
		default:
			return U"off";
		}
	}

	inline SkillProjectileMotion ParseSkillProjectileMotion(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"static" || lowered == U"stop" || lowered == U"still")
		{
			return SkillProjectileMotion::Static;
		}
		if (lowered == U"arc")
		{
			return SkillProjectileMotion::Arc;
		}
		if (lowered == U"parabola" || lowered == U"throw")
		{
			return SkillProjectileMotion::Parabola;
		}
		if (lowered == U"drop" || lowered == U"rise")
		{
			return SkillProjectileMotion::Drop;
		}
		if (lowered == U"orbit" || lowered == U"satellite" || lowered == U"circle")
		{
			return SkillProjectileMotion::Orbit;
		}
		if (lowered == U"swing")
		{
			return SkillProjectileMotion::Swing;
		}
		return SkillProjectileMotion::Direct;
	}

	inline SkillProjectileCenter ParseSkillProjectileCenter(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"on" || lowered == U"true" || lowered == U"1")
		{
			return SkillProjectileCenter::On;
		}
		if (lowered == U"end")
		{
			return SkillProjectileCenter::End;
		}
		return SkillProjectileCenter::Off;
	}

	inline String SkillBurstFireModeToTag(SkillBurstFireMode mode)
	{
		switch (mode)
		{
		case SkillBurstFireMode::Staggered:
			return U"staggered";
		default:
			return U"simultaneous";
		}
	}

	inline SkillBurstFireMode ParseSkillBurstFireMode(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"staggered" || lowered == U"interval" || lowered == U"sequential")
		{
			return SkillBurstFireMode::Staggered;
		}

		return SkillBurstFireMode::Simultaneous;
	}

	inline String SkillBurstOrderModeToTag(SkillBurstOrderMode mode)
	{
		switch (mode)
		{
		case SkillBurstOrderMode::Random:
			return U"random";
		default:
			return U"sequential";
		}
	}

	inline SkillBurstOrderMode ParseSkillBurstOrderMode(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"random" || lowered == U"shuffle")
		{
			return SkillBurstOrderMode::Random;
		}

		return SkillBurstOrderMode::Sequential;
	}

	inline String SkillRayModeToTag(SkillRayMode mode)
	{
		switch (mode)
		{
		case SkillRayMode::Image:
			return U"image";
		case SkillRayMode::Line:
			return U"line";
		default:
			return U"none";
		}
	}

	inline SkillRayMode ParseSkillRayMode(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"image")
		{
			return SkillRayMode::Image;
		}
		if (lowered == U"line")
		{
			return SkillRayMode::Line;
		}

		return SkillRayMode::None;
	}

	inline bool ReadSkillBoolSwitch(const TOMLValue& value, bool fallback)
	{
		if (const Optional<bool> boolValue = value.getOpt<bool>())
		{
			return *boolValue;
		}

		const String text = value.getOr<String>(fallback ? U"on" : U"off").lowercased();
		return text == U"on" || text == U"true" || text == U"1" || text == U"yes";
	}

	inline ColorF ReadSkillColor(const TOMLValue& skillValue, const ColorF& fallback)
	{
		return ColorF{
			Clamp(skillValue[U"color_r"].getOr<double>(fallback.r), 0.0, 1.0),
			Clamp(skillValue[U"color_g"].getOr<double>(fallback.g), 0.0, 1.0),
			Clamp(skillValue[U"color_b"].getOr<double>(fallback.b), 0.0, 1.0),
			Clamp(skillValue[U"color_a"].getOr<double>(fallback.a), 0.0, 1.0),
		};
	}
}
