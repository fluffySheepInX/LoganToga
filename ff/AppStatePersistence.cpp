# include "AppStatePersistence.h"

namespace
{
	[[nodiscard]] Optional<ff::UnitId> ToLegacyPersistentUnitId(const int32 value)
	{
		switch (value)
		{
		case static_cast<int32>(ff::UnitId::ChaseEnemies):
			return ff::UnitId::ChaseEnemies;

		case static_cast<int32>(ff::UnitId::HoldPosition):
			return ff::UnitId::HoldPosition;

		case static_cast<int32>(ff::UnitId::GuardPlayer):
			return ff::UnitId::GuardPlayer;

		case static_cast<int32>(ff::UnitId::OrbitPlayer):
			return ff::UnitId::OrbitPlayer;

		case static_cast<int32>(ff::UnitId::FixedTurret):
			return ff::UnitId::FixedTurret;

		default:
			return none;
		}
	}

	[[nodiscard]] String EscapeTomlString(const String& value)
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

	[[nodiscard]] String ToPersistentValue(const Optional<ff::UnitId>& unitId)
	{
		return unitId ? String{ ff::GetUnitStableId(*unitId) } : U"";
	}

	[[nodiscard]] String BuildTomlIntArray(const Array<int32>& values)
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

	[[nodiscard]] String BuildTomlStringArray(const Array<String>& values)
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

	[[nodiscard]] Array<String> SerializeFormationSlots(const ff::FormationSlots& slots)
	{
		Array<String> values;
		values.reserve(slots.size());

		for (const auto& slot : slots)
		{
			values << ToPersistentValue(slot);
		}

		return values;
	}

	inline void DeserializeFormationSlots(const Array<String>& values, ff::FormationSlots& slots)
	{
		slots = ff::MakeEmptyFormationSlots();

		for (size_t index = 0; index < Min(slots.size(), values.size()); ++index)
		{
			slots[index] = values[index].isEmpty() ? none : ff::ParseUnitId(values[index]);
		}
	}

	inline void DeserializeLegacyFormationSlots(const Array<int32>& values, ff::FormationSlots& slots)
	{
		slots = ff::MakeEmptyFormationSlots();

		for (size_t index = 0; index < Min(slots.size(), values.size()); ++index)
		{
			slots[index] = ToLegacyPersistentUnitId(values[index]);
		}
	}

	[[nodiscard]] Array<int32> ReadTomlIntArray(const TOMLReader& toml, const String& key)
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

	[[nodiscard]] Array<String> ReadTomlStringArray(const TOMLReader& toml, const String& key)
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

	[[nodiscard]] Optional<ff::WaveTrait> ToPersistentWaveTrait(const int32 value)
	{
		switch (value)
		{
		case static_cast<int32>(ff::WaveTrait::None):
			return ff::WaveTrait::None;

		case static_cast<int32>(ff::WaveTrait::Reinforced):
			return ff::WaveTrait::Reinforced;

		case static_cast<int32>(ff::WaveTrait::Assault):
			return ff::WaveTrait::Assault;

		case static_cast<int32>(ff::WaveTrait::Bounty):
			return ff::WaveTrait::Bounty;

		default:
			return none;
		}
	}

	[[nodiscard]] Array<int32> SerializeWaveTraits(const Array<ff::WaveTrait>& traits)
	{
		Array<int32> values;
		values.reserve(traits.size());

		for (const auto trait : traits)
		{
			if (trait != ff::WaveTrait::None)
			{
				values << static_cast<int32>(trait);
			}
		}

		return values;
	}

	inline void DeserializeWaveTraits(const Array<int32>& values, Array<ff::WaveTrait>& traits)
	{
		traits.clear();

		for (const int32 value : values)
		{
			if (const auto trait = ToPersistentWaveTrait(value))
			{
				if ((*trait != ff::WaveTrait::None) && (not traits.contains(*trait)))
				{
					traits << *trait;
				}
			}
		}
	}

	inline void DeserializeSummonDiscountTraitConfig(const TOMLReader& toml, ff::SummonDiscountTraitConfig& config)
	{
		for (size_t index = 0; index < config.size(); ++index)
		{
			DeserializeWaveTraits(ReadTomlIntArray(toml, U"summonDiscountTraits{}"_fmt(index)), config[index]);
		}
	}
}

AppData LoadAppDataFromDisk()
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
		DeserializeFormationSlots(ReadTomlStringArray(toml, U"formationSlots"), data.formationSlots);
	}
	else
	{
		DeserializeLegacyFormationSlots(ReadTomlIntArray(toml, U"formationSlots"), data.formationSlots);
	}

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
		if (schemaVersion >= 3)
		{
			DeserializeFormationSlots(ReadTomlStringArray(toml, U"preset{}"_fmt(index + 1)), data.formationPresets[index]);
		}
		else
		{
			DeserializeLegacyFormationSlots(ReadTomlIntArray(toml, U"preset{}"_fmt(index + 1)), data.formationPresets[index]);
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
			data.selectedFormationUnit = ToLegacyPersistentUnitId(toml[U"selectedFormationUnit"].get<int32>());
		}
	}
	catch (const std::exception&)
	{
	}

	if (schemaVersion >= 2)
	{
		DeserializeSummonDiscountTraitConfig(toml, data.summonDiscountTraits);
	}

	return data;
}

bool SaveAppDataToDisk(const AppData& data)
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
	content += U"formationSlots = {}\n"_fmt(BuildTomlStringArray(SerializeFormationSlots(data.formationSlots)));

	for (size_t index = 0; index < data.formationPresets.size(); ++index)
	{
		content += U"preset{} = {}\n"_fmt(index + 1, BuildTomlStringArray(SerializeFormationSlots(data.formationPresets[index])));
	}

	content += U"selectedFormationUnit = \"{}\"\n"_fmt(EscapeTomlString(ToPersistentValue(data.selectedFormationUnit)));

	for (size_t index = 0; index < data.summonDiscountTraits.size(); ++index)
	{
		content += U"summonDiscountTraits{} = {}\n"_fmt(index, BuildTomlIntArray(SerializeWaveTraits(data.summonDiscountTraits[index])));
	}

	writer.write(content);
	return true;
}
