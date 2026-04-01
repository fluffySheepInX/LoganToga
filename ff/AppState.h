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
  inline constexpr int32 FormationSaveSchemaVersion = 3;
	using FormationSlotUnit = Optional<UnitId>;
	using FormationSlots = Array<FormationSlotUnit>;

	struct FormationEditState
	{
       FormationSlots slots = FormationSlots(FormationSlotCount);
		Optional<UnitId> selectedUnit = UnitId::GuardPlayer;
	};

    [[nodiscard]] inline FormationSlots MakeEmptyFormationSlots()
	{
       return FormationSlots(FormationSlotCount);
	}

 [[nodiscard]] inline Array<FormationSlots> MakeDefaultFormationPresets()
	{
       Array<FormationSlots> presets;
		presets.reserve(FormationPresetCount);

		for (size_t index = 0; index < FormationPresetCount; ++index)
		{
			presets << MakeEmptyFormationSlots();
		}

		return presets;
	}

	[[nodiscard]] inline int32 CountAssignedFormationUnits(const FormationSlots& slots)
	{
		return static_cast<int32>(std::count_if(slots.begin(), slots.end(), [](const auto& slot)
			{
				return static_cast<bool>(slot);
			}));
	}

	[[nodiscard]] inline bool HasAssignedFormationSlot(const FormationSlots& slots)
	{
		return std::any_of(slots.begin(), slots.end(), [](const auto& slot)
			{
				return static_cast<bool>(slot);
			});
	}

	[[nodiscard]] inline FormationSlots MakeRandomFormationSlots()
	{
		const auto& unitTypes = GetAvailableUnitIds();
		FormationSlots slots = MakeEmptyFormationSlots();

		for (auto& slot : slots)
		{
			slot = unitTypes[Random(unitTypes.size() - 1)];
		}

		return slots;
	}

	inline void ClearFormationSlots(FormationSlots& slots)
	{
		for (auto& slot : slots)
		{
			slot.reset();
		}
	}

	[[nodiscard]] inline bool AssignSelectedFormationUnit(FormationEditState& state, const size_t index)
	{
		if ((index >= state.slots.size()) || (not state.selectedUnit))
		{
			return false;
		}

		state.slots[index] = state.selectedUnit;
		return true;
	}

	[[nodiscard]] inline bool ClearFormationSlot(FormationSlots& slots, const size_t index)
	{
		if (index >= slots.size())
		{
			return false;
		}

		slots[index].reset();
		return true;
	}

	[[nodiscard]] inline String GetFormationSavePath()
	{
		return U"save/formation.toml";
	}

 [[nodiscard]] inline Optional<UnitId> ToLegacyPersistentUnitId(const int32 value)
	{
		switch (value)
		{
        case static_cast<int32>(UnitId::ChaseEnemies):
			return UnitId::ChaseEnemies;

        case static_cast<int32>(UnitId::HoldPosition):
			return UnitId::HoldPosition;

     case static_cast<int32>(UnitId::GuardPlayer):
			return UnitId::GuardPlayer;

     case static_cast<int32>(UnitId::OrbitPlayer):
			return UnitId::OrbitPlayer;

     case static_cast<int32>(UnitId::FixedTurret):
			return UnitId::FixedTurret;

		default:
			return none;
		}
	}

    [[nodiscard]] inline String EscapeTomlString(const String& value)
	{
       String escaped;
		escaped.reserve(value.size());

		for (const auto ch : value)
		{
			if (ch == U'\\')
			{
				escaped += U"\\\\";
			}
			else if (ch == U'\"')
			{
				escaped += U"\\\"";
			}
			else
			{
				escaped.push_back(ch);
			}
		}

		return escaped;
	}

	[[nodiscard]] inline String ToPersistentValue(const Optional<UnitId>& unitId)
	{
		return unitId ? String{ GetUnitStableId(*unitId) } : U"";
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

   [[nodiscard]] inline String BuildTomlStringArray(const Array<String>& values)
	{
		String result = U"[";

		for (size_t index = 0; index < values.size(); ++index)
		{
			if (index > 0)
			{
				result += U", ";
			}

			result += U"\"{}\""_fmt(EscapeTomlString(values[index]));
		}

		result += U"]";
		return result;
	}

	[[nodiscard]] inline Array<String> SerializeFormationSlots(const FormationSlots& slots)
	{
        Array<String> values;
		values.reserve(slots.size());

		for (const auto& slot : slots)
		{
			values << ToPersistentValue(slot);
		}

		return values;
	}

 inline void DeserializeFormationSlots(const Array<String>& values, FormationSlots& slots)
	{
		slots = MakeEmptyFormationSlots();

		for (size_t index = 0; index < Min(slots.size(), values.size()); ++index)
		{
         slots[index] = values[index].isEmpty() ? none : ParseUnitId(values[index]);
		}
	}

	inline void DeserializeLegacyFormationSlots(const Array<int32>& values, FormationSlots& slots)
	{
		slots = MakeEmptyFormationSlots();

		for (size_t index = 0; index < Min(slots.size(), values.size()); ++index)
		{
			slots[index] = ToLegacyPersistentUnitId(values[index]);
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

	[[nodiscard]] inline Array<String> ReadTomlStringArray(const TOMLReader& toml, const String& key)
	{
		Array<String> values;

		try
		{
			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<String>();
			}
		}
		catch (const std::exception&)
		{
			values.clear();
		}

		return values;
	}

	[[nodiscard]] inline Optional<WaveTrait> ToPersistentWaveTrait(const int32 value)
	{
		switch (value)
		{
		case static_cast<int32>(WaveTrait::None):
			return WaveTrait::None;

		case static_cast<int32>(WaveTrait::Reinforced):
			return WaveTrait::Reinforced;

		case static_cast<int32>(WaveTrait::Assault):
			return WaveTrait::Assault;

		case static_cast<int32>(WaveTrait::Bounty):
			return WaveTrait::Bounty;

		default:
			return none;
		}
	}

	[[nodiscard]] inline Array<int32> SerializeWaveTraits(const Array<WaveTrait>& traits)
	{
		Array<int32> values;
		values.reserve(traits.size());

		for (const auto trait : traits)
		{
			if (trait != WaveTrait::None)
			{
				values << static_cast<int32>(trait);
			}
		}

		return values;
	}

	inline void DeserializeWaveTraits(const Array<int32>& values, Array<WaveTrait>& traits)
	{
		traits.clear();

		for (const int32 value : values)
		{
			if (const auto trait = ToPersistentWaveTrait(value))
			{
				if ((*trait != WaveTrait::None) && (not traits.contains(*trait)))
				{
					traits << *trait;
				}
			}
		}
	}

	inline void DeserializeSummonDiscountTraitConfig(const TOMLReader& toml, SummonDiscountTraitConfig& config)
	{
		for (size_t index = 0; index < config.size(); ++index)
		{
			DeserializeWaveTraits(ReadTomlIntArray(toml, U"summonDiscountTraits{}"_fmt(index)), config[index]);
		}
	}
}

struct AppData
{
    ff::FormationSlots formationSlots = ff::MakeEmptyFormationSlots();
	Array<ff::FormationSlots> formationPresets = ff::MakeDefaultFormationPresets();
	Optional<ff::UnitId> selectedFormationUnit = ff::UnitId::GuardPlayer;
     ff::SummonDiscountTraitConfig summonDiscountTraits = ff::MakeDefaultSummonDiscountTraitConfig();
  ff::TimeOfDay timeOfDay = ff::TimeOfDay::Day;
};

[[nodiscard]] inline ff::FormationEditState MakeFormationEditState(const AppData& data)
{
	return ff::FormationEditState{ data.formationSlots, data.selectedFormationUnit };
}

inline void ApplyFormationEditState(AppData& data, const ff::FormationEditState& state)
{
	data.formationSlots = state.slots;
	data.selectedFormationUnit = state.selectedUnit;
}

[[nodiscard]] inline AppData LoadAppDataFromDisk()
{
	AppData data;
	const TOMLReader toml{ ff::GetFormationSavePath() };

	if (!toml)
	{
		return data;
	}

 int32 schemaVersion = 1;

	try
	{
      schemaVersion = toml[U"schemaVersion"].get<int32>();

		if ((schemaVersion <= 0) || (schemaVersion > ff::FormationSaveSchemaVersion))
		{
			return data;
		}
	}
	catch (const std::exception&)
	{
		return data;
	}

  if (schemaVersion >= 3)
	{
		ff::DeserializeFormationSlots(ff::ReadTomlStringArray(toml, U"formationSlots"), data.formationSlots);
	}
	else
	{
		ff::DeserializeLegacyFormationSlots(ff::ReadTomlIntArray(toml, U"formationSlots"), data.formationSlots);
	}

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
        if (schemaVersion >= 3)
		{
			ff::DeserializeFormationSlots(ff::ReadTomlStringArray(toml, U"preset{}"_fmt(index + 1)), data.formationPresets[index]);
		}
		else
		{
			ff::DeserializeLegacyFormationSlots(ff::ReadTomlIntArray(toml, U"preset{}"_fmt(index + 1)), data.formationPresets[index]);
		}
	}

	try
	{
     if (schemaVersion >= 3)
		{
			const String persistentUnitId = toml[U"selectedFormationUnit"].get<String>();
			data.selectedFormationUnit = persistentUnitId.isEmpty() ? none : ff::ParseUnitId(persistentUnitId);
		}
		else
		{
			data.selectedFormationUnit = ff::ToLegacyPersistentUnitId(toml[U"selectedFormationUnit"].get<int32>());
		}
	}
	catch (const std::exception&)
	{
	}

	if (schemaVersion >= 2)
	{
		ff::DeserializeSummonDiscountTraitConfig(toml, data.summonDiscountTraits);
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
   content += U"formationSlots = {}\n"_fmt(ff::BuildTomlStringArray(ff::SerializeFormationSlots(data.formationSlots)));

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
     content += U"preset{} = {}\n"_fmt(index + 1, ff::BuildTomlStringArray(ff::SerializeFormationSlots(data.formationPresets[index])));
	}

  content += U"selectedFormationUnit = \"{}\"\n"_fmt(ff::EscapeTomlString(ff::ToPersistentValue(data.selectedFormationUnit)));

	for (size_t index = 0; index < data.summonDiscountTraits.size(); ++index)
	{
		content += U"summonDiscountTraits{} = {}\n"_fmt(index, ff::BuildTomlIntArray(ff::SerializeWaveTraits(data.summonDiscountTraits[index])));
	}

	writer.write(content);
	return true;
}

using App = SceneManager<String, AppData>;
