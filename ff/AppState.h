# pragma once
# include <Siv3D.hpp>
# include "GameConstants.h"

namespace ff
{
	enum class TimeOfDay
	{
		Day,
		Evening,
		Night,
	};
}

struct AppData
{
	Array<Optional<ff::AllyBehavior>> formationSlots = Array<Optional<ff::AllyBehavior>>(8);
   Array<Array<Optional<ff::AllyBehavior>>> formationPresets = {
		Array<Optional<ff::AllyBehavior>>(8),
		Array<Optional<ff::AllyBehavior>>(8),
		Array<Optional<ff::AllyBehavior>>(8),
	};
	Optional<ff::AllyBehavior> selectedFormationUnit = ff::AllyBehavior::GuardPlayer;
  ff::TimeOfDay timeOfDay = ff::TimeOfDay::Day;
};

using App = SceneManager<String, AppData>;
