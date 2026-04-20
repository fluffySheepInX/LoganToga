# include "MapData.hpp"

namespace
{
	[[nodiscard]] ColorF MultiplyColor(const ColorF& baseColor, const ColorF& tintColor)
	{
		return ColorF{
			(baseColor.r * tintColor.r),
			(baseColor.g * tintColor.g),
			(baseColor.b * tintColor.b),
			0.96,
		};
	}

	[[nodiscard]] double SampleTerrainNoise(const Vec2& worldPosition, const double scale)
	{
		const double frequency = Max(0.0001, scale);
		const double coarse = (0.5 + 0.5 * Math::Sin(worldPosition.x * frequency * 0.83 + Math::Cos(worldPosition.y * frequency * 0.57) * 1.9));
		const double detail = (0.5 + 0.5 * Math::Cos((worldPosition.x + worldPosition.y) * frequency * 1.74 + 0.85));
		const double diagonal = (0.5 + 0.5 * Math::Sin((worldPosition.x * 0.62 - worldPosition.y * 1.28) * frequency * 1.37));
		return Clamp((coarse * 0.54 + detail * 0.28 + diagonal * 0.18), 0.0, 1.0);
	}

	[[nodiscard]] ColorF ApplyTerrainNoise(const ColorF& color, const TerrainCell& terrainCell, const MainSupport::TerrainVisualSettings& settings)
	{
		if (not settings.noiseEnabled)
		{
			return color;
		}

		const double strength = Clamp(settings.noiseStrength, 0.0, 1.0);
		const double scale = Clamp(settings.noiseScale, 0.04, 0.60);
		const Vec3 center = ToTerrainCellCenter(terrainCell.cell);
		const Vec2 worldPosition{ center.x, center.z };
		const double primaryNoise = SampleTerrainNoise(worldPosition, scale);
		const double detailNoise = SampleTerrainNoise((worldPosition + Vec2{ 17.3, -11.8 }), (scale * 2.35));
		const double variation = ((primaryNoise - 0.5) * 2.0);
		const double detailVariation = ((detailNoise - 0.5) * 2.0);
		const double brightness = Clamp((1.0 + variation * (0.16 * strength) + detailVariation * (0.07 * strength)), 0.74, 1.26);
		const double warmth = (detailVariation * 0.08 * strength);

		ColorF result = color;
		result.r = Clamp(result.r * brightness * (1.0 + warmth * 0.45), 0.0, 1.0);
		result.g = Clamp(result.g * brightness * (1.0 + ((terrainCell.type == TerrainCellType::Grass) ? variation * 0.06 * strength : -Abs(variation) * 0.03 * strength)), 0.0, 1.0);
		result.b = Clamp(result.b * brightness * (1.0 - warmth * 0.30), 0.0, 1.0);
		result.a = color.a;
		return result;
	}
}

namespace
{
	template <class T, size_t N>
	[[nodiscard]] StringView LookupName(const std::array<std::pair<T, StringView>, N>& table, const T value, const StringView fallback)
	{
		for (const auto& [v, n] : table)
		{
			if (v == value) return n;
		}
		return fallback;
	}
}

StringView ToString(const MainSupport::UnitTeam team)
{
	static constexpr std::array<std::pair<MainSupport::UnitTeam, StringView>, 2> Table{ {
		{ MainSupport::UnitTeam::Player, U"Player" },
		{ MainSupport::UnitTeam::Enemy,  U"Enemy" },
	} };
	return LookupName(Table, team, U"Player");
}

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
	static constexpr std::array<std::pair<PlaceableModelType, StringView>, 8> Table{ {
		{ PlaceableModelType::Mill,           U"Mill" },
		{ PlaceableModelType::Tree,           U"Tree" },
		{ PlaceableModelType::Pine,           U"Pine" },
		{ PlaceableModelType::GrassPatch,     U"GrassPatch" },
		{ PlaceableModelType::Rock,           U"Rock" },
		{ PlaceableModelType::Wall,           U"Wall" },
		{ PlaceableModelType::Road,           U"Road" },
		{ PlaceableModelType::TireTrackDecal, U"TireTrackDecal" },
	} };
	return LookupName(Table, type, U"Tree");
}

bool SupportsMillDefenseParameters(const PlaceableModelType type)
{
	return (type == PlaceableModelType::Mill);
}

MillDefenseParameters GetMillDefenseParameters(const MapData& mapData)
{
	return mapData.millParameters;
}

StringView ToString(const TerrainCellType type)
{
	static constexpr std::array<std::pair<TerrainCellType, StringView>, 4> Table{ {
		{ TerrainCellType::Grass, U"Grass" },
		{ TerrainCellType::Dirt,  U"Dirt" },
		{ TerrainCellType::Sand,  U"Sand" },
		{ TerrainCellType::Rock,  U"Rock" },
	} };
	return LookupName(Table, type, U"Grass");
}

StringView ToString(const MainSupport::ResourceType type)
{
	static constexpr std::array<std::pair<MainSupport::ResourceType, StringView>, 3> Table{ {
		{ MainSupport::ResourceType::Budget,    U"Budget" },
		{ MainSupport::ResourceType::Gunpowder, U"Gunpowder" },
		{ MainSupport::ResourceType::Mana,      U"Mana" },
	} };
	return LookupName(Table, type, U"Budget");
}

Optional<size_t> FindTerrainCellIndex(const Array<TerrainCell>& terrainCells, const Point& cell)
{
	for (size_t i = 0; i < terrainCells.size(); ++i)
	{
		if (terrainCells[i].cell == cell)
		{
			return i;
		}
	}

	return none;
}

Point ToTerrainCell(const Vec3& position)
{
	return Point{
     static_cast<int32>(Floor(position.x / TerrainCellSize)),
		static_cast<int32>(Floor(position.z / TerrainCellSize)),
	};
}

Vec3 ToTerrainCellCenter(const Point& cell)
{
	return Vec3{
		((static_cast<double>(cell.x) + 0.5) * TerrainCellSize),
		0.0,
		((static_cast<double>(cell.y) + 0.5) * TerrainCellSize),
	};
}

ColorF GetTerrainCellBaseColor(const TerrainCellType type)
{
	static const std::array<std::pair<TerrainCellType, ColorF>, 4> Table{ {
		{ TerrainCellType::Grass, ColorF{ 0.42, 0.72, 0.34 } },
		{ TerrainCellType::Dirt,  ColorF{ 0.56, 0.38, 0.22 } },
		{ TerrainCellType::Sand,  ColorF{ 0.84, 0.76, 0.46 } },
		{ TerrainCellType::Rock,  ColorF{ 0.52, 0.54, 0.58 } },
	} };
	for (const auto& [t, c] : Table)
	{
		if (t == type) return c;
	}
	return ColorF{ 0.42, 0.72, 0.34 };
}

ColorF GetTerrainCellDrawColor(const TerrainCell& terrainCell)
{
	return MultiplyColor(GetTerrainCellBaseColor(terrainCell.type), ColorF{ terrainCell.color });
}

ColorF GetTerrainCellDrawColor(const TerrainCell& terrainCell, const MainSupport::TerrainVisualSettings& settings)
{
	return ApplyTerrainNoise(GetTerrainCellDrawColor(terrainCell), terrainCell, settings);
}

void SetTerrainCell(Array<TerrainCell>& terrainCells, const Point& cell, const TerrainCellType type, const Color& color)
{
	if (const auto index = FindTerrainCellIndex(terrainCells, cell))
	{
		terrainCells[*index].type = type;
		terrainCells[*index].color = color;
		return;
	}

	terrainCells << TerrainCell{
		.cell = cell,
		.type = type,
		.color = color,
	};
}

bool RemoveTerrainCell(Array<TerrainCell>& terrainCells, const Point& cell)
{
	if (const auto index = FindTerrainCellIndex(terrainCells, cell))
	{
		terrainCells.erase(terrainCells.begin() + *index);
		return true;
	}

	return false;
}
