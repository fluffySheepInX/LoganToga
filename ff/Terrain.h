# pragma once
# include "GameConstants.h"

namespace ff
{
	TerrainGrid MakeTerrain();
       SpecialTileGrid MakeEmptySpecialTiles();
 SpecialTileGrid MakeOpeningSpecialTiles(const TerrainGrid& terrain, const Point& playerTile);
		SpecialTileGrid MakeRandomSpecialTiles(const TerrainGrid& terrain);
	Array<Point> CollectEnemySpawnTiles(const TerrainGrid& terrain);
	WaterEdgeMask GetWaterEdgeMask(const TerrainGrid& terrain, const Point& index);
}
