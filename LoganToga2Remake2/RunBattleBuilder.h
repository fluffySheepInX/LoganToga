#pragma once

#include "BattleConfigMapLoader.h"
#include "RunCardLogic.h"

inline void ApplyBattleProgressionMap(BattleConfigData& config, const EnemyProgressionConfig& progression)
{
	if (progression.mapSourcePath.isEmpty())
	{
		return;
	}

	const TOMLReader mapToml{ progression.mapSourcePath };
	if (!mapToml)
	{
		throw Error{ U"Failed to load battle map override: " + progression.mapSourcePath };
	}

	LoadBattleMapConfig(config, mapToml);
}

[[nodiscard]] inline const EnemyProgressionConfig* FindBattleLayoutProgressionConfig(const BattleConfigData& baseConfig, const RunState& runState, const int32 battleNumber)
{
	if (battleNumber <= 1)
	{
		return FindEnemyProgressionConfig(baseConfig, battleNumber);
	}

	const int32 layoutIndex = battleNumber - 2;
	if ((0 <= layoutIndex) && (layoutIndex < static_cast<int32>(runState.mapProgressionBattles.size())))
	{
		if (const auto* progression = FindEnemyProgressionConfig(baseConfig, runState.mapProgressionBattles[layoutIndex]))
		{
			return progression;
		}
	}

	return FindEnemyProgressionConfig(baseConfig, battleNumber);
}

inline void AppendEnemyProgressionUnits(BattleConfigData& config, const EnemyProgressionConfig& progression)
{
	int32 unitIndex = 0;
	const auto appendUnits = [&](const UnitArchetype archetype, const int32 count)
	{
		for (int32 index = 0; index < count; ++index, ++unitIndex)
		{
			const double xOffset = 90.0 + (unitIndex * 24.0);
			const double yOffset = (unitIndex % 2 == 0) ? 50.0 : -50.0;
			config.initialUnits << InitialUnitPlacement{
				.owner = Owner::Enemy,
				.archetype = archetype,
				.position = Vec2{
					config.enemySpawn.position.x - xOffset,
					config.enemySpawn.position.y + yOffset
				}
			};
		}
	};

	appendUnits(config.enemySpawn.basicArchetype, progression.extraBasicUnits);
	appendUnits(config.enemySpawn.advancedArchetype, progression.extraAdvancedUnits);
}

inline void ReplaceEnemyInitialUnits(BattleConfigData& config, const EnemyProgressionConfig& progression)
{
	config.initialUnits.remove_if([](const InitialUnitPlacement& placement)
	{
		return placement.owner == Owner::Enemy;
	});

	for (const auto& placement : progression.enemyInitialUnits)
	{
		config.initialUnits << placement;
	}
}

inline void ApplyUnitStatBonus(PlayerUnitModifier& modifier, const RewardCardDefinition& card)
{
	switch (card.statType)
	{
	case RewardCardStatType::HP:
		modifier.hpDelta += static_cast<int32>(card.value);
		break;
	case RewardCardStatType::AttackPower:
		modifier.attackPowerDelta += static_cast<int32>(card.value);
		break;
	case RewardCardStatType::MoveSpeed:
		modifier.moveSpeedDelta += card.value;
		break;
	case RewardCardStatType::AttackRange:
		modifier.attackRangeDelta += card.value;
		break;
	case RewardCardStatType::ProductionTime:
		modifier.productionTimeDelta += card.value;
		break;
	default:
		break;
	}
}

[[nodiscard]] inline BattleConfigData BuildBattleConfigForRun(const BattleConfigData& baseConfig, const RunState& runState, const Array<RewardCardDefinition>& cards)
{
	BattleConfigData config = baseConfig;
	ResolvePlayerUnlocks(runState, cards, config.playerAvailableProductionArchetypes, config.playerAvailableConstructionArchetypes);
	ResolvePlayerTurretUpgradeUnlocks(runState, cards, config.playerAvailableTurretUpgrades);

	for (const auto& selectedId : runState.selectedCardIds)
	{
		const auto* card = FindRewardCardDefinition(cards, selectedId);
		if (!(card && (card->effectType == RewardCardEffectType::UnitStatBonus)))
		{
			continue;
		}

		auto* modifier = FindPlayerUnitModifier(config, card->targetArchetype);
		if (!modifier)
		{
			config.playerUnitModifiers << PlayerUnitModifier{ .archetype = card->targetArchetype };
			modifier = &config.playerUnitModifiers.back();
		}

		ApplyUnitStatBonus(*modifier, *card);
	}

	const int32 battleNumber = runState.currentBattleIndex + 1;
	config.hud.title = s3d::Format(baseConfig.hud.title, U"  RUN ", battleNumber, U"/", runState.totalBattles);
	const auto* layoutProgression = FindBattleLayoutProgressionConfig(baseConfig, runState, battleNumber);
	if (layoutProgression)
	{
		ApplyBattleProgressionMap(config, *layoutProgression);
		if (layoutProgression->replaceEnemyInitialUnits && !layoutProgression->enemyInitialUnits.isEmpty())
		{
			ReplaceEnemyInitialUnits(config, *layoutProgression);
		}

		AppendEnemyProgressionUnits(config, *layoutProgression);
	}

	if (const auto* progression = FindEnemyProgressionConfig(baseConfig, battleNumber))
	{
		config.enemyGold += progression->goldBonus;
		config.income.enemyAmount += progression->incomeBonus;
		if (progression->spawnInterval > 0.0)
		{
			config.enemySpawn.interval = progression->spawnInterval;
		}
		if (progression->assaultUnitThreshold > 0)
		{
			config.enemyAI.assaultUnitThreshold = progression->assaultUnitThreshold;
		}
		if (progression->overrideEnemyAiMode)
		{
			config.enemyAI.mode = progression->enemyAiMode;
		}
		if (progression->stagingAssaultMinUnits > 0)
		{
			config.enemyAI.stagingAssaultMinUnits = progression->stagingAssaultMinUnits;
		}
		if (progression->stagingAssaultGatherRadius > 0.0)
		{
			config.enemyAI.stagingAssaultGatherRadius = progression->stagingAssaultGatherRadius;
		}
		if (progression->stagingAssaultMaxWait > 0.0)
		{
			config.enemyAI.stagingAssaultMaxWait = progression->stagingAssaultMaxWait;
		}
		if (progression->stagingAssaultCommitTime > 0.0)
		{
			config.enemyAI.stagingAssaultCommitTime = progression->stagingAssaultCommitTime;
		}
	}

	return config;
}
