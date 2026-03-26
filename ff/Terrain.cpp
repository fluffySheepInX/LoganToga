# include "Terrain.h"

namespace ff
{
	namespace
	{
        bool CanPlaceSpecialArea(const TerrainGrid& terrain, const SpecialTileGrid& specialTiles, const Point& origin)
		{
			for (int32 y = 0; y < 2; ++y)
			{
				for (int32 x = 0; x < 2; ++x)
				{
					const Point tile{ (origin.x + x), (origin.y + y) };

					if ((not InRange(tile.x, 0, (MapSize.x - 1)))
						|| (not InRange(tile.y, 0, (MapSize.y - 1)))
						|| IsWaterTile(terrain[tile]))
					{
						return false;
					}

					if (specialTiles[tile] != SpecialTileKind::None)
					{
						return false;
					}
				}
			}

			return true;
		}

		void PlaceSpecialArea(SpecialTileGrid& specialTiles, const Point& origin, const SpecialTileKind tileKind)
		{
			for (int32 y = 0; y < 2; ++y)
			{
				for (int32 x = 0; x < 2; ++x)
				{
                  specialTiles[Point{ (origin.x + x), (origin.y + y) }] = tileKind;
				}
			}
		}

		Array<Point> CollectSpecialAreaCandidates(const TerrainGrid& terrain, const SpecialTileGrid& specialTiles)
		{
			Array<Point> candidates;

			for (int32 y = 0; y < (MapSize.y - 1); ++y)
			{
				for (int32 x = 0; x < (MapSize.x - 1); ++x)
				{
					const Point origin{ x, y };

					if (CanPlaceSpecialArea(terrain, specialTiles, origin))
					{
						candidates << origin;
					}
				}
			}

			return candidates;
		}

		bool TryPlaceSpecialArea(const TerrainGrid& terrain, SpecialTileGrid& specialTiles, const Array<Point>& candidates, const SpecialTileKind tileKind)
		{
			for (const auto& origin : candidates)
			{
				if (CanPlaceSpecialArea(terrain, specialTiles, origin))
				{
					PlaceSpecialArea(specialTiles, origin, tileKind);
					return true;
				}
			}

			return false;
		}

		bool HasLandNeighbor(const TerrainGrid& terrain, const Point& index, const Point& offset)
		{
			const Point neighbor{ (index.x + offset.x), (index.y + offset.y) };

			if ((not InRange(neighbor.x, 0, (MapSize.x - 1)))
				|| (not InRange(neighbor.y, 0, (MapSize.y - 1))))
			{
				return false;
			}

			return IsLandTile(terrain[neighbor]);
		}
	}

    SpecialTileGrid MakeEmptySpecialTiles()
	{
		SpecialTileGrid specialTiles{ MapSize };

		for (int32 y = 0; y < MapSize.y; ++y)
		{
			for (int32 x = 0; x < MapSize.x; ++x)
			{
				specialTiles[Point{ x, y }] = SpecialTileKind::None;
			}
		}

		return specialTiles;
	}

	SpecialTileGrid MakeRandomSpecialTiles(const TerrainGrid& terrain)
	{
		SpecialTileGrid specialTiles = MakeEmptySpecialTiles();
        Array<Point> candidates = CollectSpecialAreaCandidates(terrain, specialTiles);

		candidates.shuffle();
		TryPlaceSpecialArea(terrain, specialTiles, candidates, SpecialTileKind::Bonus);

       candidates = CollectSpecialAreaCandidates(terrain, specialTiles);
		candidates.shuffle();
		TryPlaceSpecialArea(terrain, specialTiles, candidates, SpecialTileKind::Penalty);

		return specialTiles;
	}

	SpecialTileGrid MakeOpeningSpecialTiles(const TerrainGrid& terrain, const Point& playerTile)
	{
		SpecialTileGrid specialTiles = MakeEmptySpecialTiles();
		const Array<Point> preferredBonusOrigins = {
			(playerTile + Point{ 1, -1 }),
			(playerTile + Point{ -2, -1 }),
			(playerTile + Point{ -1, 1 }),
			(playerTile + Point{ -1, -2 }),
			(playerTile + Point{ 2, -1 }),
			(playerTile + Point{ -3, -1 }),
			(playerTile + Point{ -1, 2 }),
			(playerTile + Point{ -1, -3 }),
			(playerTile + Point{ -1, -1 }),
			(playerTile + Point{ 0, -1 }),
			(playerTile + Point{ -1, 0 }),
			(playerTile + Point{ 0, 0 }),
		};

		if (not TryPlaceSpecialArea(terrain, specialTiles, preferredBonusOrigins, SpecialTileKind::Bonus))
		{
			Array<Point> fallbackBonusOrigins = CollectSpecialAreaCandidates(terrain, specialTiles);
			fallbackBonusOrigins.shuffle();
			TryPlaceSpecialArea(terrain, specialTiles, fallbackBonusOrigins, SpecialTileKind::Bonus);
		}

		Array<Point> penaltyOrigins = CollectSpecialAreaCandidates(terrain, specialTiles);
		penaltyOrigins.shuffle();
		TryPlaceSpecialArea(terrain, specialTiles, penaltyOrigins, SpecialTileKind::Penalty);

		return specialTiles;
	}

	TerrainGrid MakeTerrain()
	{
		TerrainGrid terrain{ MapSize };
		const Vec2 center{ ((MapSize.x - 1) * 0.5), ((MapSize.y - 1) * 0.5) };

		for (int32 y = 0; y < MapSize.y; ++y)
		{
			for (int32 x = 0; x < MapSize.x; ++x)
			{
				const Vec2 pos{ static_cast<double>(x), static_cast<double>(y) };
				const double distance = pos.distanceFrom(center);
				const double shape = (Sin((x * 0.47) + 0.4) + Cos((y * 0.43) - 0.8) + Sin((x + y) * 0.19));

				TerrainTile tileType = TerrainTile::Grass;

				if (distance > (11.5 + (shape * 0.55)))
				{
					tileType = TerrainTile::Water;
				}
				else if (distance > (10.0 + (shape * 0.4)))
				{
					tileType = TerrainTile::Sand;
				}
				else if ((Abs(x - y) <= 1) || (Abs((x + y) - 19) <= 1))
				{
					tileType = TerrainTile::Path;
				}
				else if ((((x * 3) + (y * 5)) % 17) == 0)
				{
					tileType = TerrainTile::Path;
				}

				terrain[Point{ x, y }] = tileType;
			}
		}

		terrain[Point{ 12, 12 }] = TerrainTile::Grass;
		terrain[Point{ 12, 11 }] = TerrainTile::Grass;
		terrain[Point{ 11, 12 }] = TerrainTile::Grass;
		return terrain;
	}

	Array<Point> CollectEnemySpawnTiles(const TerrainGrid& terrain)
	{
		Array<Point> spawnTiles;
		const Vec2 center{ ((MapSize.x - 1) * 0.5), ((MapSize.y - 1) * 0.5) };

		for (int32 y = 0; y < MapSize.y; ++y)
		{
			for (int32 x = 0; x < MapSize.x; ++x)
			{
				const Point tileIndex{ x, y };

				if (IsWaterTile(terrain[tileIndex]))
				{
					continue;
				}

				const double distance = Vec2{ static_cast<double>(x), static_cast<double>(y) }.distanceFrom(center);

				if (distance >= 8.8)
				{
					spawnTiles << tileIndex;
				}
			}
		}

		return spawnTiles;
	}

	WaterEdgeMask GetWaterEdgeMask(const TerrainGrid& terrain, const Point& index)
	{
		if (not IsWaterTile(terrain[index]))
		{
			return{};
		}

		WaterEdgeMask mask;
		mask.upperRight = HasLandNeighbor(terrain, index, Point{ 0, -1 });
		mask.lowerRight = HasLandNeighbor(terrain, index, Point{ 1, 0 });
		mask.lowerLeft = HasLandNeighbor(terrain, index, Point{ 0, 1 });
		mask.upperLeft = HasLandNeighbor(terrain, index, Point{ -1, 0 });
		mask.topCornerOnly = (HasLandNeighbor(terrain, index, Point{ -1, -1 })
			&& (not mask.upperLeft)
			&& (not mask.upperRight));
		mask.bottomCornerOnly = (HasLandNeighbor(terrain, index, Point{ 1, 1 })
			&& (not mask.lowerRight)
			&& (not mask.lowerLeft));
		mask.rightCornerOnly = (HasLandNeighbor(terrain, index, Point{ 1, -1 })
			&& (not mask.upperRight)
			&& (not mask.lowerRight));
		mask.leftCornerOnly = (HasLandNeighbor(terrain, index, Point{ -1, 1 })
			&& (not mask.lowerLeft)
			&& (not mask.upperLeft));
		return mask;
	}
}
