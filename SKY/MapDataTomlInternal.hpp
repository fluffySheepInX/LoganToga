# pragma once
# include "MapData.hpp"

namespace MapDataTomlDetail
{
	[[nodiscard]] inline double SanitizeMillAttackRange(double v) { return Clamp(v, 1.0, 20.0); }
	[[nodiscard]] inline double SanitizeMillAttackDamage(double v) { return Clamp(v, 1.0, 80.0); }
	[[nodiscard]] inline double SanitizeMillAttackInterval(double v) { return Clamp(v, 0.2, 5.0); }
	[[nodiscard]] inline int32 SanitizeMillAttackTargetCount(int64 v) { return Clamp<int32>(static_cast<int32>(v), 1, 6); }
	[[nodiscard]] inline double SanitizeMillSuppressionDuration(double v) { return Clamp(v, 0.2, 10.0); }
	[[nodiscard]] inline double SanitizeMillSuppressionMoveSpeedMultiplier(double v) { return Clamp(v, 0.1, 1.0); }
	[[nodiscard]] inline double SanitizeMillSuppressionAttackDamageMultiplier(double v) { return Clamp(v, 0.1, 1.0); }
	[[nodiscard]] inline double SanitizeMillSuppressionAttackIntervalMultiplier(double v) { return Clamp(v, 1.0, 10.0); }
	[[nodiscard]] inline double SanitizeNavPointRadius(double v) { return Clamp(v, 0.5, 8.0); }
	[[nodiscard]] inline double SanitizeNavLinkCostMultiplier(double v) { return Clamp(v, 0.1, 10.0); }
	[[nodiscard]] inline double SanitizeWallLength(double v) { return Clamp(v, 2.0, 80.0); }
	[[nodiscard]] inline double SanitizeRoadSpan(double v) { return Clamp(v, 2.0, 80.0); }

	template <class TomlNode>
	[[nodiscard]] Vec3 ReadTomlVec3(const TomlNode& toml, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<double>();
				if (values.size() >= 3) break;
			}

			if (values.size() < 3) return fallback;

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	void WriteTomlVec3(TextWriter& writer, const String& key, const Vec3& value);
	void AppendLoadMessage(String& message, StringView detail);
	[[nodiscard]] bool HasTomlTableArraySection(FilePathView path, const String& key);
	[[nodiscard]] Optional<PlaceableModelType> ParsePlaceableModelType(const String& value);
	[[nodiscard]] Optional<TerrainCellType> ParseTerrainCellType(const String& value);
	[[nodiscard]] Optional<MainSupport::ResourceType> ParseResourceType(const String& value);
	[[nodiscard]] Optional<MainSupport::UnitTeam> ParseUnitTeam(const String& value);
}
