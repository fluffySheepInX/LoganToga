#pragma once

#include "BattleCommandUi.h"
#include "Localization.h"

namespace BattleUiText
{
	[[nodiscard]] inline String GetLocalizedTurretUpgradeLabel(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.label.power");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.label.rapid");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.label.dual");
		default:
			return Localization::GetText(U"battle.upgrade.label.default");
		}
	}

	[[nodiscard]] inline String GetLocalizedTurretUpgradeDescription(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.desc.power");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.desc.rapid");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.desc.dual");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedTurretUpgradeFlavorText(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.flavor.power");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.flavor.rapid");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.flavor.dual");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetCommandStatusReady()
	{
		return Localization::GetText(U"battle.command.status.ready");
	}

	[[nodiscard]] inline String GetCommandStatusBattleEnded()
	{
		return Localization::GetText(U"battle.command.status.battle_ended");
	}

	[[nodiscard]] inline String GetCommandStatusUnavailable()
	{
		return Localization::GetText(U"battle.command.status.unavailable");
	}

	[[nodiscard]] inline String GetCommandStatusNotEnoughGold()
	{
		return Localization::GetText(U"battle.command.status.not_enough_gold");
	}

	[[nodiscard]] inline String GetCommandStatusProducerOffline()
	{
		return Localization::GetText(U"battle.command.status.producer_offline");
	}

	[[nodiscard]] inline String GetCommandStatusLocked()
	{
		return Localization::GetText(U"battle.command.status.locked");
	}

	[[nodiscard]] inline String GetCommandStatusUpgraded()
	{
		return Localization::GetText(U"battle.command.status.upgraded");
	}

	[[nodiscard]] inline String GetCommandStatusNoTarget()
	{
		return Localization::GetText(U"battle.command.status.no_target");
	}

	[[nodiscard]] inline String GetCommandStatusFullHp()
	{
		return Localization::GetText(U"battle.command.status.full_hp");
	}

	[[nodiscard]] inline String GetCommandStatusArmed()
	{
		return Localization::GetText(U"battle.command.status.armed");
	}

	[[nodiscard]] inline String GetRepairCommandLabel()
	{
		return Localization::GetText(U"battle.command.label.repair");
	}

	[[nodiscard]] inline String GetRepairCommandDescription()
	{
		return Localization::GetText(U"battle.command.desc.repair");
	}

	[[nodiscard]] inline String GetRepairCommandFlavorText()
	{
		return Localization::GetText(U"battle.command.flavor.repair");
	}

	[[nodiscard]] inline String GetDetonateCommandLabel()
	{
		return Localization::GetText(U"battle.command.label.detonate");
	}

	[[nodiscard]] inline String GetDetonateCommandDescription()
	{
		return Localization::GetText(U"battle.command.desc.detonate");
	}

	[[nodiscard]] inline String GetDetonateCommandFlavorText()
	{
		return Localization::GetText(U"battle.command.flavor.detonate");
	}

	[[nodiscard]] inline String GetCommandPanelWorkerTitle()
	{
		return Localization::GetText(U"battle.command.panel.worker");
	}

	[[nodiscard]] inline String GetCommandPanelArchetypeTitle(const String& archetypeLabel)
	{
		return Localization::FormatText(U"battle.command.panel.archetype", archetypeLabel);
	}

	[[nodiscard]] inline String GetCommandPanelSelectionTitle()
	{
		return Localization::GetText(U"battle.command.panel.selection");
	}

	[[nodiscard]] inline String GetCommandSectionCommands()
	{
		return Localization::GetText(U"battle.command.section.commands");
	}

	[[nodiscard]] inline String GetCommandSectionUpgrades()
	{
		return Localization::GetText(U"battle.command.section.upgrades");
	}

	[[nodiscard]] inline String GetCommandSectionRepair()
	{
		return Localization::GetText(U"battle.command.section.repair");
	}

	[[nodiscard]] inline String GetCommandSectionSpecial()
	{
		return Localization::GetText(U"battle.command.section.special");
	}

	[[nodiscard]] inline String GetCommandSectionConstruction()
	{
		return Localization::GetText(U"battle.command.section.construction");
	}

	[[nodiscard]] inline String GetCommandSectionProduction()
	{
		return Localization::GetText(U"battle.command.section.production");
	}

	[[nodiscard]] inline String GetFormationPanelTitle()
	{
		return Localization::GetText(U"battle.formation.panel");
	}

	[[nodiscard]] inline String GetEnemyAiDebugTitle()
	{
		return Localization::GetText(U"battle.enemy_ai_debug.title");
	}

	[[nodiscard]] inline String GetEnemyAiModeTomlLabel()
	{
		return Localization::GetText(U"battle.enemy_ai_debug.toml");
	}

	[[nodiscard]] inline String GetEnemyAiModeLabel(const EnemyAiMode mode)
	{
		switch (mode)
		{
		case EnemyAiMode::StagingAssault:
			return Localization::GetText(U"battle.enemy_ai_debug.staging");
		case EnemyAiMode::Default:
		default:
			return Localization::GetText(U"battle.enemy_ai_debug.default");
		}
	}

	[[nodiscard]] inline String GetCommandKindLabel(const CommandKind kind)
	{
		switch (kind)
		{
		case CommandKind::Construction:
			return Localization::GetText(U"battle.command.kind.build");
		case CommandKind::Repair:
			return Localization::GetText(U"battle.command.kind.repair");
		case CommandKind::Detonate:
			return Localization::GetText(U"battle.command.kind.detonate");
		case CommandKind::Upgrade:
			return Localization::GetText(U"battle.command.kind.upgrade");
		case CommandKind::Production:
		default:
			return Localization::GetText(U"battle.command.kind.queue");
		}
	}

	[[nodiscard]] inline String GetCommandCost(const int32 cost)
	{
		return Localization::FormatText(U"battle.command.cost", cost);
	}
}
