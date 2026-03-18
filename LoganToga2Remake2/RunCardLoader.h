#pragma once

#include "BattleConfigPathResolver.h"
#include "RunTypes.h"

[[nodiscard]] inline RewardCardDefinition* FindMutableRewardCardDefinition(Array<RewardCardDefinition>& cards, const String& id)
{
	for (auto& card : cards)
	{
		if (card.id == id)
		{
			return &card;
		}
	}

	return nullptr;
}

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

inline void ApplyRewardCardDefinitionOverrides(Array<RewardCardDefinition>& cards, const TOMLReader& toml)
{
	if (!toml[U"cards"].isTableArray())
	{
		return;
	}

	for (const auto& table : toml[U"cards"].tableArrayView())
	{
		const String id = table[U"id"].get<String>();
		auto* card = FindMutableRewardCardDefinition(cards, id);
		if (!card)
		{
			cards << RewardCardDefinition{};
			card = &cards.back();
			card->id = id;
		}

		card->name = table[U"name"].getOr<String>(card->name);
     card->nameJa = table[U"name_ja"].getOr<String>(card->nameJa);
		card->nameEn = table[U"name_en"].getOr<String>(card->nameEn);
		card->description = table[U"description"].getOr<String>(card->description);
     card->descriptionJa = table[U"description_ja"].getOr<String>(card->descriptionJa);
		card->descriptionEn = table[U"description_en"].getOr<String>(card->descriptionEn);
		if (const auto rarityValue = table[U"rarity"].getOpt<String>())
		{
			card->rarity = ParseRewardCardRarity(*rarityValue);
		}
		if (const auto effectTypeValue = table[U"effect_type"].getOpt<String>())
		{
			card->effectType = ParseRewardCardEffectType(*effectTypeValue);
		}
		card->repeatable = table[U"repeatable"].getOr<bool>(card->repeatable);
		card->value = table[U"value"].getOr<double>(card->value);
		if (const auto archetypeValue = table[U"target_archetype"].getOpt<String>())
		{
			card->targetArchetype = ParseUnitArchetype(*archetypeValue);
		}
		if (const auto upgradeValue = table[U"target_turret_upgrade"].getOpt<String>())
		{
			card->targetTurretUpgradeType = ParseTurretUpgradeType(*upgradeValue);
		}
		if (const auto statValue = table[U"stat"].getOpt<String>())
		{
			card->statType = ParseRewardCardStatType(*statValue);
		}
	}
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
     card.nameJa = table[U"name_ja"].getOr<String>(card.name);
		card.nameEn = table[U"name_en"].getOr<String>(card.name);
		card.description = table[U"description"].get<String>();
        card.descriptionJa = table[U"description_ja"].getOr<String>(card.description);
		card.descriptionEn = table[U"description_en"].getOr<String>(card.description);
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

	if (HasTomlOverride(path))
	{
		const String overridePath = ResolveTomlOverridePath(path);
		const TOMLReader overrideToml{ overridePath };
		if (!overrideToml)
		{
			throw Error{ U"Failed to load reward card override config: " + overridePath };
		}

		ApplyRewardCardDefinitionOverrides(cards, overrideToml);
	}

	if (HasTomlEditorOverride(path))
	{
		const String editorOverridePath = ResolveTomlEditorOverridePath(path);
		const TOMLReader editorOverrideToml{ editorOverridePath };
		if (!editorOverrideToml)
		{
			throw Error{ U"Failed to load reward card editor override config: " + editorOverridePath };
		}

		ApplyRewardCardDefinitionOverrides(cards, editorOverrideToml);
	}

	return cards;
}
