#include "BalanceEditScene.h"

#include "Localization.h"

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
     return Localization::GetText(U"common.unit_archetype.base");
	case UnitArchetype::Barracks:
     return Localization::GetText(U"common.unit_archetype.barracks");
	case UnitArchetype::Stable:
       return Localization::GetText(U"common.unit_archetype.stable");
	case UnitArchetype::Turret:
       return Localization::GetText(U"common.unit_archetype.turret");
	case UnitArchetype::Worker:
       return Localization::GetText(U"common.unit_archetype.worker");
	case UnitArchetype::Soldier:
      return Localization::GetText(U"common.unit_archetype.soldier");
	case UnitArchetype::Archer:
       return Localization::GetText(U"common.unit_archetype.archer");
	case UnitArchetype::Sniper:
       return Localization::GetText(U"common.unit_archetype.sniper");
	case UnitArchetype::Katyusha:
     return Localization::GetText(U"common.unit_archetype.katyusha");
	case UnitArchetype::MachineGun:
       return Localization::GetText(U"common.unit_archetype.machine_gun");
	case UnitArchetype::Goliath:
      return Localization::GetText(U"common.unit_archetype.goliath");
	case UnitArchetype::Healer:
       return Localization::GetText(U"common.unit_archetype.healer");
	case UnitArchetype::Spinner:
	default:
      return Localization::GetText(U"common.unit_archetype.spinner");
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

String BalanceEditScene::toEnemyAiModeDisplayString(const EnemyAiMode mode)
{
	switch (mode)
	{
	case EnemyAiMode::StagingAssault:
		return Localization::GetText(U"battle.enemy_ai_debug.staging");
	case EnemyAiMode::Default:
	default:
		return Localization::GetText(U"battle.enemy_ai_debug.default");
	}
}

String BalanceEditScene::toEnemyAiModeTomlString(const EnemyAiMode mode)
{
	switch (mode)
	{
	case EnemyAiMode::StagingAssault:
		return U"staging_assault";
	case EnemyAiMode::Default:
	default:
		return U"default";
	}
}

String BalanceEditScene::toRewardCardRarityDisplayString(const RewardCardRarity rarity)
{
	switch (rarity)
	{
	case RewardCardRarity::Common:
       return Localization::GetText(U"common.reward_card_rarity.common");
	case RewardCardRarity::Rare:
     return Localization::GetText(U"common.reward_card_rarity.rare");
	case RewardCardRarity::Epic:
	default:
     return Localization::GetText(U"common.reward_card_rarity.epic");
	}
}

String BalanceEditScene::toRewardCardRarityTomlString(const RewardCardRarity rarity)
{
    switch (rarity)
	{
	case RewardCardRarity::Common:
		return U"common";
	case RewardCardRarity::Rare:
		return U"rare";
	case RewardCardRarity::Epic:
	default:
		return U"epic";
	}
}

String BalanceEditScene::toRewardCardEffectTypeDisplayString(const RewardCardEffectType effectType)
{
	switch (effectType)
	{
	case RewardCardEffectType::UnitStatBonus:
      return Localization::GetText(U"balance_edit.effect_type.unit_stat_bonus");
	case RewardCardEffectType::ProductionUnlock:
        return Localization::GetText(U"balance_edit.effect_type.production_unlock");
	case RewardCardEffectType::ConstructionUnlock:
      return Localization::GetText(U"balance_edit.effect_type.construction_unlock");
	case RewardCardEffectType::TurretUpgradeUnlock:
	default:
        return Localization::GetText(U"balance_edit.effect_type.turret_upgrade_unlock");
	}
}

String BalanceEditScene::toRewardCardStatTypeDisplayString(const RewardCardStatType statType)
{
	switch (statType)
	{
	case RewardCardStatType::HP:
		return U"HP";
	case RewardCardStatType::AttackPower:
     return Localization::GetText(U"balance_edit.stat_type.attack_power");
	case RewardCardStatType::MoveSpeed:
       return Localization::GetText(U"balance_edit.stat_type.move_speed");
	case RewardCardStatType::AttackRange:
     return Localization::GetText(U"balance_edit.stat_type.attack_range");
	case RewardCardStatType::ProductionTime:
	default:
      return Localization::GetText(U"balance_edit.stat_type.production_time");
	}
}

String BalanceEditScene::toTurretUpgradeTypeDisplayString(const TurretUpgradeType type)
{
	switch (type)
	{
	case TurretUpgradeType::Power:
        return Localization::GetText(U"common.turret_upgrade.power");
	case TurretUpgradeType::Rapid:
        return Localization::GetText(U"common.turret_upgrade.rapid");
	case TurretUpgradeType::Dual:
	default:
     return Localization::GetText(U"common.turret_upgrade.dual");
	}
}
