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

	inline constexpr size_t FormationSlotCount = 8;
	inline constexpr size_t FormationPresetCount = 3;
	inline constexpr int32 FormationSaveSchemaVersion = 1;

	[[nodiscard]] inline Array<Optional<AllyBehavior>> MakeEmptyFormationSlots()
	{
		return Array<Optional<AllyBehavior>>(FormationSlotCount);
	}

	[[nodiscard]] inline Array<Array<Optional<AllyBehavior>>> MakeDefaultFormationPresets()
	{
		Array<Array<Optional<AllyBehavior>>> presets;
		presets.reserve(FormationPresetCount);

		for (size_t index = 0; index < FormationPresetCount; ++index)
		{
			presets << MakeEmptyFormationSlots();
		}

		return presets;
	}

	[[nodiscard]] inline String GetFormationSavePath()
	{
		return U"save/formation.toml";
	}

	[[nodiscard]] inline Optional<AllyBehavior> ToPersistentAllyBehavior(const int32 value)
	{
		switch (value)
		{
		case static_cast<int32>(AllyBehavior::ChaseEnemies):
			return AllyBehavior::ChaseEnemies;

		case static_cast<int32>(AllyBehavior::HoldPosition):
			return AllyBehavior::HoldPosition;

		case static_cast<int32>(AllyBehavior::GuardPlayer):
			return AllyBehavior::GuardPlayer;

		case static_cast<int32>(AllyBehavior::OrbitPlayer):
			return AllyBehavior::OrbitPlayer;

		case static_cast<int32>(AllyBehavior::FixedTurret):
			return AllyBehavior::FixedTurret;

		default:
			return none;
		}
	}

	[[nodiscard]] inline int32 ToPersistentValue(const Optional<AllyBehavior>& behavior)
	{
		return behavior ? static_cast<int32>(*behavior) : -1;
	}

	[[nodiscard]] inline String BuildTomlIntArray(const Array<int32>& values)
	{
		String result = U"[";

		for (size_t index = 0; index < values.size(); ++index)
		{
			if (index > 0)
			{
				result += U", ";
			}

			result += Format(values[index]);
		}

		result += U"]";
		return result;
	}

	[[nodiscard]] inline Array<int32> SerializeFormationSlots(const Array<Optional<AllyBehavior>>& slots)
	{
		Array<int32> values;
		values.reserve(slots.size());

		for (const auto& slot : slots)
		{
			values << ToPersistentValue(slot);
		}

		return values;
	}

	inline void DeserializeFormationSlots(const Array<int32>& values, Array<Optional<AllyBehavior>>& slots)
	{
		slots = MakeEmptyFormationSlots();

		for (size_t index = 0; index < Min(slots.size(), values.size()); ++index)
		{
			slots[index] = ToPersistentAllyBehavior(values[index]);
		}
	}

	[[nodiscard]] inline Array<int32> ReadTomlIntArray(const TOMLReader& toml, const String& key)
	{
		Array<int32> values;

		try
		{
			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<int32>();
			}
		}
		catch (const std::exception&)
		{
			values.clear();
		}

		return values;
	}
}

struct AppData
{
    Array<Optional<ff::AllyBehavior>> formationSlots = ff::MakeEmptyFormationSlots();
	Array<Array<Optional<ff::AllyBehavior>>> formationPresets = ff::MakeDefaultFormationPresets();
	Optional<ff::AllyBehavior> selectedFormationUnit = ff::AllyBehavior::GuardPlayer;
  ff::TimeOfDay timeOfDay = ff::TimeOfDay::Day;
};

[[nodiscard]] inline AppData LoadAppDataFromDisk()
{
	AppData data;
	const TOMLReader toml{ ff::GetFormationSavePath() };

	if (!toml)
	{
		return data;
	}

	try
	{
		if (toml[U"schemaVersion"].get<int32>() != ff::FormationSaveSchemaVersion)
		{
			return data;
		}
	}
	catch (const std::exception&)
	{
		return data;
	}

	ff::DeserializeFormationSlots(ff::ReadTomlIntArray(toml, U"formationSlots"), data.formationSlots);

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
		ff::DeserializeFormationSlots(ff::ReadTomlIntArray(toml, U"preset{}"_fmt(index + 1)), data.formationPresets[index]);
	}

	try
	{
		data.selectedFormationUnit = ff::ToPersistentAllyBehavior(toml[U"selectedFormationUnit"].get<int32>());
	}
	catch (const std::exception&)
	{
	}

	return data;
}

[[nodiscard]] inline bool SaveAppDataToDisk(const AppData& data)
{
	const String savePath = ff::GetFormationSavePath();
	FileSystem::CreateDirectories(FileSystem::ParentPath(savePath));

	TextWriter writer{ savePath };
	if (!writer)
	{
		return false;
	}

	String content;
	content += U"schemaVersion = {}\n"_fmt(ff::FormationSaveSchemaVersion);
	content += U"formationSlots = {}\n"_fmt(ff::BuildTomlIntArray(ff::SerializeFormationSlots(data.formationSlots)));

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
		content += U"preset{} = {}\n"_fmt(index + 1, ff::BuildTomlIntArray(ff::SerializeFormationSlots(data.formationPresets[index])));
	}

	content += U"selectedFormationUnit = {}\n"_fmt(ff::ToPersistentValue(data.selectedFormationUnit));
	writer.write(content);
	return true;
}

using App = SceneManager<String, AppData>;
