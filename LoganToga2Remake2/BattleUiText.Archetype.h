#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleUiText
{
	[[nodiscard]] inline String GetLocalizedArchetypeLabel(const UnitArchetype archetype)
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

	[[nodiscard]] inline String GetLocalizedArchetypeDescription(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
			return Localization::GetText(U"battle.archetype_desc.base");
		case UnitArchetype::Barracks:
			return Localization::GetText(U"battle.archetype_desc.barracks");
		case UnitArchetype::Stable:
			return Localization::GetText(U"battle.archetype_desc.stable");
		case UnitArchetype::Turret:
			return Localization::GetText(U"battle.archetype_desc.turret");
		case UnitArchetype::Worker:
			return Localization::GetText(U"battle.archetype_desc.worker");
		case UnitArchetype::Soldier:
			return Localization::GetText(U"battle.archetype_desc.soldier");
		case UnitArchetype::Archer:
			return Localization::GetText(U"battle.archetype_desc.archer");
		case UnitArchetype::Sniper:
			return Localization::GetText(U"battle.archetype_desc.sniper");
		case UnitArchetype::Katyusha:
			return Localization::GetText(U"battle.archetype_desc.katyusha");
		case UnitArchetype::MachineGun:
			return Localization::GetText(U"battle.archetype_desc.machine_gun");
		case UnitArchetype::Goliath:
			return Localization::GetText(U"battle.archetype_desc.goliath");
		case UnitArchetype::Healer:
			return Localization::GetText(U"battle.archetype_desc.healer");
		case UnitArchetype::Spinner:
			return Localization::GetText(U"battle.archetype_desc.spinner");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedArchetypeFlavorText(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Stable:
			return Localization::GetText(U"battle.archetype_flavor.stable");
		case UnitArchetype::Turret:
			return Localization::GetText(U"battle.archetype_flavor.turret");
		case UnitArchetype::Barracks:
			return Localization::GetText(U"battle.archetype_flavor.barracks");
		case UnitArchetype::Soldier:
			return Localization::GetText(U"battle.archetype_flavor.soldier");
		case UnitArchetype::Archer:
			return Localization::GetText(U"battle.archetype_flavor.archer");
		case UnitArchetype::Sniper:
			return Localization::GetText(U"battle.archetype_flavor.sniper");
		case UnitArchetype::Katyusha:
			return Localization::GetText(U"battle.archetype_flavor.katyusha");
		case UnitArchetype::MachineGun:
			return Localization::GetText(U"battle.archetype_flavor.machine_gun");
		case UnitArchetype::Goliath:
			return Localization::GetText(U"battle.archetype_flavor.goliath");
		case UnitArchetype::Healer:
			return Localization::GetText(U"battle.archetype_flavor.healer");
		case UnitArchetype::Spinner:
			return Localization::GetText(U"battle.archetype_flavor.spinner");
		default:
			return U"";
		}
	}
}
