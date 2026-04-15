# pragma once
# include "UnitDefinitions.hpp"

namespace MainSupport
{
	struct UnitEditorSettings
	{
		Array<UnitParameters> parameters;
     Array<ExplosionSkillParameters> explosionSkillParameters;
		Array<FilePath> modelPaths;

		UnitEditorSettings()
		{
			parameters.resize(GetUnitDefinitions().size() * 2);
         explosionSkillParameters.resize(GetUnitDefinitions().size() * 2);
			modelPaths.resize(GetUnitDefinitions().size() * 2);

			for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
			{
				for (const auto& definition : GetUnitDefinitions())
				{
					parameters[GetUnitParameterSlotIndex(team, definition.unitType)] = MakeDefaultUnitParameters(team, definition.unitType);
                   explosionSkillParameters[GetUnitParameterSlotIndex(team, definition.unitType)] = MakeDefaultExplosionSkillParameters(team, definition.unitType);
				}
			}
		}
	};

	struct UnitEditorSelection
	{
		UnitTeam team = UnitTeam::Player;
		SapperUnitType unitType = SapperUnitType::Infantry;

		[[nodiscard]] bool operator==(const UnitEditorSelection& other) const
		{
			return ((team == other.team) && (unitType == other.unitType));
		}
	};

	enum class UnitEditorPage
	{
		Basic,
		Combat,
		Footprint,
      Skill,
	};

	[[nodiscard]] inline const UnitParameters& GetUnitParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitParameters& GetUnitParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline const FilePath& GetUnitModelPath(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.modelPaths[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline const ExplosionSkillParameters& GetExplosionSkillParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.explosionSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline ExplosionSkillParameters& GetExplosionSkillParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.explosionSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline FilePath& GetUnitModelPath(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.modelPaths[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitAiRole GetUnitAiRole(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return GetUnitParameters(settings, team, unitType).aiRole;
	}
}
