#pragma once

#include "BattleCommandUi.h"
#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleUiText
{
   [[nodiscard]] inline String GetLocalizedArchetypeLabel(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
            return Localization::GetText(U"battle.archetype.base");
		case UnitArchetype::Barracks:
         return Localization::GetText(U"battle.archetype.barracks");
		case UnitArchetype::Stable:
         return Localization::GetText(U"battle.archetype.stable");
		case UnitArchetype::Turret:
         return Localization::GetText(U"battle.archetype.turret");
		case UnitArchetype::Worker:
            return Localization::GetText(U"battle.archetype.worker");
		case UnitArchetype::Soldier:
           return Localization::GetText(U"battle.archetype.soldier");
		case UnitArchetype::Archer:
         return Localization::GetText(U"battle.archetype.archer");
		case UnitArchetype::Sniper:
            return Localization::GetText(U"battle.archetype.sniper");
		case UnitArchetype::Katyusha:
         return Localization::GetText(U"battle.archetype.katyusha");
		case UnitArchetype::MachineGun:
           return Localization::GetText(U"battle.archetype.machine_gun");
		case UnitArchetype::Goliath:
            return Localization::GetText(U"battle.archetype.goliath");
		case UnitArchetype::Healer:
            return Localization::GetText(U"battle.archetype.healer");
		case UnitArchetype::Spinner:
         return Localization::GetText(U"battle.archetype.spinner");
		default:
           return Localization::GetText(U"battle.archetype.unit");
		}
	}

	[[nodiscard]] inline String GetLocalizedArchetypeDescription(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
            return Localization::GetText(U"battle.archetype_desc.base");
		case UnitArchetype::Barracks:
         return Localization::GetText(U"battle.archetype_desc.barracks");
		case UnitArchetype::Stable:
          return Localization::GetText(U"battle.archetype_desc.stable");
		case UnitArchetype::Turret:
            return Localization::GetText(U"battle.archetype_desc.turret");
		case UnitArchetype::Worker:
         return Localization::GetText(U"battle.archetype_desc.worker");
		case UnitArchetype::Soldier:
          return Localization::GetText(U"battle.archetype_desc.soldier");
		case UnitArchetype::Archer:
           return Localization::GetText(U"battle.archetype_desc.archer");
		case UnitArchetype::Sniper:
           return Localization::GetText(U"battle.archetype_desc.sniper");
		case UnitArchetype::Katyusha:
          return Localization::GetText(U"battle.archetype_desc.katyusha");
		case UnitArchetype::MachineGun:
           return Localization::GetText(U"battle.archetype_desc.machine_gun");
		case UnitArchetype::Goliath:
          return Localization::GetText(U"battle.archetype_desc.goliath");
		case UnitArchetype::Healer:
            return Localization::GetText(U"battle.archetype_desc.healer");
		case UnitArchetype::Spinner:
         return Localization::GetText(U"battle.archetype_desc.spinner");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedArchetypeFlavorText(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Stable:
           return Localization::GetText(U"battle.archetype_flavor.stable");
		case UnitArchetype::Turret:
         return Localization::GetText(U"battle.archetype_flavor.turret");
		case UnitArchetype::Barracks:
         return Localization::GetText(U"battle.archetype_flavor.barracks");
		case UnitArchetype::Soldier:
           return Localization::GetText(U"battle.archetype_flavor.soldier");
		case UnitArchetype::Archer:
            return Localization::GetText(U"battle.archetype_flavor.archer");
		case UnitArchetype::Sniper:
            return Localization::GetText(U"battle.archetype_flavor.sniper");
		case UnitArchetype::Katyusha:
            return Localization::GetText(U"battle.archetype_flavor.katyusha");
		case UnitArchetype::MachineGun:
            return Localization::GetText(U"battle.archetype_flavor.machine_gun");
		case UnitArchetype::Goliath:
          return Localization::GetText(U"battle.archetype_flavor.goliath");
		case UnitArchetype::Healer:
         return Localization::GetText(U"battle.archetype_flavor.healer");
		case UnitArchetype::Spinner:
          return Localization::GetText(U"battle.archetype_flavor.spinner");
		default:
			return U"";
		}
	}

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

	[[nodiscard]] inline String GetHudTitle(const BattleConfigData& config)
	{
		const String english = config.hud.title.isEmpty() ? U"LoganToga2 Remake Prototype" : config.hud.title;
        return Localization::Legacy::GetText(U"battle.hud.title", U"LoganToga2 Remake Prototype", english);
	}

	[[nodiscard]] inline String GetResourceSummary(const int32 pointCount, const int32 income)
	{
     return Localization::FormatText(U"battle.hud.resource_summary", pointCount, income);
	}

	[[nodiscard]] inline String GetRunBattleLabel(const int32 currentBattle, const int32 totalBattles)
	{
      return Localization::FormatText(U"battle.hud.run_battle", currentBattle, totalBattles);
	}

	[[nodiscard]] inline String GetRunInactiveLabel()
	{
      return Localization::GetText(U"battle.hud.run_inactive");
	}

	[[nodiscard]] inline String GetGoldLabel(const int32 gold)
	{
      return Localization::FormatText(U"battle.hud.gold", gold);
	}

	[[nodiscard]] inline String GetPlayerWinLabel()
	{
       return Localization::GetText(U"battle.hud.player_win");
	}

	[[nodiscard]] inline String GetEnemyWinLabel()
	{
      return Localization::GetText(U"battle.hud.enemy_win");
	}

	[[nodiscard]] inline String GetWinHint(const BattleConfigData& config)
	{
		const String english = config.hud.winHint.isEmpty() ? U"Enter: title / R: retry" : config.hud.winHint;
        return Localization::Legacy::GetText(U"battle.hud.win_hint", U"Enter: タイトル / R: リトライ", english);
	}

	[[nodiscard]] inline String GetRunRewardHint()
	{
       return Localization::GetText(U"battle.hud.run_reward_hint");
	}

	[[nodiscard]] inline String GetRunTitleHint()
	{
        return Localization::GetText(U"battle.hud.run_title_hint");
	}

	[[nodiscard]] inline String GetControls(const BattleConfigData& config)
	{
		const String english = config.hud.controls.isEmpty()
			? U"L drag: pan / R click: move or attack / Q-W-E: formation / 1-0: command / X: cancel"
			: config.hud.controls;
        return Localization::Legacy::GetText(U"battle.hud.controls", U"左ドラッグ: 視点移動 / 右クリック: 移動・攻撃 / Q-W-E: 陣形 / 1-0: コマンド / X: キャンセル", english);
	}

	[[nodiscard]] inline String GetEscapeHint(const BattleConfigData& config)
	{
		const String english = config.hud.escapeHint.isEmpty() ? U"Esc: pause menu" : config.hud.escapeHint;
        return Localization::Legacy::GetText(U"battle.hud.escape_hint", U"Esc: ポーズメニュー", english);
	}

	[[nodiscard]] inline String GetResultVictoryTitle()
	{
        return Localization::GetText(U"battle.result.victory_title");
	}

	[[nodiscard]] inline String GetResultRunClearedTitle()
	{
      return Localization::GetText(U"battle.result.run_cleared_title");
	}

	[[nodiscard]] inline String GetResultDefeatTitle()
	{
      return Localization::GetText(U"battle.result.defeat_title");
	}

	[[nodiscard]] inline String GetResultSubtitle(const int32 currentBattle, const int32 totalBattles, const size_t selectedCardCount)
	{
      return Localization::FormatText(U"battle.result.subtitle", currentBattle, totalBattles, selectedCardCount);
	}

	[[nodiscard]] inline String GetResultRetryAction()
	{
       return Localization::GetText(U"battle.result.retry_action");
	}

	[[nodiscard]] inline String GetResultEnterReturnTitle()
	{
        return Localization::GetText(U"battle.result.enter_return_title");
	}

	[[nodiscard]] inline String GetResultFooterRunEnded()
	{
     return Localization::GetText(U"battle.result.footer_run_ended");
	}

	[[nodiscard]] inline String GetResultEnterChooseReward()
	{
       return Localization::GetText(U"battle.result.enter_choose_reward");
	}

	[[nodiscard]] inline String GetResultFooterChooseReward()
	{
      return Localization::GetText(U"battle.result.footer_choose_reward");
	}

	[[nodiscard]] inline String GetResultEnterBonusRoom()
	{
      return Localization::GetText(U"battle.result.enter_bonus_room");
	}

	[[nodiscard]] inline String GetResultFooterBonusRoom()
	{
      return Localization::GetText(U"battle.result.footer_bonus_room");
	}

	[[nodiscard]] inline String GetResultFooterRunComplete()
	{
       return Localization::GetText(U"battle.result.footer_run_complete");
	}

	[[nodiscard]] inline String GetPauseTitle()
	{
      return Localization::GetText(U"battle.pause.title");
	}

	[[nodiscard]] inline String GetPauseResume()
	{
     return Localization::GetText(U"battle.pause.resume");
	}

	[[nodiscard]] inline String GetPauseRestartBattle()
	{
        return Localization::GetText(U"battle.pause.restart_battle");
	}

	[[nodiscard]] inline String GetPauseReturnToTitle()
	{
      return Localization::GetText(U"battle.pause.return_to_title");
	}

	[[nodiscard]] inline String GetPauseSaveReturnToTitle()
	{
      return Localization::GetText(U"battle.pause.save_return_to_title");
	}

	[[nodiscard]] inline String GetPauseProductionPrefix()
	{
      return Localization::GetText(U"battle.pause.production_prefix");
	}

	[[nodiscard]] inline String GetPauseBuildNone()
	{
        return Localization::GetText(U"battle.pause.build_none");
	}

	[[nodiscard]] inline String GetPauseBuildText(const int32 slot, const String& archetypeLabel, const int32 cost)
	{
     return Localization::FormatText(U"battle.pause.build_text", slot, archetypeLabel, cost);
	}

	[[nodiscard]] inline String GetPauseMenuHint()
	{
      return Localization::GetText(U"battle.pause.menu_hint");
	}

	[[nodiscard]] inline String GetQueueTitle()
	{
        return Localization::GetText(U"battle.queue.title");
	}

	[[nodiscard]] inline String GetQueueIdle()
	{
        return Localization::GetText(U"battle.queue.idle");
	}

	[[nodiscard]] inline String GetQueueNoQueuedUnits()
	{
        return Localization::GetText(U"battle.queue.no_queued_units");
	}

	[[nodiscard]] inline String GetQueueConstructing()
	{
        return Localization::GetText(U"battle.queue.constructing");
	}

	[[nodiscard]] inline String GetQueueSubtitle(const String& buildingLabel, const size_t queueCount)
	{
      return Localization::FormatText(U"battle.queue.subtitle", buildingLabel, queueCount);
	}

	[[nodiscard]] inline String GetQueueSelectHint(const String& buildingLabel)
	{
       return Localization::FormatText(U"battle.queue.select_hint", buildingLabel);
	}

	[[nodiscard]] inline String GetWorldPlaceLabel(const UnitArchetype archetype)
	{
       return Localization::FormatText(U"battle.world.place", GetLocalizedArchetypeLabel(archetype));
	}

	[[nodiscard]] inline String GetWorldBuildLabel(const UnitArchetype archetype)
	{
       return Localization::FormatText(U"battle.world.build", GetLocalizedArchetypeLabel(archetype));
	}

	[[nodiscard]] inline String GetWorldConstructing()
	{
        return Localization::GetText(U"battle.world.constructing");
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
