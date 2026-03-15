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

String BalanceEditScene::toRewardCardRarityDisplayString(const RewardCardRarity rarity)
{
	switch (rarity)
	{
	case RewardCardRarity::Common:
		return U"Common";
	case RewardCardRarity::Rare:
		return U"Rare";
	case RewardCardRarity::Epic:
	default:
		return U"Epic";
	}
}

String BalanceEditScene::toRewardCardRarityTomlString(const RewardCardRarity rarity)
{
	return toRewardCardRarityDisplayString(rarity).lowercased();
}

String BalanceEditScene::toRewardCardEffectTypeDisplayString(const RewardCardEffectType effectType)
{
	switch (effectType)
	{
	case RewardCardEffectType::UnitStatBonus:
		return U"Unit Stat Bonus";
	case RewardCardEffectType::ProductionUnlock:
		return U"Production Unlock";
	case RewardCardEffectType::ConstructionUnlock:
		return U"Construction Unlock";
	case RewardCardEffectType::TurretUpgradeUnlock:
	default:
		return U"Turret Upgrade Unlock";
	}
}

String BalanceEditScene::toRewardCardStatTypeDisplayString(const RewardCardStatType statType)
{
	switch (statType)
	{
	case RewardCardStatType::HP:
		return U"HP";
	case RewardCardStatType::AttackPower:
		return U"Attack Power";
	case RewardCardStatType::MoveSpeed:
		return U"Move Speed";
	case RewardCardStatType::AttackRange:
		return U"Attack Range";
	case RewardCardStatType::ProductionTime:
	default:
		return U"Production Time";
	}
}

String BalanceEditScene::toTurretUpgradeTypeDisplayString(const TurretUpgradeType type)
{
	switch (type)
	{
	case TurretUpgradeType::Power:
		return U"Power";
	case TurretUpgradeType::Rapid:
		return U"Rapid";
	case TurretUpgradeType::Dual:
	default:
		return U"Dual";
	}
}
