#pragma once

#include "BattleConfig.h"
#include "BattleState.h"

enum class CommandKind
{
	Production,
	Construction,
	Upgrade,
	Repair,
	Detonate
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

struct FormationButtonEntry
{
	FormationType formation = FormationType::Line;
	String label;
	bool isActive = false;
};

struct FormationButtonLayout
{
	FormationButtonEntry button;
	RectF rect{ 0, 0, 0, 0 };
};

struct FormationPanelLayout
{
	String title = U"FORMATION";
	RectF panelRect{ 0, 0, 0, 0 };
	Array<FormationButtonLayout> buttons;
};

enum class EnemyAiDebugModeSelection
{
	Toml,
	Default,
	StagingAssault
};

struct EnemyAiDebugModeButtonEntry
{
	EnemyAiDebugModeSelection selection = EnemyAiDebugModeSelection::Toml;
	String label;
	bool isActive = false;
};

struct EnemyAiDebugModeButtonLayout
{
	EnemyAiDebugModeButtonEntry button;
	RectF rect{ 0, 0, 0, 0 };
};

struct EnemyAiDebugPanelLayout
{
	String title = U"ENEMY AI DEBUG";
	RectF panelRect{ 0, 0, 0, 0 };
	Array<EnemyAiDebugModeButtonLayout> buttons;
};

[[nodiscard]] bool HasSelectedWorker(const BattleState& state);
[[nodiscard]] Array<UnitArchetype> CollectSelectedBuildingArchetypes(const BattleState& state);
[[nodiscard]] Optional<int32> FindSingleSelectedPlayerTurretId(const BattleState& state);
[[nodiscard]] Array<CommandIconEntry> CollectProductionCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectTurretUpgradeCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectConstructionCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectRepairCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectDetonateCommands(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Array<CommandIconEntry> CollectCommandEntries(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] String GetCommandPanelTitle(const BattleState& state);
[[nodiscard]] String GetCommandSectionLabel(const Array<CommandIconEntry>& commands);
[[nodiscard]] Optional<CommandPanelLayout> BuildCommandPanelLayout(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Optional<CommandIconEntry> HitTestCommandIcon(const CommandPanelLayout& layout, const Vec2& cursorScreenPos);
[[nodiscard]] FormationPanelLayout BuildFormationPanelLayout(const BattleState& state);
[[nodiscard]] Optional<FormationType> HitTestFormationButton(const FormationPanelLayout& layout, const Vec2& cursorScreenPos);
[[nodiscard]] Optional<EnemyAiDebugPanelLayout> BuildEnemyAiDebugPanelLayout(const BattleState& state, const BattleConfigData& config);
[[nodiscard]] Optional<EnemyAiDebugModeSelection> HitTestEnemyAiDebugModeButton(const EnemyAiDebugPanelLayout& layout, const Vec2& cursorScreenPos);
