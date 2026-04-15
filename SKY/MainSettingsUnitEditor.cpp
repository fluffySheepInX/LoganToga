# include "MainSettingsInternal.hpp"

namespace MainSupport
{
 namespace
	{
		[[nodiscard]] String EscapeTomlString(StringView value)
		{
			return String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
		}
	}

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
                const String groupKey = GetUnitSettingsGroupKey(team, unitDefinition.unitType);
				SettingsDetail::LoadUnitParameterGroup(toml,
                 groupKey,
					GetUnitParameters(settings, team, unitDefinition.unitType));

				if (const auto modelPath = toml[(groupKey + U"ModelPath")].getOpt<String>())
				{
					GetUnitModelPath(settings, team, unitDefinition.unitType) = *modelPath;
				}
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
             const String groupKey = GetUnitSettingsGroupKey(team, unitDefinition.unitType);
				if (needsBlankLine)
				{
					writer.writeln(U"");
				}

				SettingsDetail::SaveUnitParameterGroup(writer,
                 groupKey,
					GetUnitParameters(settings, team, unitDefinition.unitType));

				const FilePath& modelPath = GetUnitModelPath(settings, team, unitDefinition.unitType);
				if (not modelPath.isEmpty())
				{
					writer.writeln(U"{}ModelPath = \"{}\""_fmt(groupKey, EscapeTomlString(modelPath)));
				}
				needsBlankLine = true;
			}
		}

		return true;
	}
}
