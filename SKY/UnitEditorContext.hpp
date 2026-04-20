# pragma once
# include "UnitDefinitions.hpp"

namespace MainSupport
{
	struct UnitEditorSettings
	{
		Array<UnitParameters> parameters;
	 Array<ExplosionSkillParameters> explosionSkillParameters;
	 Array<BuildMillSkillParameters> buildMillSkillParameters;
		Array<HealSkillParameters> healSkillParameters;
		Array<ScoutSkillParameters> scoutSkillParameters;
		Array<FilePath> modelPaths;

		UnitEditorSettings()
		{
			const size_t total = (GetUnitDefinitions().size() * 2);
			parameters.resize(total);
			explosionSkillParameters.resize(total);
			buildMillSkillParameters.resize(total);
			healSkillParameters.resize(total);
			scoutSkillParameters.resize(total);
			modelPaths.resize(total);

			for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
			{
				for (const auto& definition : GetUnitDefinitions())
				{
					const size_t slot = GetUnitParameterSlotIndex(team, definition.unitType);
					parameters[slot] = MakeDefaultUnitParameters(team, definition.unitType);
					explosionSkillParameters[slot] = MakeDefaultExplosionSkillParameters(team, definition.unitType);
					buildMillSkillParameters[slot] = MakeDefaultBuildMillSkillParameters(team, definition.unitType);
					healSkillParameters[slot] = MakeDefaultHealSkillParameters(team, definition.unitType);
					scoutSkillParameters[slot] = MakeDefaultScoutSkillParameters(team, definition.unitType);
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

	[[nodiscard]] inline const BuildMillSkillParameters& GetBuildMillSkillParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.buildMillSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline BuildMillSkillParameters& GetBuildMillSkillParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.buildMillSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline const HealSkillParameters& GetHealSkillParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.healSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline HealSkillParameters& GetHealSkillParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.healSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline const ScoutSkillParameters& GetScoutSkillParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.scoutSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline ScoutSkillParameters& GetScoutSkillParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.scoutSkillParameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitAiRole GetUnitAiRole(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return GetUnitParameters(settings, team, unitType).aiRole;
	}
}
