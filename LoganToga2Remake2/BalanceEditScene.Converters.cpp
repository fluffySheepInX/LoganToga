#include "BalanceEditScene.h"

void BalanceEditScene::appendTomlLine(String& content, const String& key, const String& value)
{
	content += key + U" = " + value + U"\n";
}

String BalanceEditScene::quoteTomlString(const String& value)
{
	String escaped = value;
	escaped.replace(U"\\", U"\\\\");
	escaped.replace(U"\"", U"\\\"");
	return U"\"" + escaped + U"\"";
}

String BalanceEditScene::toUnitArchetypeDisplayString(const UnitArchetype archetype)
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

String BalanceEditScene::toUnitArchetypeTomlString(const UnitArchetype archetype)
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
