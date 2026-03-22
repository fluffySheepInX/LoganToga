#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleUiText
{
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
}
