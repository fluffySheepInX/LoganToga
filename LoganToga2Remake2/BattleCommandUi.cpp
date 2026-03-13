#include "BattleCommandUi.h"

String GetCommandPanelTitle(const BattleState& state)
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

String GetCommandSectionLabel(const Array<CommandIconEntry>& commands)
{
	bool hasProduction = false;
	bool hasConstruction = false;
	bool hasUpgrade = false;

	for (const auto& command : commands)
	{
		hasProduction |= (command.kind == CommandKind::Production);
		hasConstruction |= (command.kind == CommandKind::Construction);
		hasUpgrade |= (command.kind == CommandKind::Upgrade);
	}

	if ((static_cast<int32>(hasProduction) + static_cast<int32>(hasConstruction) + static_cast<int32>(hasUpgrade)) >= 2)
	{
		return U"COMMANDS";
	}

	if (hasUpgrade)
	{
		return U"UPGRADES";
	}

	if (hasConstruction)
	{
		return U"CONSTRUCTION";
	}

	return U"PRODUCTION";
}

Optional<CommandPanelLayout> BuildCommandPanelLayout(const BattleState& state, const BattleConfigData& config)
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

Optional<CommandIconEntry> HitTestCommandIcon(const CommandPanelLayout& layout, const Vec2& cursorScreenPos)
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
