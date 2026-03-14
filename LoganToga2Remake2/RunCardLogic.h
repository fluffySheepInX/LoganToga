#pragma once

#include "RunTypes.h"

[[nodiscard]] inline const RewardCardDefinition* FindRewardCardDefinition(const Array<RewardCardDefinition>& cards, const String& id)
{
	for (const auto& card : cards)
	{
		if (card.id == id)
		{
			return &card;
		}
	}

	return nullptr;
}

[[nodiscard]] inline bool HasSelectedRewardCard(const RunState& runState, const String& id)
{
	for (const auto& selectedId : runState.selectedCardIds)
	{
		if (selectedId == id)
		{
			return true;
		}
	}

	return false;
}

inline void ApplyPlayerUnlockCard(const RewardCardDefinition& card, Array<UnitArchetype>& productionArchetypes, Array<UnitArchetype>& constructionArchetypes)
{
	if (card.effectType == RewardCardEffectType::ProductionUnlock)
	{
		AppendUniqueArchetype(productionArchetypes, card.targetArchetype);
		if (card.targetArchetype == UnitArchetype::Spinner)
		{
			AppendUniqueArchetype(constructionArchetypes, UnitArchetype::Stable);
		}
	}
	else if (card.effectType == RewardCardEffectType::ConstructionUnlock)
	{
		AppendUniqueArchetype(constructionArchetypes, card.targetArchetype);
	}
}

[[nodiscard]] inline Array<int32> BuildSequentialRunMapProgressionBattles(const BattleConfigData& baseConfig, const int32 totalBattles)
{
	Array<int32> battles;
	for (int32 battleNumber = 2; battleNumber <= totalBattles; ++battleNumber)
	{
		const auto* progression = FindEnemyProgressionConfig(baseConfig, battleNumber);
		if (!(progression && !progression->mapSourcePath.isEmpty()))
		{
			continue;
		}

		battles << progression->battle;
	}

	return battles;
}

[[nodiscard]] inline Array<int32> BuildRandomizedRunMapProgressionBattles(const BattleConfigData& baseConfig, const int32 totalBattles)
{
	Array<int32> availableBattles = BuildSequentialRunMapProgressionBattles(baseConfig, totalBattles);
	Array<int32> randomizedBattles;
	while (!availableBattles.isEmpty())
	{
		const int32 index = Random(static_cast<int32>(availableBattles.size()) - 1);
		randomizedBattles << availableBattles[index];
		availableBattles.remove_at(index);
	}

	return randomizedBattles;
}

inline void BeginNewRun(RunState& runState, const BattleConfigData& baseConfig, const bool useDebugFullUnlocks)
{
	runState.isActive = true;
	runState.useDebugFullUnlocks = useDebugFullUnlocks;
	runState.currentBattleIndex = 0;
	runState.totalBattles = Random(3, 5);
	runState.isFailed = false;
	runState.isCleared = false;
	runState.mapProgressionBattles = BuildRandomizedRunMapProgressionBattles(baseConfig, runState.totalBattles);
	runState.selectedCardIds.clear();
	runState.pendingRewardCardIds.clear();
}

inline void BeginNewRun(RunState& runState, const BattleConfigData& baseConfig)
{
	BeginNewRun(runState, baseConfig, runState.useDebugFullUnlocks);
}

inline void ResolvePlayerTurretUpgradeUnlocks(const RunState& runState, const Array<RewardCardDefinition>& cards, Array<TurretUpgradeType>& upgradeTypes)
{
	if (runState.useDebugFullUnlocks)
	{
		for (const auto& card : cards)
		{
			if ((card.effectType == RewardCardEffectType::TurretUpgradeUnlock) && card.targetTurretUpgradeType)
			{
				AppendUniqueTurretUpgradeType(upgradeTypes, *card.targetTurretUpgradeType);
			}
		}
	}

	for (const auto& selectedId : runState.selectedCardIds)
	{
		const auto* card = FindRewardCardDefinition(cards, selectedId);
		if ((card && (card->effectType == RewardCardEffectType::TurretUpgradeUnlock)) && card->targetTurretUpgradeType)
		{
			AppendUniqueTurretUpgradeType(upgradeTypes, *card->targetTurretUpgradeType);
		}
	}
}

inline void ResolvePlayerUnlocks(const RunState& runState, const Array<RewardCardDefinition>& cards, Array<UnitArchetype>& productionArchetypes, Array<UnitArchetype>& constructionArchetypes)
{
	productionArchetypes = { UnitArchetype::Worker, UnitArchetype::Soldier, UnitArchetype::Sniper };
	productionArchetypes << UnitArchetype::MachineGun;
	constructionArchetypes = { UnitArchetype::Barracks };

	if (runState.useDebugFullUnlocks)
	{
		for (const auto& card : cards)
		{
			ApplyPlayerUnlockCard(card, productionArchetypes, constructionArchetypes);
		}
	}

	for (const auto& selectedId : runState.selectedCardIds)
	{
		const auto* card = FindRewardCardDefinition(cards, selectedId);
		if (!card)
		{
			continue;
		}

		ApplyPlayerUnlockCard(*card, productionArchetypes, constructionArchetypes);
	}
}

[[nodiscard]] inline bool IsRewardCardEligible(const RewardCardDefinition& card, const RunState& runState, const Array<RewardCardDefinition>& allCards)
{
	if (!card.repeatable && HasSelectedRewardCard(runState, card.id))
	{
		return false;
	}

	Array<UnitArchetype> productionArchetypes;
	Array<UnitArchetype> constructionArchetypes;
	ResolvePlayerUnlocks(runState, allCards, productionArchetypes, constructionArchetypes);
	if ((card.effectType == RewardCardEffectType::ProductionUnlock) && ContainsArchetype(productionArchetypes, card.targetArchetype))
	{
		return false;
	}
	if ((card.effectType == RewardCardEffectType::ConstructionUnlock) && ContainsArchetype(constructionArchetypes, card.targetArchetype))
	{
		return false;
	}
	if (card.effectType == RewardCardEffectType::TurretUpgradeUnlock)
	{
		Array<TurretUpgradeType> turretUpgradeTypes;
		ResolvePlayerTurretUpgradeUnlocks(runState, allCards, turretUpgradeTypes);
		if (card.targetTurretUpgradeType && ContainsTurretUpgradeType(turretUpgradeTypes, *card.targetTurretUpgradeType))
		{
			return false;
		}
	}

	return true;
}

[[nodiscard]] inline RewardCardRarity PickRewardCardRarity()
{
	const int32 roll = Random(1, 100);
	if (roll <= 60)
	{
		return RewardCardRarity::Common;
	}
	if (roll <= 90)
	{
		return RewardCardRarity::Rare;
	}

	return RewardCardRarity::Epic;
}

[[nodiscard]] inline Array<String> BuildRewardCardChoices(const RunState& runState, const Array<RewardCardDefinition>& cards)
{
	Array<const RewardCardDefinition*> eligibleCards;
	for (const auto& card : cards)
	{
		if (IsRewardCardEligible(card, runState, cards))
		{
			eligibleCards << &card;
		}
	}

	Array<String> choices;
	for (int32 slotIndex = 0; (slotIndex < 3) && (choices.size() < eligibleCards.size()); ++slotIndex)
	{
		const RewardCardRarity desiredRarity = PickRewardCardRarity();
		Array<int32> rarityMatches;
		Array<int32> fallbackMatches;
		for (int32 cardIndex = 0; cardIndex < static_cast<int32>(eligibleCards.size()); ++cardIndex)
		{
			const auto* card = eligibleCards[cardIndex];
			bool alreadyChosen = false;
			for (const auto& choice : choices)
			{
				if (choice == card->id)
				{
					alreadyChosen = true;
					break;
				}
			}
			if (alreadyChosen)
			{
				continue;
			}

			fallbackMatches << cardIndex;
			if (card->rarity == desiredRarity)
			{
				rarityMatches << cardIndex;
			}
		}

		const Array<int32>& matchPool = rarityMatches.isEmpty() ? fallbackMatches : rarityMatches;
		if (matchPool.isEmpty())
		{
			break;
		}

		const int32 chosenIndex = matchPool[Random(static_cast<int32>(matchPool.size() - 1))];
		choices << eligibleCards[chosenIndex]->id;
	}

	return choices;
}

inline void ApplyRewardCardChoice(RunState& runState, const Array<RewardCardDefinition>& cards, const String& cardId)
{
	const auto* card = FindRewardCardDefinition(cards, cardId);
	if (!(card && (card->repeatable || !HasSelectedRewardCard(runState, cardId))))
	{
		runState.pendingRewardCardIds.clear();
		return;
	}

	runState.selectedCardIds << cardId;
	runState.pendingRewardCardIds.clear();
}
