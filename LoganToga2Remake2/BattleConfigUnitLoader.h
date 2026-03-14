#pragma once

#include "BattleConfigLookup.h"
#include "BattleConfigParsers.h"

[[nodiscard]] inline UnitDefinition* FindMutableUnitDefinition(BattleConfigData& config, const UnitArchetype archetype)
{
	for (auto& definition : config.unitDefinitions)
	{
		if (definition.archetype == archetype)
		{
			return &definition;
		}
	}

	return nullptr;
}

[[nodiscard]] inline ProductionSlot* FindMutableProductionSlot(BattleConfigData& config, const int32 slotNumber)
{
	for (auto& slot : config.playerProductionSlots)
	{
		if (slot.slot == slotNumber)
		{
			return &slot;
		}
	}

	return nullptr;
}

[[nodiscard]] inline ConstructionSlot* FindMutableConstructionSlot(BattleConfigData& config, const int32 slotNumber)
{
	for (auto& slot : config.playerConstructionSlots)
	{
		if (slot.slot == slotNumber)
		{
			return &slot;
		}
	}

	return nullptr;
}

[[nodiscard]] inline TurretUpgradeDefinition* FindMutableTurretUpgradeDefinition(BattleConfigData& config, const TurretUpgradeType type)
{
	for (auto& definition : config.turretUpgradeDefinitions)
	{
		if (definition.type == type)
		{
			return &definition;
		}
	}

	return nullptr;
}

inline void RefreshBattleUnitAvailability(BattleConfigData& config)
{
	config.playerAvailableProductionArchetypes.clear();
	config.playerAvailableConstructionArchetypes.clear();
	config.playerAvailableTurretUpgrades.clear();

	for (const auto& slot : config.playerProductionSlots)
	{
		AppendUniqueArchetype(config.playerAvailableProductionArchetypes, slot.archetype);
	}

	for (const auto& slot : config.playerConstructionSlots)
	{
		AppendUniqueArchetype(config.playerAvailableConstructionArchetypes, slot.archetype);
	}

	for (const auto& definition : config.turretUpgradeDefinitions)
	{
		if (definition.unlockedByDefault)
		{
			AppendUniqueTurretUpgradeType(config.playerAvailableTurretUpgrades, definition.type);
		}
	}
}

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

inline void ApplyBattleUnitConfigOverrides(BattleConfigData& config, const TOMLReader& toml)
{
	if (toml[U"units"].isTableArray())
	{
		for (const auto& table : toml[U"units"].tableArrayView())
		{
			const UnitArchetype archetype = ParseUnitArchetype(table[U"archetype"].get<String>());
			auto* definition = FindMutableUnitDefinition(config, archetype);
			if (!definition)
			{
				config.unitDefinitions << UnitDefinition{};
				definition = &config.unitDefinitions.back();
				definition->archetype = archetype;
			}

			definition->description = table[U"description"].getOr<String>(definition->description);
			definition->flavorText = table[U"flavor_text"].getOr<String>(definition->flavorText);
			definition->radius = table[U"radius"].getOr<double>(definition->radius);
			definition->moveSpeed = table[U"move_speed"].getOr<double>(definition->moveSpeed);
			definition->attackRange = table[U"attack_range"].getOr<double>(definition->attackRange);
			definition->attackCooldown = table[U"attack_cooldown"].getOr<double>(definition->attackCooldown);
			definition->attackPower = table[U"attack_power"].getOr<int32>(definition->attackPower);
			definition->hp = table[U"hp"].getOr<int32>(definition->hp);
			definition->cost = table[U"cost"].getOr<int32>(definition->cost);
			definition->productionTime = table[U"production_time"].getOr<double>(definition->productionTime);
			definition->canMove = table[U"can_move"].getOr<bool>(definition->canMove);
			definition->aggroRange = table[U"aggro_range"].getOr<double>(definition->aggroRange);
		}
	}

	if (toml[U"player_production"].isTableArray())
	{
		for (const auto& table : toml[U"player_production"].tableArrayView())
		{
			const int32 slotNumber = table[U"slot"].get<int32>();
			auto* slot = FindMutableProductionSlot(config, slotNumber);
			const bool isNewSlot = (slot == nullptr);
			if (!slot)
			{
				config.playerProductionSlots << ProductionSlot{};
				slot = &config.playerProductionSlots.back();
				slot->slot = slotNumber;
			}

			if (const auto producerValue = table[U"producer"].getOpt<String>())
			{
				slot->producer = ParseUnitArchetype(*producerValue);
			}
			else if (isNewSlot)
			{
				throw Error{ U"Production override requires producer for slot: " + Format(slotNumber) };
			}

			if (const auto archetypeValue = table[U"archetype"].getOpt<String>())
			{
				slot->archetype = ParseUnitArchetype(*archetypeValue);
			}
			else if (isNewSlot)
			{
				throw Error{ U"Production override requires archetype for slot: " + Format(slotNumber) };
			}

			slot->cost = table[U"cost"].getOr<int32>(slot->cost);
			slot->batchCount = Max(1, table[U"batch_count"].getOr<int32>(slot->batchCount));
		}
	}

	if (toml[U"player_construction"].isTableArray())
	{
		for (const auto& table : toml[U"player_construction"].tableArrayView())
		{
			const int32 slotNumber = table[U"slot"].get<int32>();
			auto* slot = FindMutableConstructionSlot(config, slotNumber);
			const bool isNewSlot = (slot == nullptr);
			if (!slot)
			{
				config.playerConstructionSlots << ConstructionSlot{};
				slot = &config.playerConstructionSlots.back();
				slot->slot = slotNumber;
			}

			if (const auto archetypeValue = table[U"archetype"].getOpt<String>())
			{
				slot->archetype = ParseUnitArchetype(*archetypeValue);
			}
			else if (isNewSlot)
			{
				throw Error{ U"Construction override requires archetype for slot: " + Format(slotNumber) };
			}
		}
	}

	if (toml[U"turret_upgrades"].isTableArray())
	{
		for (const auto& table : toml[U"turret_upgrades"].tableArrayView())
		{
			const TurretUpgradeType type = ParseTurretUpgradeType(table[U"type"].get<String>());
			auto* definition = FindMutableTurretUpgradeDefinition(config, type);
			if (!definition)
			{
				config.turretUpgradeDefinitions << TurretUpgradeDefinition{};
				definition = &config.turretUpgradeDefinitions.back();
				definition->type = type;
			}

			definition->slot = table[U"slot"].getOr<int32>(definition->slot);
			definition->label = table[U"label"].getOr<String>(definition->label);
			definition->glyph = table[U"glyph"].getOr<String>(definition->glyph);
			definition->description = table[U"description"].getOr<String>(definition->description);
			definition->flavorText = table[U"flavor_text"].getOr<String>(definition->flavorText);
			definition->cost = table[U"cost"].getOr<int32>(definition->cost);
			definition->attackPowerDelta = table[U"attack_power_delta"].getOr<int32>(definition->attackPowerDelta);
			definition->attackCooldownDelta = table[U"attack_cooldown_delta"].getOr<double>(definition->attackCooldownDelta);
			definition->unlockedByDefault = table[U"unlocked_by_default"].getOr<bool>(definition->unlockedByDefault);
		}
	}

	RefreshBattleUnitAvailability(config);
}
