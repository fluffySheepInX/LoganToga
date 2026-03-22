#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleUiText
{
 [[nodiscard]] inline String GetHudTitle(const BattleConfigData&)
	{
      return Localization::GetText(U"battle.hud.title");
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

  [[nodiscard]] inline String GetWinHint(const BattleConfigData&)
	{
      return Localization::GetText(U"battle.hud.win_hint");
	}

	[[nodiscard]] inline String GetRunRewardHint()
	{
		return Localization::GetText(U"battle.hud.run_reward_hint");
	}

	[[nodiscard]] inline String GetRunTitleHint()
	{
		return Localization::GetText(U"battle.hud.run_title_hint");
	}

 [[nodiscard]] inline String GetControls(const BattleConfigData&)
	{
        return Localization::GetText(U"battle.hud.controls");
	}

   [[nodiscard]] inline String GetEscapeHint(const BattleConfigData&)
	{
        return Localization::GetText(U"battle.hud.escape_hint");
	}
}
