#pragma once

#include "Localization.h"

namespace BattleUiText
{
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
}
