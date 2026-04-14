# pragma once
# include <Siv3D.hpp>
# include "MainContext.hpp"
# include "MainContextTypes.hpp"

enum class PlaceableModelType
{
	Mill,
	Tree,
	Pine,
  GrassPatch,
 Rock,
	Wall,
  Road,
  TireTrackDecal,
};

enum class TerrainCellType
{
	Grass,
	Dirt,
	Sand,
	Rock,
};

inline constexpr double TerrainCellSize = 2.0;

struct PlacedModel
{
	PlaceableModelType type = PlaceableModelType::Tree;
	Vec3 position{ 0, 0, 0 };
   double yaw = 0.0;
	double wallLength = 10.0;
   double roadLength = 8.0;
	double roadWidth = 4.0;
  double attackRange = MainSupport::MillDefenseRange;
	double attackDamage = MainSupport::MillDefenseDamage;
	double attackInterval = MainSupport::MillDefenseInterval;
    int32 attackTargetCount = MainSupport::MillDefenseTargetCount;
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

struct TerrainCell
{
	Point cell{ 0, 0 };
	TerrainCellType type = TerrainCellType::Grass;
	Color color{ 255, 255, 255 };
};

struct MapData
{
 Vec3 playerBasePosition{ 8.0, 0.0, 4.0 };
	Vec3 enemyBasePosition{ -2.0, 0.0, 13.0 };
	Vec3 sapperRallyPoint{ 10.5, 0, 10.5 };
    Array<ResourceArea> resourceAreas;
    Array<NavPoint> navPoints;
	Array<NavLink> navLinks;
    Array<TerrainCell> terrainCells;
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
[[nodiscard]] StringView ToString(TerrainCellType type);
[[nodiscard]] StringView ToString(MainSupport::ResourceType type);
[[nodiscard]] Optional<size_t> FindTerrainCellIndex(const Array<TerrainCell>& terrainCells, const Point& cell);
[[nodiscard]] Point ToTerrainCell(const Vec3& position);
[[nodiscard]] Vec3 ToTerrainCellCenter(const Point& cell);
[[nodiscard]] ColorF GetTerrainCellBaseColor(TerrainCellType type);
[[nodiscard]] ColorF GetTerrainCellDrawColor(const TerrainCell& terrainCell);
[[nodiscard]] ColorF GetTerrainCellDrawColor(const TerrainCell& terrainCell, const MainSupport::TerrainVisualSettings& settings);
void SetTerrainCell(Array<TerrainCell>& terrainCells, const Point& cell, TerrainCellType type, const Color& color);
bool RemoveTerrainCell(Array<TerrainCell>& terrainCells, const Point& cell);
