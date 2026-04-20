# include "MainSettingsInternal.hpp"
# include "SettingsRegistry.hpp"
# include "SkillDescriptors.hpp"

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
				const size_t slot = GetUnitParameterSlotIndex(team, unitDefinition.unitType);

				SettingsDetail::LoadUnitParameterGroup(toml,
					groupKey,
					GetUnitParameters(settings, team, unitDefinition.unitType));

				ForEachSkill([&](const auto& descriptor)
				{
					descriptor.load(toml,
						(groupKey + String{ descriptor.keySuffix }),
						(settings.*(descriptor.arrayMember))[slot]);
				});

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
		return SaveSettingsFile(UnitSettingsPath, [&](TextWriter& writer)
		{
			bool needsBlankLine = false;
			for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
			{
				for (const auto& unitDefinition : GetUnitDefinitions())
				{
					const String groupKey = GetUnitSettingsGroupKey(team, unitDefinition.unitType);
					const size_t slot = GetUnitParameterSlotIndex(team, unitDefinition.unitType);

					if (needsBlankLine)
					{
						writer.writeln(U"");
					}

					SettingsDetail::SaveUnitParameterGroup(writer,
						groupKey,
						GetUnitParameters(settings, team, unitDefinition.unitType));

					ForEachSkill([&](const auto& descriptor)
					{
						descriptor.save(writer,
							(groupKey + String{ descriptor.keySuffix }),
							(settings.*(descriptor.arrayMember))[slot]);
					});

					const FilePath& modelPath = GetUnitModelPath(settings, team, unitDefinition.unitType);
					if (not modelPath.isEmpty())
					{
						writer.writeln(U"{}ModelPath = \"{}\""_fmt(groupKey, EscapeTomlString(modelPath)));
					}
					needsBlankLine = true;
				}
			}
		});
	}
}
