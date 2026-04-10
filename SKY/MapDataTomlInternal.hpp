# pragma once
# include "MapData.hpp"

namespace MapDataTomlDetail
{
	[[nodiscard]] double SanitizeMillAttackRange(double value);
	[[nodiscard]] double SanitizeMillAttackDamage(double value);
	[[nodiscard]] double SanitizeMillAttackInterval(double value);
 [[nodiscard]] int32 SanitizeMillAttackTargetCount(int64 value);
	[[nodiscard]] double SanitizeMillSuppressionDuration(double value);
	[[nodiscard]] double SanitizeMillSuppressionMoveSpeedMultiplier(double value);
	[[nodiscard]] double SanitizeMillSuppressionAttackDamageMultiplier(double value);
	[[nodiscard]] double SanitizeMillSuppressionAttackIntervalMultiplier(double value);
	[[nodiscard]] double SanitizeNavPointRadius(double value);
	[[nodiscard]] double SanitizeNavLinkCostMultiplier(double value);
   [[nodiscard]] double SanitizeWallLength(double value);
   [[nodiscard]] double SanitizeRoadSpan(double value);
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback);
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLValue& tomlValue, const String& key, const Vec3& fallback);
	void WriteTomlVec3(TextWriter& writer, const String& key, const Vec3& value);
	void AppendLoadMessage(String& message, StringView detail);
	[[nodiscard]] bool HasTomlTableArraySection(FilePathView path, const String& key);
	[[nodiscard]] Optional<PlaceableModelType> ParsePlaceableModelType(const String& value);
   [[nodiscard]] Optional<TerrainCellType> ParseTerrainCellType(const String& value);
	[[nodiscard]] Optional<MainSupport::ResourceType> ParseResourceType(const String& value);
}
