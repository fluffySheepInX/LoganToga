#pragma once

#include "Localization.h"

namespace BattleUiText
{
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
}
