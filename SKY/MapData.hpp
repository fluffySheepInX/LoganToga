# pragma once
# include <Siv3D.hpp>
# include "MainContext.hpp"

enum class PlaceableModelType
{
	Mill,
	Tree,
	Pine,
  Rock,
};

struct PlacedModel
{
	PlaceableModelType type = PlaceableModelType::Tree;
	Vec3 position{ 0, 0, 0 };
  double attackRange = MainSupport::MillDefenseRange;
	double attackDamage = MainSupport::MillDefenseDamage;
	double attackInterval = MainSupport::MillDefenseInterval;
  double suppressionDuration = MainSupport::MillSuppressionDuration;
	double suppressionMoveSpeedMultiplier = MainSupport::MillSuppressionMoveSpeedMultiplier;
	double suppressionAttackDamageMultiplier = MainSupport::MillSuppressionAttackDamageMultiplier;
	double suppressionAttackIntervalMultiplier = MainSupport::MillSuppressionAttackIntervalMultiplier;
};

struct ResourceArea
{
	MainSupport::ResourceType type = MainSupport::ResourceType::Budget;
	Vec3 position{ 0, 0, 0 };
	double radius = MainSupport::ResourceAreaDefaultRadius;
};

struct NavPoint
{
	Vec3 position{ 0, 0, 0 };
	double radius = 1.4;
};

struct NavLink
{
	size_t fromIndex = 0;
	size_t toIndex = 0;
	bool bidirectional = true;
	double costMultiplier = 1.0;
};

struct MapData
{
 Vec3 playerBasePosition{ 8.0, 0.0, 4.0 };
	Vec3 enemyBasePosition{ -2.0, 0.0, 13.0 };
	Vec3 sapperRallyPoint{ 10.5, 0, 10.5 };
    Array<ResourceArea> resourceAreas;
    Array<NavPoint> navPoints;
	Array<NavLink> navLinks;
	Array<PlacedModel> placedModels;
};

[[nodiscard]] MapData MakeDefaultMapData();
struct MapDataLoadResult
{
	MapData mapData;
	String message;
};

[[nodiscard]] MapDataLoadResult LoadMapDataWithStatus(FilePathView path);
[[nodiscard]] MapData LoadMapData(FilePathView path);
bool SaveMapData(const MapData& mapData, FilePathView path);
[[nodiscard]] StringView ToString(PlaceableModelType type);
[[nodiscard]] StringView ToString(MainSupport::ResourceType type);
