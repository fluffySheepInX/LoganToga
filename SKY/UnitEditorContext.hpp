# pragma once
# include "UnitDefinitions.hpp"

namespace MainSupport
{
	struct UnitEditorSettings
	{
		Array<UnitParameters> parameters;

		UnitEditorSettings()
		{
			parameters.resize(GetUnitDefinitions().size() * 2);

			for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
			{
				for (const auto& definition : GetUnitDefinitions())
				{
					parameters[GetUnitParameterSlotIndex(team, definition.unitType)] = MakeDefaultUnitParameters(team, definition.unitType);
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
	};

	[[nodiscard]] inline const UnitParameters& GetUnitParameters(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitParameters& GetUnitParameters(UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return settings.parameters[GetUnitParameterSlotIndex(team, unitType)];
	}

	[[nodiscard]] inline UnitAiRole GetUnitAiRole(const UnitEditorSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
		return GetUnitParameters(settings, team, unitType).aiRole;
	}
}
