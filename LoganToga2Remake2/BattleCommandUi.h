#pragma once

#include "BattleConfig.h"
#include "BattleState.h"

enum class CommandKind
{
	Production,
	Construction,
	Upgrade,
	Repair
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
	String displayLabel;
	String glyphText;
	String descriptionText;
	String flavorText;
	Optional<TurretUpgradeType> turretUpgradeType;
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

[[nodiscard]] bool HasSelectedWorker(const BattleState& state);
[[nodiscard]] Array<UnitArchetype> CollectSelectedBuildingArchetypes(const BattleState& state);
[[nodiscard]] Optional<int32> FindSingleSelectedPlayerTurretId(const BattleState& state);
[[nodiscard]] Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectTurretUpgradeCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectConstructionCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectRepairCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectCommandEntries(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] String GetCommandPanelTitle(const BattleState& state);
[[nodiscard]] String GetCommandSectionLabel(const Array<CommandIconEntry>& commands);
[[nodiscard]] Optional<CommandPanelLayout> BuildCommandPanelLayout(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Optional<CommandIconEntry> HitTestCommandIcon(const CommandPanelLayout& layout, const Vec2& cursorScreenPos);
