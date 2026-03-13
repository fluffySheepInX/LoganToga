#pragma once

#include "BattleConfigLookup.h"
#include "BattleConfigParsers.h"

inline void LoadBattleUnitConfig(BattleConfigData& config, const TOMLReader& toml)
{
	config.unitDefinitions.clear();
	config.playerProductionSlots.clear();
	config.playerConstructionSlots.clear();
	config.turretUpgradeDefinitions.clear();
	config.playerAvailableProductionArchetypes.clear();
	config.playerAvailableConstructionArchetypes.clear();
	config.playerAvailableTurretUpgrades.clear();

	for (const auto& table : toml[U"units"].tableArrayView())
	{
		UnitDefinition definition;
		definition.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		definition.description = table[U"description"].getOr<String>(U"");
		definition.flavorText = table[U"flavor_text"].getOr<String>(U"");
		definition.radius = table[U"radius"].get<double>();
		definition.moveSpeed = table[U"move_speed"].get<double>();
		definition.attackRange = table[U"attack_range"].get<double>();
		definition.attackCooldown = table[U"attack_cooldown"].get<double>();
		definition.attackPower = table[U"attack_power"].get<int32>();
		definition.hp = table[U"hp"].get<int32>();
		definition.cost = table[U"cost"].get<int32>();
		definition.productionTime = table[U"production_time"].get<double>();
		definition.canMove = table[U"can_move"].get<bool>();
		definition.aggroRange = table[U"aggro_range"].get<double>();
		config.unitDefinitions << definition;
	}

	for (const auto& table : toml[U"player_production"].tableArrayView())
	{
		ProductionSlot slot;
		slot.slot = table[U"slot"].get<int32>();
		slot.producer = ParseUnitArchetype(table[U"producer"].get<String>());
		slot.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		slot.cost = table[U"cost"].getOr<int32>(slot.cost);
		slot.batchCount = Max(1, table[U"batch_count"].getOr<int32>(slot.batchCount));
		config.playerProductionSlots << slot;
		AppendUniqueArchetype(config.playerAvailableProductionArchetypes, slot.archetype);
	}

	for (const auto& table : toml[U"player_construction"].tableArrayView())
	{
		ConstructionSlot slot;
		slot.slot = table[U"slot"].get<int32>();
		slot.archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
		config.playerConstructionSlots << slot;
		AppendUniqueArchetype(config.playerAvailableConstructionArchetypes, slot.archetype);
	}

	for (const auto& table : toml[U"turret_upgrades"].tableArrayView())
	{
		TurretUpgradeDefinition definition;
		definition.type = ParseTurretUpgradeType(table[U"type"].get<String>());
		definition.slot = table[U"slot"].getOr<int32>(definition.slot);
		definition.label = table[U"label"].get<String>();
		definition.glyph = table[U"glyph"].get<String>();
		definition.description = table[U"description"].getOr<String>(U"");
		definition.flavorText = table[U"flavor_text"].getOr<String>(U"");
		definition.cost = table[U"cost"].getOr<int32>(definition.cost);
		definition.attackPowerDelta = table[U"attack_power_delta"].getOr<int32>(0);
		definition.attackCooldownDelta = table[U"attack_cooldown_delta"].getOr<double>(0.0);
		definition.unlockedByDefault = table[U"unlocked_by_default"].getOr<bool>(definition.unlockedByDefault);
		config.turretUpgradeDefinitions << definition;
		if (definition.unlockedByDefault)
		{
			AppendUniqueTurretUpgradeType(config.playerAvailableTurretUpgrades, definition.type);
		}
	}
}
