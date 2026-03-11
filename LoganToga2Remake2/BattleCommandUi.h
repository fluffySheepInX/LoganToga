#pragma once

#include "BattleConfig.h"

struct CommandIconEntry
{
	int32 slot = 0;
	UnitArchetype producer = UnitArchetype::Base;
	UnitArchetype archetype = UnitArchetype::Soldier;
	int32 cost = 0;
	bool isEnabled = true;
	String statusText = U"READY";
};

struct CommandIconLayout
{
	CommandIconEntry command;
	RectF rect{ 0, 0, 0, 0 };
};

struct CommandPanelLayout
{
	String title = U"BUILDING COMMANDS";
	RectF panelRect{ 0, 0, 0, 0 };
	Array<CommandIconLayout> productionIcons;
};

[[nodiscard]] inline Array<UnitArchetype> CollectSelectedBuildingArchetypes(const BattleState& state)
{
	Array<UnitArchetype> archetypes;

	for (const auto& unit : state.units)
	{
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && IsBuildingArchetype(unit.archetype)))
		{
			continue;
		}

		bool alreadyAdded = false;
		for (const auto archetype : archetypes)
		{
			if (archetype == unit.archetype)
			{
				alreadyAdded = true;
				break;
			}
		}

		if (!alreadyAdded)
		{
			archetypes << unit.archetype;
		}
	}

	return archetypes;
}

[[nodiscard]] inline Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config)
{
	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	Array<CommandIconEntry> commands;

	for (const auto& slot : config.playerProductionSlots)
	{
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
		const int32 cost = definition ? definition->cost : 0;
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

		commands << CommandIconEntry{ slot.slot, slot.producer, slot.archetype, cost, isEnabled, statusText };
	}

	return commands;
}

[[nodiscard]] inline String GetCommandPanelTitle(const BattleState& state)
{
	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	if (selectedBuildings.size() == 1)
	{
		return GetArchetypeLabel(selectedBuildings.front()) + U" COMMANDS";
	}

	return U"BUILDING COMMANDS";
}

[[nodiscard]] inline Optional<CommandPanelLayout> BuildCommandPanelLayout(const BattleState& state, const BattleConfigData& config)
{
	const Array<CommandIconEntry> productionCommands = CollectProductionCommands(state, config);
	if (productionCommands.isEmpty())
	{
		return none;
	}

	constexpr int32 Columns = 3;
	constexpr double IconSize = 88.0;
	constexpr double Gap = 10.0;
	const int32 rowCount = static_cast<int32>((productionCommands.size() + Columns - 1) / Columns);
	const double panelWidth = 16 + (Columns * IconSize) + ((Columns - 1) * Gap) + 16;
	const double panelHeight = 50 + (rowCount * IconSize) + ((rowCount - 1) * Gap) + 18;
	const RectF panelRect{
		Scene::Width() - panelWidth - 16,
		Scene::Height() - panelHeight - 16,
		panelWidth,
		panelHeight
	};

	CommandPanelLayout layout;
	layout.title = GetCommandPanelTitle(state);
	layout.panelRect = panelRect;

	const Vec2 origin = panelRect.pos.movedBy(16, 50);
	for (size_t index = 0; index < productionCommands.size(); ++index)
	{
		const int32 column = static_cast<int32>(index % Columns);
		const int32 row = static_cast<int32>(index / Columns);
		layout.productionIcons << CommandIconLayout{
			productionCommands[index],
			RectF{
				origin.x + ((IconSize + Gap) * column),
				origin.y + ((IconSize + Gap) * row),
				IconSize,
				IconSize
			}
		};
	}

	return layout;
}

[[nodiscard]] inline Optional<CommandIconEntry> HitTestCommandIcon(const CommandPanelLayout& layout, const Vec2& cursorScreenPos)
{
	for (const auto& icon : layout.productionIcons)
	{
		if (icon.rect.intersects(cursorScreenPos))
		{
			return icon.command;
		}
	}

	return none;
}
