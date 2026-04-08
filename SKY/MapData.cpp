# include "MapData.hpp"

MapData MakeDefaultMapData()
{
	MapData mapData;
    mapData.playerBasePosition = Vec3{ 8.0, 0.0, 4.0 };
	mapData.enemyBasePosition = Vec3{ -2.0, 0.0, 13.0 };
	mapData.sapperRallyPoint = Vec3{ 10.5, 0.0, 10.5 };
    mapData.resourceAreas = {
		ResourceArea{ .type = MainSupport::ResourceType::Budget, .position = Vec3{ 5.5, 0.0, 8.0 }, .radius = 4.8 },
		ResourceArea{ .type = MainSupport::ResourceType::Gunpowder, .position = Vec3{ -6.5, 0.0, 9.0 }, .radius = 4.8 },
		ResourceArea{ .type = MainSupport::ResourceType::Mana, .position = Vec3{ 1.0, 0.0, -2.0 }, .radius = 4.8 },
	};
    mapData.navPoints = {
		NavPoint{ .position = mapData.playerBasePosition, .radius = 1.8 },
		NavPoint{ .position = Vec3{ 12.0, 0.0, 4.5 }, .radius = 1.6 },
		NavPoint{ .position = Vec3{ 12.0, 0.0, 10.5 }, .radius = 1.6 },
		NavPoint{ .position = Vec3{ 5.0, 0.0, 13.0 }, .radius = 1.6 },
		NavPoint{ .position = mapData.enemyBasePosition, .radius = 1.8 },
	};
	mapData.navLinks = {
		NavLink{ .fromIndex = 0, .toIndex = 1 },
		NavLink{ .fromIndex = 1, .toIndex = 2 },
		NavLink{ .fromIndex = 2, .toIndex = 3 },
		NavLink{ .fromIndex = 3, .toIndex = 4 },
	};
	mapData.placedModels = {
      PlacedModel{ .type = PlaceableModelType::Mill, .position = Vec3{ 4.5, 0, 6.0 } },
		PlacedModel{ .type = PlaceableModelType::Mill, .position = Vec3{ 11.5, 0, 6.0 } },
		PlacedModel{ .type = PlaceableModelType::Tree, .position = Vec3{ 16, 0, 4 } },
		PlacedModel{ .type = PlaceableModelType::Pine, .position = Vec3{ 16, 0, 0 } },
	};
	return mapData;
}


StringView ToString(const PlaceableModelType type)
{
	switch (type)
	{
	case PlaceableModelType::Mill:
		return U"Mill";

	case PlaceableModelType::Tree:
		return U"Tree";

	case PlaceableModelType::Pine:
		return U"Pine";

	case PlaceableModelType::Rock:
		return U"Rock";

	default:
		return U"Tree";
	}
}

StringView ToString(const MainSupport::ResourceType type)
{
	switch (type)
	{
	case MainSupport::ResourceType::Budget:
		return U"Budget";

	case MainSupport::ResourceType::Gunpowder:
		return U"Gunpowder";

	case MainSupport::ResourceType::Mana:
		return U"Mana";

	default:
		return U"Budget";
	}
}
