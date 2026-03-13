#pragma once

#include "BattleConfigTypes.h"

[[nodiscard]] inline UnitArchetype ParseUnitArchetype(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"base")
	{
		return UnitArchetype::Base;
	}
	if (normalized == U"barracks")
	{
		return UnitArchetype::Barracks;
	}
	if (normalized == U"stable")
	{
		return UnitArchetype::Stable;
	}
	if (normalized == U"turret")
	{
		return UnitArchetype::Turret;
	}
	if (normalized == U"worker")
	{
		return UnitArchetype::Worker;
	}
	if (normalized == U"soldier")
	{
		return UnitArchetype::Soldier;
	}
	if (normalized == U"archer")
	{
		return UnitArchetype::Archer;
	}
	if ((normalized == U"sniper") || (normalized == U"marksman"))
	{
		return UnitArchetype::Sniper;
	}
	if ((normalized == U"katyusha") || (normalized == U"rocket_truck") || (normalized == U"rockettruck"))
	{
		return UnitArchetype::Katyusha;
	}
	if ((normalized == U"machine_gun") || (normalized == U"machinegun") || (normalized == U"mgun"))
	{
		return UnitArchetype::MachineGun;
	}
	if ((normalized == U"goliath") || (normalized == U"demolition_cart") || (normalized == U"bomb_cart"))
	{
		return UnitArchetype::Goliath;
	}
	if ((normalized == U"healer") || (normalized == U"medic"))
	{
		return UnitArchetype::Healer;
	}
	if ((normalized == U"spinner") || (normalized == U"top_rider"))
	{
		return UnitArchetype::Spinner;
	}

	throw Error{ U"Unknown unit archetype: " + value };
}

[[nodiscard]] inline Owner ParseOwner(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"player")
	{
		return Owner::Player;
	}
	if (normalized == U"enemy")
	{
		return Owner::Enemy;
	}
	if (normalized == U"neutral")
	{
		return Owner::Neutral;
	}

	throw Error{ U"Unknown owner: " + value };
}

[[nodiscard]] inline EnemyAiMode ParseEnemyAiMode(const String& value)
{
	const String normalized = value.lowercased();
	if ((normalized == U"default") || (normalized == U"basic"))
	{
		return EnemyAiMode::Default;
	}
	if ((normalized == U"staging_assault") || (normalized == U"staging") || (normalized == U"group_assault"))
	{
		return EnemyAiMode::StagingAssault;
	}

	throw Error{ U"Unknown enemy AI mode: " + value };
}

[[nodiscard]] inline TurretUpgradeType ParseTurretUpgradeType(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"power")
	{
		return TurretUpgradeType::Power;
	}
	if ((normalized == U"rapid") || (normalized == U"speed"))
	{
		return TurretUpgradeType::Rapid;
	}
	if ((normalized == U"dual") || (normalized == U"hybrid") || (normalized == U"both"))
	{
		return TurretUpgradeType::Dual;
	}

	throw Error{ U"Unknown turret upgrade type: " + value };
}
