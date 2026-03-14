#include "MapEditScene.h"

Owner MapEditScene::cycleUnitPlacementOwner(const Owner owner)
{
	return (owner == Owner::Player) ? Owner::Enemy : Owner::Player;
}

Owner MapEditScene::cycleResourceOwner(const Owner owner)
{
	switch (owner)
	{
	case Owner::Neutral:
		return Owner::Player;
	case Owner::Player:
		return Owner::Enemy;
	case Owner::Enemy:
	default:
		return Owner::Neutral;
	}
}

UnitArchetype MapEditScene::cycleUnitArchetype(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return UnitArchetype::Barracks;
	case UnitArchetype::Barracks:
		return UnitArchetype::Stable;
	case UnitArchetype::Stable:
		return UnitArchetype::Turret;
	case UnitArchetype::Turret:
		return UnitArchetype::Worker;
	case UnitArchetype::Worker:
		return UnitArchetype::Soldier;
	case UnitArchetype::Soldier:
		return UnitArchetype::Archer;
	case UnitArchetype::Archer:
		return UnitArchetype::Sniper;
	case UnitArchetype::Sniper:
		return UnitArchetype::Katyusha;
	case UnitArchetype::Katyusha:
		return UnitArchetype::MachineGun;
	case UnitArchetype::MachineGun:
		return UnitArchetype::Goliath;
	case UnitArchetype::Goliath:
		return UnitArchetype::Healer;
	case UnitArchetype::Healer:
		return UnitArchetype::Spinner;
	case UnitArchetype::Spinner:
	default:
		return UnitArchetype::Base;
	}
}

ColorF MapEditScene::getOwnerColor(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return ColorF{ 0.24, 0.58, 0.98 };
	case Owner::Enemy:
		return ColorF{ 0.92, 0.26, 0.26 };
	case Owner::Neutral:
	default:
		return ColorF{ 0.78, 0.78, 0.78 };
	}
}

String MapEditScene::toOwnerDisplayString(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return U"Player";
	case Owner::Enemy:
		return U"Enemy";
	case Owner::Neutral:
	default:
		return U"Neutral";
	}
}

String MapEditScene::toOwnerTomlString(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return U"player";
	case Owner::Enemy:
		return U"enemy";
	case Owner::Neutral:
	default:
		return U"neutral";
	}
}

String MapEditScene::toUnitArchetypeDisplayString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"Base";
	case UnitArchetype::Barracks:
		return U"Barracks";
	case UnitArchetype::Stable:
		return U"Stable";
	case UnitArchetype::Turret:
		return U"Turret";
	case UnitArchetype::Worker:
		return U"Worker";
	case UnitArchetype::Soldier:
		return U"Soldier";
	case UnitArchetype::Archer:
		return U"Archer";
	case UnitArchetype::Sniper:
		return U"Sniper";
	case UnitArchetype::Katyusha:
		return U"Katyusha";
	case UnitArchetype::MachineGun:
		return U"MachineGun";
	case UnitArchetype::Goliath:
		return U"Goliath";
	case UnitArchetype::Healer:
		return U"Healer";
	case UnitArchetype::Spinner:
	default:
		return U"Spinner";
	}
}

String MapEditScene::toUnitArchetypeTomlString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"base";
	case UnitArchetype::Barracks:
		return U"barracks";
	case UnitArchetype::Stable:
		return U"stable";
	case UnitArchetype::Turret:
		return U"turret";
	case UnitArchetype::Worker:
		return U"worker";
	case UnitArchetype::Soldier:
		return U"soldier";
	case UnitArchetype::Archer:
		return U"archer";
	case UnitArchetype::Sniper:
		return U"sniper";
	case UnitArchetype::Katyusha:
		return U"katyusha";
	case UnitArchetype::MachineGun:
		return U"machine_gun";
	case UnitArchetype::Goliath:
		return U"goliath";
	case UnitArchetype::Healer:
		return U"healer";
	case UnitArchetype::Spinner:
	default:
		return U"spinner";
	}
}

String MapEditScene::toUnitArchetypeShortString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"B";
	case UnitArchetype::Barracks:
		return U"Br";
	case UnitArchetype::Stable:
		return U"St";
	case UnitArchetype::Turret:
		return U"T";
	case UnitArchetype::Worker:
		return U"W";
	case UnitArchetype::Soldier:
		return U"S";
	case UnitArchetype::Archer:
		return U"A";
	case UnitArchetype::Sniper:
		return U"Sn";
	case UnitArchetype::Katyusha:
		return U"K";
	case UnitArchetype::MachineGun:
		return U"MG";
	case UnitArchetype::Goliath:
		return U"G";
	case UnitArchetype::Healer:
		return U"H";
	case UnitArchetype::Spinner:
	default:
		return U"Sp";
	}
}
