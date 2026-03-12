#pragma once

#include "RunTypes.h"

[[nodiscard]] inline RewardCardRarity ParseRewardCardRarity(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"common")
	{
		return RewardCardRarity::Common;
	}
	if (normalized == U"rare")
	{
		return RewardCardRarity::Rare;
	}
	if (normalized == U"epic")
	{
		return RewardCardRarity::Epic;
	}

	throw Error{ U"Unknown reward card rarity: " + value };
}

[[nodiscard]] inline RewardCardEffectType ParseRewardCardEffectType(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"unit_stat_bonus")
	{
		return RewardCardEffectType::UnitStatBonus;
	}
	if (normalized == U"production_unlock")
	{
		return RewardCardEffectType::ProductionUnlock;
	}
	if (normalized == U"construction_unlock")
	{
		return RewardCardEffectType::ConstructionUnlock;
	}
	if (normalized == U"turret_upgrade_unlock")
	{
		return RewardCardEffectType::TurretUpgradeUnlock;
	}

	throw Error{ U"Unknown reward card effect type: " + value };
}

[[nodiscard]] inline RewardCardStatType ParseRewardCardStatType(const String& value)
{
	const String normalized = value.lowercased();
	if (normalized == U"hp")
	{
		return RewardCardStatType::HP;
	}
	if (normalized == U"attack_power")
	{
		return RewardCardStatType::AttackPower;
	}
	if (normalized == U"move_speed")
	{
		return RewardCardStatType::MoveSpeed;
	}
	if (normalized == U"attack_range")
	{
		return RewardCardStatType::AttackRange;
	}
	if (normalized == U"production_time")
	{
		return RewardCardStatType::ProductionTime;
	}

	throw Error{ U"Unknown reward card stat type: " + value };
}

[[nodiscard]] inline Array<RewardCardDefinition> LoadRewardCardDefinitions(const String& path)
{
	const TOMLReader toml{ path };
	if (!toml)
	{
		throw Error{ U"Failed to load reward card config: " + path };
	}

	Array<RewardCardDefinition> cards;
	for (const auto& table : toml[U"cards"].tableArrayView())
	{
		RewardCardDefinition card;
		card.id = table[U"id"].get<String>();
		card.name = table[U"name"].get<String>();
		card.description = table[U"description"].get<String>();
		card.rarity = ParseRewardCardRarity(table[U"rarity"].get<String>());
		card.effectType = ParseRewardCardEffectType(table[U"effect_type"].get<String>());
		card.repeatable = table[U"repeatable"].getOr<bool>(false);
		card.value = table[U"value"].getOr<double>(0.0);
		if (card.effectType == RewardCardEffectType::TurretUpgradeUnlock)
		{
			card.targetTurretUpgradeType = ParseTurretUpgradeType(table[U"target_turret_upgrade"].get<String>());
		}
		else
		{
			card.targetArchetype = ParseUnitArchetype(table[U"target_archetype"].get<String>());
		}
		if (card.effectType == RewardCardEffectType::UnitStatBonus)
		{
			card.statType = ParseRewardCardStatType(table[U"stat"].get<String>());
		}
		cards << card;
	}

	return cards;
}
