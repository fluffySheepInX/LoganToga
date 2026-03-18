#include "BattleCommandUi.h"

#include "BattleUiText.h"

Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config)
{
	if (state.tutorialActive)
	{
		const bool productionPhase = (state.tutorialPhase == TutorialPhase::ProduceUnit)
			|| (state.tutorialPhase == TutorialPhase::DefendWave)
			|| (state.tutorialPhase == TutorialPhase::Completed);
		if (!productionPhase)
		{
			return {};
		}
	}

	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	Array<CommandIconEntry> commands;

	for (const auto& slot : config.playerProductionSlots)
	{
		if (state.tutorialActive && (slot.archetype != config.tutorial.requiredProduction))
		{
			continue;
		}

		if (!ContainsArchetype(config.playerAvailableProductionArchetypes, slot.archetype))
		{
			continue;
		}

		bool isAvailable = false;
		for (const auto producer : selectedBuildings)
		{
			if (producer == slot.producer)
			{
				isAvailable = true;
				break;
			}
		}

		if (!isAvailable)
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = (slot.cost > 0)
			? slot.cost
			: (definition ? definition->cost : 0);
      const String descriptionText = BattleUiText::GetLocalizedArchetypeDescription(slot.archetype);
		const String flavorText = BattleUiText::GetLocalizedArchetypeFlavorText(slot.archetype);
		bool hasProducer = false;
		for (const auto& building : state.buildings)
		{
			const auto* unit = state.findUnit(building.unitId);
			if (unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == slot.producer) && building.isConstructed)
			{
				hasProducer = true;
				break;
			}
		}

		const bool hasEnoughGold = (state.playerGold >= cost);
		const bool isEnabled = (!state.winner) && hasProducer && (cost > 0) && hasEnoughGold;
       String statusText = BattleUiText::GetCommandStatusReady();
		if (state.winner)
		{
           statusText = BattleUiText::GetCommandStatusBattleEnded();
		}
		else if (!hasProducer)
		{
           statusText = BattleUiText::GetCommandStatusProducerOffline();
		}
		else if (cost <= 0)
		{
            statusText = BattleUiText::GetCommandStatusUnavailable();
		}
		else if (!hasEnoughGold)
		{
            statusText = BattleUiText::GetCommandStatusNotEnoughGold();
		}

		commands << CommandIconEntry{
			CommandKind::Production,
			slot.slot,
			slot.producer,
			slot.archetype,
			cost,
			isEnabled,
			statusText,
			U"",
			U"",
			descriptionText,
			flavorText,
			none
		};
	}

	return commands;
}
