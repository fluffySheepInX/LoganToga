#pragma once

#include "BattleTypes.h"

[[nodiscard]] inline Vec2 ClampToWorld(const RectF& bounds, const Vec2& position, const double radius)
{
	return {
		Clamp(position.x, bounds.leftX() + radius, bounds.rightX() - radius),
		Clamp(position.y, bounds.topY() + radius, bounds.bottomY() - radius)
	};
}

template <class TUnitState>
[[nodiscard]] inline bool IsEnemy(const TUnitState& lhs, const TUnitState& rhs)
{
	return (lhs.owner != rhs.owner);
}

[[nodiscard]] inline bool IsBuildingArchetype(const UnitArchetype archetype)
{
	return (archetype == UnitArchetype::Base)
		|| (archetype == UnitArchetype::Barracks)
		|| (archetype == UnitArchetype::Turret);
}

[[nodiscard]] inline ColorF GetOwnerColor(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return ColorF{ 0.25, 0.75, 1.0 };
	case Owner::Enemy:
		return ColorF{ 1.0, 0.38, 0.32 };
	default:
		return ColorF{ 0.75, 0.78, 0.84 };
	}
}

[[nodiscard]] inline String GetFormationLabel(const FormationType formation)
{
	switch (formation)
	{
	case FormationType::Line:
		return U"CLUSTER";
	case FormationType::Column:
		return U"ROW";
	case FormationType::Square:
		return U"SQUARE";
	default:
		return U"FORMATION";
	}
}

[[nodiscard]] inline String GetArchetypeLabel(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"BASE";
	case UnitArchetype::Barracks:
		return U"BARRACKS";
	case UnitArchetype::Turret:
		return U"TURRET";
	case UnitArchetype::Worker:
		return U"WORKER";
	case UnitArchetype::Soldier:
		return U"SOLDIER";
	case UnitArchetype::Archer:
		return U"ARCHER";
	default:
		return U"UNIT";
	}
}

[[nodiscard]] inline String GetTurretUpgradeLabel(const TurretUpgradeType type)
{
	switch (type)
	{
	case TurretUpgradeType::Power:
		return U"POWER";
	case TurretUpgradeType::Rapid:
		return U"RAPID";
	case TurretUpgradeType::Dual:
		return U"DUAL";
	default:
		return U"UPGRADE";
	}
}
