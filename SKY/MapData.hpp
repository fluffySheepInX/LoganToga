# pragma once
# include <Siv3D.hpp>
# include "MainContext.hpp"

enum class PlaceableModelType
{
	Mill,
	Tree,
	Pine,
};

struct PlacedModel
{
	PlaceableModelType type = PlaceableModelType::Tree;
	Vec3 position{ 0, 0, 0 };
};

struct ResourceArea
{
	MainSupport::ResourceType type = MainSupport::ResourceType::Budget;
	Vec3 position{ 0, 0, 0 };
	double radius = MainSupport::ResourceAreaDefaultRadius;
};

struct MapData
{
 Vec3 playerBasePosition{ 8.0, 0.0, 4.0 };
	Vec3 enemyBasePosition{ -2.0, 0.0, 13.0 };
	Vec3 sapperRallyPoint{ 10.5, 0, 10.5 };
    Array<ResourceArea> resourceAreas;
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
