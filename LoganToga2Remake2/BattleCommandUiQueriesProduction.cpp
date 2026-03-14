#include "BattleCommandUi.h"

Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config)
{
	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	Array<CommandIconEntry> commands;

	for (const auto& slot : config.playerProductionSlots)
	{
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
		const String descriptionText = definition ? definition->description : U"";
		const String flavorText = definition ? definition->flavorText : U"";
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
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (!hasProducer)
		{
			statusText = U"PRODUCER OFFLINE";
		}
		else if (cost <= 0)
		{
			statusText = U"UNAVAILABLE";
		}
		else if (!hasEnoughGold)
		{
			statusText = U"NOT ENOUGH GOLD";
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
