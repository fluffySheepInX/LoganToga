#pragma once

#include "BattleConfig.h"
#include "BattleState.h"

enum class CommandKind
{
	Production,
	Construction
};

struct CommandIconEntry
{
	CommandKind kind = CommandKind::Production;
	int32 slot = 0;
	UnitArchetype sourceArchetype = UnitArchetype::Base;
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
	String title = U"COMMANDS";
	String sectionLabel = U"COMMANDS";
	RectF panelRect{ 0, 0, 0, 0 };
	Array<CommandIconLayout> commandIcons;
};

[[nodiscard]] inline bool HasSelectedWorker(const BattleState& state)
{
	for (const auto& unit : state.units)
	{
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
		{
			return true;
		}
	}

	return false;
}

[[nodiscard]] inline Array<UnitArchetype> CollectSelectedBuildingArchetypes(const BattleState& state)
{
	Array<UnitArchetype> archetypes;

	for (const auto& unit : state.units)
	{
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && IsBuildingArchetype(unit.archetype)))
		{
			continue;
		}

		AppendUniqueArchetype(archetypes, unit.archetype);
	}

	return archetypes;
}

[[nodiscard]] inline Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config)
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

		commands << CommandIconEntry{ CommandKind::Production, slot.slot, slot.producer, slot.archetype, cost, isEnabled, statusText };
	}

	return commands;
}

[[nodiscard]] inline Array<CommandIconEntry> CollectConstructionCommands(const BattleState& state, const BattleConfigData& config)
{
	if (!HasSelectedWorker(state))
	{
		return {};
	}

	Array<CommandIconEntry> commands;
	for (const auto& slot : config.playerConstructionSlots)
	{
		if (!ContainsArchetype(config.playerAvailableConstructionArchetypes, slot.archetype))
		{
			continue;
		}

		const auto* definition = FindUnitDefinition(config, slot.archetype);
		const int32 cost = definition ? definition->cost : 0;
		const bool hasEnoughGold = (state.playerGold >= cost);
		const bool isEnabled = (!state.winner) && (cost > 0) && hasEnoughGold;
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (cost <= 0)
		{
			statusText = U"UNAVAILABLE";
		}
		else if (!hasEnoughGold)
		{
			statusText = U"NOT ENOUGH GOLD";
		}

		commands << CommandIconEntry{ CommandKind::Construction, slot.slot, UnitArchetype::Worker, slot.archetype, cost, isEnabled, statusText };
	}

	return commands;
}

[[nodiscard]] inline Array<CommandIconEntry> CollectCommandEntries(const BattleState& state, const BattleConfigData& config)
{
	Array<CommandIconEntry> commands = CollectProductionCommands(state, config);
	for (const auto& command : CollectConstructionCommands(state, config))
	{
		commands << command;
	}

	return commands;
}

[[nodiscard]] inline String GetCommandPanelTitle(const BattleState& state)
{
	const Array<UnitArchetype> selectedBuildings = CollectSelectedBuildingArchetypes(state);
	const bool hasSelectedWorker = HasSelectedWorker(state);
	if (hasSelectedWorker && selectedBuildings.isEmpty())
	{
		return U"WORKER COMMANDS";
	}

	if (!hasSelectedWorker && (selectedBuildings.size() == 1))
	{
		return GetArchetypeLabel(selectedBuildings.front()) + U" COMMANDS";
	}

	return U"SELECTION COMMANDS";
}

[[nodiscard]] inline String GetCommandSectionLabel(const Array<CommandIconEntry>& commands)
{
	bool hasProduction = false;
	bool hasConstruction = false;

	for (const auto& command : commands)
	{
		hasProduction |= (command.kind == CommandKind::Production);
		hasConstruction |= (command.kind == CommandKind::Construction);
	}

	if (hasProduction && hasConstruction)
	{
		return U"COMMANDS";
	}

	if (hasConstruction)
	{
		return U"CONSTRUCTION";
	}

	return U"PRODUCTION";
}

[[nodiscard]] inline Optional<CommandPanelLayout> BuildCommandPanelLayout(const BattleState& state, const BattleConfigData& config)
{
	const Array<CommandIconEntry> commands = CollectCommandEntries(state, config);
	if (commands.isEmpty())
	{
		return none;
	}

	constexpr int32 Columns = 3;
	constexpr double IconSize = 88.0;
	constexpr double Gap = 10.0;
	const int32 rowCount = static_cast<int32>((commands.size() + Columns - 1) / Columns);
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
	layout.sectionLabel = GetCommandSectionLabel(commands);
	layout.panelRect = panelRect;

	const Vec2 origin = panelRect.pos.movedBy(16, 50);
	for (size_t index = 0; index < commands.size(); ++index)
	{
		const int32 column = static_cast<int32>(index % Columns);
		const int32 row = static_cast<int32>(index / Columns);
		layout.commandIcons << CommandIconLayout{
			commands[index],
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
	for (const auto& icon : layout.commandIcons)
	{
		if (icon.rect.intersects(cursorScreenPos))
		{
			return icon.command;
		}
	}

	return none;
}
