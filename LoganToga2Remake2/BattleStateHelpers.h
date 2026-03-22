#pragma once

#include "BattleTypes.h"
#include "Localization.h"

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
		|| (archetype == UnitArchetype::Stable)
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
      return Localization::GetText(U"battle.formation.line");
	case FormationType::Column:
      return Localization::GetText(U"battle.formation.column");
	case FormationType::Square:
       return Localization::GetText(U"battle.formation.square");
	default:
        return Localization::GetText(U"battle.formation.panel");
	}
}

[[nodiscard]] inline String GetArchetypeLabel(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
     return Localization::GetText(U"battle.archetype.base");
	case UnitArchetype::Barracks:
     return Localization::GetText(U"battle.archetype.barracks");
	case UnitArchetype::Stable:
       return Localization::GetText(U"battle.archetype.stable");
	case UnitArchetype::Turret:
       return Localization::GetText(U"battle.archetype.turret");
	case UnitArchetype::Worker:
       return Localization::GetText(U"battle.archetype.worker");
	case UnitArchetype::Soldier:
      return Localization::GetText(U"battle.archetype.soldier");
	case UnitArchetype::Archer:
       return Localization::GetText(U"battle.archetype.archer");
	case UnitArchetype::Sniper:
		return Localization::GetText(U"battle.archetype.sniper");
	case UnitArchetype::Katyusha:
		return Localization::GetText(U"battle.archetype.katyusha");
	case UnitArchetype::MachineGun:
        return Localization::GetText(U"battle.archetype.machine_gun");
	case UnitArchetype::Goliath:
      return Localization::GetText(U"battle.archetype.goliath");
	case UnitArchetype::Healer:
       return Localization::GetText(U"battle.archetype.healer");
	case UnitArchetype::Spinner:
      return Localization::GetText(U"battle.archetype.spinner");
	default:
     return Localization::GetText(U"battle.archetype.unit");
	}
}

[[nodiscard]] inline String GetTurretUpgradeLabel(const TurretUpgradeType type)
{
	switch (type)
	{
	case TurretUpgradeType::Power:
        return Localization::GetText(U"battle.upgrade.label.power");
	case TurretUpgradeType::Rapid:
        return Localization::GetText(U"battle.upgrade.label.rapid");
	case TurretUpgradeType::Dual:
     return Localization::GetText(U"battle.upgrade.label.dual");
	default:
      return Localization::GetText(U"battle.upgrade.label.default");
	}
}
