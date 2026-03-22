#pragma once

#include "BattleUiText.Archetype.h"
#include "Localization.h"

namespace BattleUiText
{
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
}
