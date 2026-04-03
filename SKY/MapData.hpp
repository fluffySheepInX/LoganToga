# pragma once
# include <Siv3D.hpp>

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

struct MapData
{
	Vec3 sapperRallyPoint{ 10.5, 0, 10.5 };
	Array<PlacedModel> placedModels;
};

[[nodiscard]] MapData MakeDefaultMapData();
[[nodiscard]] MapData LoadMapData(FilePathView path);
bool SaveMapData(const MapData& mapData, FilePathView path);
[[nodiscard]] StringView ToString(PlaceableModelType type);
