# pragma once
# include <Siv3D.hpp>
# include "MapData.hpp"

struct TerrainSurfaceCell
{
	Point cell{ 0, 0 };
	TerrainCellType dominantType = TerrainCellType::Dirt;
	double wear = 0.0;
	double ambientOcclusion = 0.0;
	ColorF finalColor{ 1.0, 1.0, 1.0, 0.0 };
};

struct TerrainSurfaceData
{
	Array<TerrainSurfaceCell> cells;
};

[[nodiscard]] uint64 ComputeTerrainSurfaceRevision(const MapData& mapData, const MainSupport::TerrainVisualSettings& settings);
[[nodiscard]] TerrainSurfaceData BuildTerrainSurface(const MapData& mapData, const MainSupport::TerrainVisualSettings& settings);
