#pragma once

#include "BattleConfig.h"

enum class RewardCardRarity
{
	Common,
	Rare,
	Epic
};

enum class RewardCardEffectType
{
	UnitStatBonus,
	ProductionUnlock,
	ConstructionUnlock,
	TurretUpgradeUnlock
};

enum class RewardCardStatType
{
	HP,
	AttackPower,
	MoveSpeed,
	AttackRange,
	ProductionTime
};

struct RewardCardDefinition
{
	String id;
	String name;
	String description;
	RewardCardRarity rarity = RewardCardRarity::Common;
	RewardCardEffectType effectType = RewardCardEffectType::UnitStatBonus;
	UnitArchetype targetArchetype = UnitArchetype::Soldier;
	Optional<TurretUpgradeType> targetTurretUpgradeType;
	RewardCardStatType statType = RewardCardStatType::HP;
	double value = 0.0;
	bool repeatable = false;
};

struct RunState
{
	bool isActive = false;
	bool useDebugFullUnlocks = false;
	int32 currentBattleIndex = 0;
	int32 totalBattles = 3;
	bool isFailed = false;
	bool isCleared = false;
	Array<String> selectedCardIds;
	Array<String> pendingRewardCardIds;
};
