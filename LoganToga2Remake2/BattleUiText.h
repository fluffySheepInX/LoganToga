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
			return Localization::GetText(U"battle.archetype.base", U"本拠地", U"BASE");
		case UnitArchetype::Barracks:
			return Localization::GetText(U"battle.archetype.barracks", U"兵舎", U"BARRACKS");
		case UnitArchetype::Stable:
			return Localization::GetText(U"battle.archetype.stable", U"厩舎", U"STABLE");
		case UnitArchetype::Turret:
			return Localization::GetText(U"battle.archetype.turret", U"砲台", U"TURRET");
		case UnitArchetype::Worker:
			return Localization::GetText(U"battle.archetype.worker", U"作業員", U"WORKER");
		case UnitArchetype::Soldier:
			return Localization::GetText(U"battle.archetype.soldier", U"兵士", U"SOLDIER");
		case UnitArchetype::Archer:
			return Localization::GetText(U"battle.archetype.archer", U"弓兵", U"ARCHER");
		case UnitArchetype::Sniper:
			return Localization::GetText(U"battle.archetype.sniper", U"狙撃兵", U"SNIPER");
		case UnitArchetype::Katyusha:
			return Localization::GetText(U"battle.archetype.katyusha", U"カチューシャ", U"KATYUSHA");
		case UnitArchetype::MachineGun:
			return Localization::GetText(U"battle.archetype.machine_gun", U"機関銃兵", U"M-GUN");
		case UnitArchetype::Goliath:
			return Localization::GetText(U"battle.archetype.goliath", U"ゴライアス", U"GOLIATH");
		case UnitArchetype::Healer:
			return Localization::GetText(U"battle.archetype.healer", U"回復兵", U"HEALER");
		case UnitArchetype::Spinner:
			return Localization::GetText(U"battle.archetype.spinner", U"スピナー", U"SPINNER");
		default:
			return Localization::GetText(U"battle.archetype.unit", U"ユニット", U"UNIT");
		}
	}

	[[nodiscard]] inline String GetLocalizedArchetypeDescription(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Base:
			return Localization::GetText(U"battle.archetype_desc.base", U"本拠地。ここを失うと戦線が崩れる。", U"Main base. Lose this and the front collapses.");
		case UnitArchetype::Barracks:
			return Localization::GetText(U"battle.archetype_desc.barracks", U"兵士を呼べる生産拠点。戦線の継続に必要。", U"Production building for infantry. Essential for sustaining the front.");
		case UnitArchetype::Stable:
			return Localization::GetText(U"battle.archetype_desc.stable", U"回転騎兵を送り出す専用厩舎。遊びの起点になる。", U"Stable dedicated to Spinner cavalry. Opens up aggressive plays.");
		case UnitArchetype::Turret:
			return Localization::GetText(U"battle.archetype_desc.turret", U"足を止めて迎撃する防衛施設。前線の支え向き。", U"Static defense that intercepts enemies. Good for holding the line.");
		case UnitArchetype::Worker:
			return Localization::GetText(U"battle.archetype_desc.worker", U"建設と序盤の穴埋めを担う補助役。", U"Support unit for construction and early frontline patching.");
		case UnitArchetype::Soldier:
			return Localization::GetText(U"battle.archetype_desc.soldier", U"低コストで前線維持に向く基本兵。", U"Basic low-cost infantry suited for holding the front.");
		case UnitArchetype::Archer:
			return Localization::GetText(U"battle.archetype_desc.archer", U"後衛から削る射撃兵。守りの後ろで活きる。", U"Ranged unit that chips from behind. Works well behind a defense line.");
		case UnitArchetype::Sniper:
			return Localization::GetText(U"battle.archetype_desc.sniper", U"超長射程で危険な後衛を抜く狙撃兵。数は要らないが、置くだけで相手の厚みを削れる。", U"Ultra-long-range sniper that picks off dangerous backliners. You do not need many, but one changes the fight.");
		case UnitArchetype::Katyusha:
			return Localization::GetText(U"battle.archetype_desc.katyusha", U"自動でロケットを撃ち込み、密集した敵をまとめて削る後衛支援車両。単体処理は苦手。", U"Backline support launcher that fires rockets automatically and punishes clusters. Weak at single-target cleanup.");
		case UnitArchetype::MachineGun:
			return Localization::GetText(U"battle.archetype_desc.machine_gun", U"長射程を高速連射で維持する重射手。高価だが放っておいても前線を支える。", U"Heavy gunner with long range and rapid fire. Expensive, but strong even with low babysitting.");
		case UnitArchetype::Goliath:
			return Localization::GetText(U"battle.archetype_desc.goliath", U"敵に近づいて爆ぜる使い切り特攻兵。撃ち落とされても周囲を巻き込む。", U"Disposable suicide unit that explodes on approach. Even if shot down, it still threatens nearby enemies.");
		case UnitArchetype::Healer:
			return Localization::GetText(U"battle.archetype_desc.healer", U"味方歩兵を自動回復する後方支援兵。建物は治せない。", U"Backline support that automatically heals allied infantry. Cannot repair buildings.");
		case UnitArchetype::Spinner:
			return Localization::GetText(U"battle.archetype_desc.spinner", U"走りながら周囲を削る回転騎兵。深く考えず突っ込ませやすい。", U"Fast spinning cavalry that damages nearby enemies while moving. Easy to throw into the fight.");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedArchetypeFlavorText(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Stable:
			return Localization::GetText(U"battle.archetype_flavor.stable", U"勢いで押し込む兵は、ここから飛び出す。", U"Units that break through on momentum dash out from here.");
		case UnitArchetype::Turret:
			return Localization::GetText(U"battle.archetype_flavor.turret", U"据えれば静かに仕事をする。", U"Set it down and it quietly does its job.");
		case UnitArchetype::Barracks:
			return Localization::GetText(U"battle.archetype_flavor.barracks", U"粗末でも、ここが兵の集まる場所になる。", U"Rough or not, this is where soldiers gather.");
		case UnitArchetype::Soldier:
			return Localization::GetText(U"battle.archetype_flavor.soldier", U"派手さはないが、命令には素直だ。", U"Nothing flashy, but reliable under orders.");
		case UnitArchetype::Archer:
			return Localization::GetText(U"battle.archetype_flavor.archer", U"一歩引いた位置から、確実に矢を重ねる。", U"From one step back, they keep layering arrows where it matters.");
		case UnitArchetype::Sniper:
			return Localization::GetText(U"battle.archetype_flavor.sniper", U"一発ずつ、危ない相手から黙らせる。", U"One shot at a time, the dangerous targets go quiet first.");
		case UnitArchetype::Katyusha:
			return Localization::GetText(U"battle.archetype_flavor.katyusha", U"散らばれば軽い。固まれば危ない。", U"Spread out and it is manageable. Clump up and it becomes lethal.");
		case UnitArchetype::MachineGun:
			return Localization::GetText(U"battle.archetype_flavor.machine_gun", U"弾は軽いが、撃ち続ければ相手は前に出づらい。", U"Each bullet is light, but sustained fire makes advancing miserable.");
		case UnitArchetype::Goliath:
			return Localization::GetText(U"battle.archetype_flavor.goliath", U"止めても危ない。通しても危ない。", U"Dangerous if stopped. Dangerous if it gets through.");
		case UnitArchetype::Healer:
			return Localization::GetText(U"battle.archetype_flavor.healer", U"少し後ろに混ぜるだけで、前線が長く持つ。", U"Mix one slightly behind the line and the front lasts much longer.");
		case UnitArchetype::Spinner:
			return Localization::GetText(U"battle.archetype_flavor.spinner", U"止まると並、走れば楽しい。", U"Average when stopped, fun when moving.");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedTurretUpgradeLabel(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.label.power", U"威力", U"POWER");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.label.rapid", U"速射", U"RAPID");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.label.dual", U"両立", U"DUAL");
		default:
			return Localization::GetText(U"battle.upgrade.label.default", U"強化", U"UPGRADE");
		}
	}

	[[nodiscard]] inline String GetLocalizedTurretUpgradeDescription(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.desc.power", U"威力のみ強化。重い一撃で前線を支える。", U"Boosts damage only. Supports the front with heavier hits.");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.desc.rapid", U"連射のみ強化。継続迎撃に向く。", U"Boosts fire rate only. Best for sustained interception.");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.desc.dual", U"威力と連射を両立した上位強化。", U"Advanced upgrade that improves both damage and fire rate.");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetLocalizedTurretUpgradeFlavorText(const TurretUpgradeType type)
	{
		switch (type)
		{
		case TurretUpgradeType::Power:
			return Localization::GetText(U"battle.upgrade.flavor.power", U"少ない手数でも、撃てば違いが出る。", U"Even with fewer shots, each hit leaves a mark.");
		case TurretUpgradeType::Rapid:
			return Localization::GetText(U"battle.upgrade.flavor.rapid", U"細かく刻めば、押し返す時間が延びる。", U"Chip away faster and the line holds longer.");
		case TurretUpgradeType::Dual:
			return Localization::GetText(U"battle.upgrade.flavor.dual", U"大きな投資に見合うだけの制圧力を得る。", U"A large investment, but one that buys real control of the field.");
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetCommandStatusReady()
	{
		return Localization::GetText(U"battle.command.status.ready", U"準備完了", U"READY");
	}

	[[nodiscard]] inline String GetCommandStatusBattleEnded()
	{
		return Localization::GetText(U"battle.command.status.battle_ended", U"戦闘終了", U"BATTLE ENDED");
	}

	[[nodiscard]] inline String GetCommandStatusUnavailable()
	{
		return Localization::GetText(U"battle.command.status.unavailable", U"利用不可", U"UNAVAILABLE");
	}

	[[nodiscard]] inline String GetCommandStatusNotEnoughGold()
	{
		return Localization::GetText(U"battle.command.status.not_enough_gold", U"ゴールド不足", U"NOT ENOUGH GOLD");
	}

	[[nodiscard]] inline String GetCommandStatusProducerOffline()
	{
		return Localization::GetText(U"battle.command.status.producer_offline", U"生産元が停止中", U"PRODUCER OFFLINE");
	}

	[[nodiscard]] inline String GetCommandStatusLocked()
	{
		return Localization::GetText(U"battle.command.status.locked", U"未解放", U"LOCKED");
	}

	[[nodiscard]] inline String GetCommandStatusUpgraded()
	{
		return Localization::GetText(U"battle.command.status.upgraded", U"強化済み", U"UPGRADED");
	}

	[[nodiscard]] inline String GetCommandStatusNoTarget()
	{
		return Localization::GetText(U"battle.command.status.no_target", U"対象なし", U"NO TARGET");
	}

	[[nodiscard]] inline String GetCommandStatusFullHp()
	{
		return Localization::GetText(U"battle.command.status.full_hp", U"HP満タン", U"FULL HP");
	}

	[[nodiscard]] inline String GetCommandStatusArmed()
	{
		return Localization::GetText(U"battle.command.status.armed", U"起爆中", U"ARMED");
	}

	[[nodiscard]] inline String GetRepairCommandLabel()
	{
		return Localization::GetText(U"battle.command.label.repair", U"修理", U"REPAIR");
	}

	[[nodiscard]] inline String GetRepairCommandDescription()
	{
		return Localization::GetText(U"battle.command.desc.repair", U"押したあと、傷ついた砲台か拠点をクリックして修理します。", U"After pressing this, click a damaged turret or base to repair it.");
	}

	[[nodiscard]] inline String GetRepairCommandFlavorText()
	{
		return Localization::GetText(U"battle.command.flavor.repair", U"先に命令してから、傷んだ砲台か拠点を選ぶ。", U"Issue the order first, then pick the damaged turret or base.");
	}

	[[nodiscard]] inline String GetDetonateCommandLabel()
	{
		return Localization::GetText(U"battle.command.label.detonate", U"起爆", U"DETONATE");
	}

	[[nodiscard]] inline String GetDetonateCommandDescription()
	{
		return Localization::GetText(U"battle.command.desc.detonate", U"選択中のゴライアスを短い導火時間のあと自爆させます。", U"Self-destruct selected Goliaths after a short fuse.");
	}

	[[nodiscard]] inline String GetDetonateCommandFlavorText()
	{
		return Localization::GetText(U"battle.command.flavor.detonate", U"近づけて押すだけ。落とされても周囲を巻き込む。", U"Just get close and trigger it. Even if it gets shot down, it still catches nearby enemies.");
	}

	[[nodiscard]] inline String GetHudTitle(const BattleConfigData& config)
	{
		const String english = config.hud.title.isEmpty() ? U"LoganToga2 Remake Prototype" : config.hud.title;
		return Localization::GetText(U"battle.hud.title", U"LoganToga2 Remake Prototype", english);
	}

	[[nodiscard]] inline String GetResourceSummary(const int32 pointCount, const int32 income)
	{
		return Localization::FormatText(U"battle.hud.resource_summary", U"資源拠点: {0} 箇所 / +{1} 収入", U"Resource: {0} pts / +{1} income", pointCount, income);
	}

	[[nodiscard]] inline String GetRunBattleLabel(const int32 currentBattle, const int32 totalBattles)
	{
		return Localization::FormatText(U"battle.hud.run_battle", U"ラン: 戦闘 {0}/{1}", U"Run: battle {0}/{1}", currentBattle, totalBattles);
	}

	[[nodiscard]] inline String GetRunInactiveLabel()
	{
		return Localization::GetText(U"battle.hud.run_inactive", U"ラン: 非アクティブ", U"Run: inactive");
	}

	[[nodiscard]] inline String GetGoldLabel(const int32 gold)
	{
		return Localization::FormatText(U"battle.hud.gold", U"ゴールド: {0}", U"Gold: {0}", gold);
	}

	[[nodiscard]] inline String GetPlayerWinLabel()
	{
		return Localization::GetText(U"battle.hud.player_win", U"PLAYER WIN", U"PLAYER WIN");
	}

	[[nodiscard]] inline String GetEnemyWinLabel()
	{
		return Localization::GetText(U"battle.hud.enemy_win", U"ENEMY WIN", U"ENEMY WIN");
	}

	[[nodiscard]] inline String GetWinHint(const BattleConfigData& config)
	{
		const String english = config.hud.winHint.isEmpty() ? U"Enter: title / R: retry" : config.hud.winHint;
		return Localization::GetText(U"battle.hud.win_hint", U"Enter: タイトル / R: リトライ", english);
	}

	[[nodiscard]] inline String GetRunRewardHint()
	{
		return Localization::GetText(U"battle.hud.run_reward_hint", U"Enter: 報酬選択 / R: 新しいラン", U"Enter: choose reward / R: new run");
	}

	[[nodiscard]] inline String GetRunTitleHint()
	{
		return Localization::GetText(U"battle.hud.run_title_hint", U"Enter: タイトル / R: 新しいラン", U"Enter: title / R: new run");
	}

	[[nodiscard]] inline String GetControls(const BattleConfigData& config)
	{
		const String english = config.hud.controls.isEmpty()
			? U"L drag: pan / R click: move or attack / Q-W-E: formation / 1-0: command / X: cancel"
			: config.hud.controls;
		return Localization::GetText(U"battle.hud.controls", U"左ドラッグ: 視点移動 / 右クリック: 移動・攻撃 / Q-W-E: 陣形 / 1-0: コマンド / X: キャンセル", english);
	}

	[[nodiscard]] inline String GetEscapeHint(const BattleConfigData& config)
	{
		const String english = config.hud.escapeHint.isEmpty() ? U"Esc: pause menu" : config.hud.escapeHint;
		return Localization::GetText(U"battle.hud.escape_hint", U"Esc: ポーズメニュー", english);
	}

	[[nodiscard]] inline String GetResultVictoryTitle()
	{
		return Localization::GetText(U"battle.result.victory_title", U"勝利", U"Victory");
	}

	[[nodiscard]] inline String GetResultRunClearedTitle()
	{
		return Localization::GetText(U"battle.result.run_cleared_title", U"ラン制覇", U"Run Cleared");
	}

	[[nodiscard]] inline String GetResultDefeatTitle()
	{
		return Localization::GetText(U"battle.result.defeat_title", U"敗北", U"Defeat");
	}

	[[nodiscard]] inline String GetResultSubtitle(const int32 currentBattle, const int32 totalBattles, const size_t selectedCardCount)
	{
		return Localization::FormatText(U"battle.result.subtitle", U"戦闘 {0}/{1}   選択済みカード: {2}", U"Battle {0}/{1}   Cards selected: {2}", currentBattle, totalBattles, selectedCardCount);
	}

	[[nodiscard]] inline String GetResultRetryAction()
	{
		return Localization::GetText(U"battle.result.retry_action", U"R: 新しいランを開始", U"R: Start New Run");
	}

	[[nodiscard]] inline String GetResultEnterReturnTitle()
	{
		return Localization::GetText(U"battle.result.enter_return_title", U"Enter: タイトルへ戻る", U"Enter: Return to Title");
	}

	[[nodiscard]] inline String GetResultFooterRunEnded()
	{
		return Localization::GetText(U"battle.result.footer_run_ended", U"このランは終了しました", U"This run has ended");
	}

	[[nodiscard]] inline String GetResultEnterChooseReward()
	{
		return Localization::GetText(U"battle.result.enter_choose_reward", U"Enter: 報酬を選ぶ", U"Enter: Choose Reward");
	}

	[[nodiscard]] inline String GetResultFooterChooseReward()
	{
		return Localization::GetText(U"battle.result.footer_choose_reward", U"次の戦闘の前に報酬カードを1枚選びましょう", U"Choose 1 reward card before the next battle");
	}

	[[nodiscard]] inline String GetResultEnterBonusRoom()
	{
		return Localization::GetText(U"battle.result.enter_bonus_room", U"Enter: Bonus Room へ", U"Enter: Bonus Room");
	}

	[[nodiscard]] inline String GetResultFooterBonusRoom()
	{
		return Localization::GetText(U"battle.result.footer_bonus_room", U"ラン制覇後に Bonus Room を1つ選べます", U"Choose 1 bonus room after clearing the run");
	}

	[[nodiscard]] inline String GetResultFooterRunComplete()
	{
		return Localization::GetText(U"battle.result.footer_run_complete", U"すべての Bonus Room は閲覧済みです。ラン完了", U"All bonus rooms already viewed. Run complete");
	}

	[[nodiscard]] inline String GetPauseTitle()
	{
		return Localization::GetText(U"battle.pause.title", U"PAUSED", U"PAUSED");
	}

	[[nodiscard]] inline String GetPauseResume()
	{
		return Localization::GetText(U"battle.pause.resume", U"再開", U"Resume");
	}

	[[nodiscard]] inline String GetPauseRestartBattle()
	{
		return Localization::GetText(U"battle.pause.restart_battle", U"戦闘をやり直す", U"Restart Battle");
	}

	[[nodiscard]] inline String GetPauseReturnToTitle()
	{
		return Localization::GetText(U"battle.pause.return_to_title", U"タイトルへ戻る", U"Return to Title");
	}

	[[nodiscard]] inline String GetPauseSaveReturnToTitle()
	{
		return Localization::GetText(U"battle.pause.save_return_to_title", U"保存してタイトルへ戻る", U"Save & Return to Title");
	}

	[[nodiscard]] inline String GetPauseProductionPrefix()
	{
		return Localization::GetText(U"battle.pause.production_prefix", U"生産: ", U"Production: ");
	}

	[[nodiscard]] inline String GetPauseBuildNone()
	{
		return Localization::GetText(U"battle.pause.build_none", U"建築: なし", U"Build: none");
	}

	[[nodiscard]] inline String GetPauseBuildText(const int32 slot, const String& archetypeLabel, const int32 cost)
	{
		return Localization::FormatText(U"battle.pause.build_text", U"建築: {0}: {1} ({2}G)", U"Build: {0}: {1} ({2}G)", slot, archetypeLabel, cost);
	}

	[[nodiscard]] inline String GetPauseMenuHint()
	{
		return Localization::GetText(U"battle.pause.menu_hint", U"メニュー: Esc 再開 / 上下で選択 / Enter 決定", U"Menu: Esc resume / Up-Down select / Enter confirm");
	}

	[[nodiscard]] inline String GetQueueTitle()
	{
		return Localization::GetText(U"battle.queue.title", U"QUEUE", U"QUEUE");
	}

	[[nodiscard]] inline String GetQueueIdle()
	{
		return Localization::GetText(U"battle.queue.idle", U"待機中", U"Idle");
	}

	[[nodiscard]] inline String GetQueueNoQueuedUnits()
	{
		return Localization::GetText(U"battle.queue.no_queued_units", U"生産キューは空です", U"No queued units");
	}

	[[nodiscard]] inline String GetQueueConstructing()
	{
		return Localization::GetText(U"battle.queue.constructing", U"建設中", U"Constructing");
	}

	[[nodiscard]] inline String GetQueueSubtitle(const String& buildingLabel, const size_t queueCount)
	{
		return Localization::FormatText(U"battle.queue.subtitle", U"@ {0} / キュー {1}", U"@ {0} / {1} in queue", buildingLabel, queueCount);
	}

	[[nodiscard]] inline String GetQueueSelectHint(const String& buildingLabel)
	{
		return Localization::FormatText(U"battle.queue.select_hint", U"{0} を選択してユニットをキューに追加", U"Select {0} and queue a unit", buildingLabel);
	}

	[[nodiscard]] inline String GetWorldPlaceLabel(const UnitArchetype archetype)
	{
     return Localization::FormatText(U"battle.world.place", U"配置: {0}", U"Place {0}", GetLocalizedArchetypeLabel(archetype));
	}

	[[nodiscard]] inline String GetWorldBuildLabel(const UnitArchetype archetype)
	{
     return Localization::FormatText(U"battle.world.build", U"建築: {0}", U"BUILD {0}", GetLocalizedArchetypeLabel(archetype));
	}

	[[nodiscard]] inline String GetWorldConstructing()
	{
		return Localization::GetText(U"battle.world.constructing", U"建設中", U"Constructing");
	}

	[[nodiscard]] inline String GetCommandPanelWorkerTitle()
	{
		return Localization::GetText(U"battle.command.panel.worker", U"WORKER COMMANDS", U"WORKER COMMANDS");
	}

	[[nodiscard]] inline String GetCommandPanelArchetypeTitle(const String& archetypeLabel)
	{
		return Localization::FormatText(U"battle.command.panel.archetype", U"{0} COMMANDS", U"{0} COMMANDS", archetypeLabel);
	}

	[[nodiscard]] inline String GetCommandPanelSelectionTitle()
	{
		return Localization::GetText(U"battle.command.panel.selection", U"SELECTION COMMANDS", U"SELECTION COMMANDS");
	}

	[[nodiscard]] inline String GetCommandSectionCommands()
	{
		return Localization::GetText(U"battle.command.section.commands", U"COMMANDS", U"COMMANDS");
	}

	[[nodiscard]] inline String GetCommandSectionUpgrades()
	{
		return Localization::GetText(U"battle.command.section.upgrades", U"UPGRADES", U"UPGRADES");
	}

	[[nodiscard]] inline String GetCommandSectionRepair()
	{
		return Localization::GetText(U"battle.command.section.repair", U"REPAIR", U"REPAIR");
	}

	[[nodiscard]] inline String GetCommandSectionSpecial()
	{
		return Localization::GetText(U"battle.command.section.special", U"SPECIAL", U"SPECIAL");
	}

	[[nodiscard]] inline String GetCommandSectionConstruction()
	{
		return Localization::GetText(U"battle.command.section.construction", U"CONSTRUCTION", U"CONSTRUCTION");
	}

	[[nodiscard]] inline String GetCommandSectionProduction()
	{
		return Localization::GetText(U"battle.command.section.production", U"PRODUCTION", U"PRODUCTION");
	}

	[[nodiscard]] inline String GetCommandKindLabel(const CommandKind kind)
	{
		switch (kind)
		{
		case CommandKind::Construction:
			return Localization::GetText(U"battle.command.kind.build", U"Build", U"Build");
		case CommandKind::Repair:
			return Localization::GetText(U"battle.command.kind.repair", U"Repair", U"Repair");
		case CommandKind::Detonate:
			return Localization::GetText(U"battle.command.kind.detonate", U"Detonate", U"Detonate");
		case CommandKind::Upgrade:
			return Localization::GetText(U"battle.command.kind.upgrade", U"Upgrade", U"Upgrade");
		case CommandKind::Production:
		default:
			return Localization::GetText(U"battle.command.kind.queue", U"Queue", U"Queue");
		}
	}

	[[nodiscard]] inline String GetCommandCost(const int32 cost)
	{
		return Localization::FormatText(U"battle.command.cost", U"Cost: {0}G", U"Cost: {0}G", cost);
	}
}
