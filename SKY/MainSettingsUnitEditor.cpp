# include "MainSettingsInternal.hpp"

namespace MainSupport
{
	UnitEditorSettings LoadUnitEditorSettings()
	{
		const TOMLReader toml{ UnitSettingsPath };
		UnitEditorSettings settings;

		if (not toml)
		{
			return settings;
		}

		for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
		{
			for (const auto& unitDefinition : GetUnitDefinitions())
			{
				SettingsDetail::LoadUnitParameterGroup(toml,
					GetUnitSettingsGroupKey(team, unitDefinition.unitType),
					GetUnitParameters(settings, team, unitDefinition.unitType));
			}
		}

		return settings;
	}

	bool SaveUnitEditorSettings(const UnitEditorSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ UnitSettingsPath };

		if (not writer)
		{
			return false;
		}

		bool needsBlankLine = false;
		for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
		{
			for (const auto& unitDefinition : GetUnitDefinitions())
			{
				if (needsBlankLine)
				{
					writer.writeln(U"");
				}

				SettingsDetail::SaveUnitParameterGroup(writer,
					GetUnitSettingsGroupKey(team, unitDefinition.unitType),
					GetUnitParameters(settings, team, unitDefinition.unitType));
				needsBlankLine = true;
			}
		}

		return true;
	}
}
