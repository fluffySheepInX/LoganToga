# include "FogOfWar.hpp"
# include "SkyAppSupport.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		[[nodiscard]] int64 ToTerrainCellKey(const Point& cell)
		{
			return (static_cast<int64>(cell.x) << 32)
				^ static_cast<uint32>(cell.y);
		}

		void ExpandBounds(Point& minCell, Point& maxCell, const Vec3& position, const double radius = 0.0)
		{
			const Point minPositionCell = ToTerrainCell(position.movedBy(-radius, 0, -radius));
			const Point maxPositionCell = ToTerrainCell(position.movedBy(radius, 0, radius));
			minCell.x = Min(minCell.x, minPositionCell.x);
			minCell.y = Min(minCell.y, minPositionCell.y);
			maxCell.x = Max(maxCell.x, maxPositionCell.x);
			maxCell.y = Max(maxCell.y, maxPositionCell.y);
		}

		[[nodiscard]] Rect BuildCoverageBounds(const FogOfWarSettings& settings,
			const MapData& mapData,
			const Array<SpawnedSapper>& spawnedSappers)
		{
			Point minCell = ToTerrainCell(mapData.playerBasePosition);
			Point maxCell = minCell;
			ExpandBounds(minCell, maxCell, mapData.enemyBasePosition);
			ExpandBounds(minCell, maxCell, mapData.sapperRallyPoint);
			ExpandBounds(minCell, maxCell, mapData.playerBasePosition, settings.baseVisionRange);
			ExpandBounds(minCell, maxCell, mapData.enemyBasePosition, settings.baseVisionRange);

			for (const auto& terrainCell : mapData.terrainCells)
			{
				minCell.x = Min(minCell.x, terrainCell.cell.x);
				minCell.y = Min(minCell.y, terrainCell.cell.y);
				maxCell.x = Max(maxCell.x, terrainCell.cell.x);
				maxCell.y = Max(maxCell.y, terrainCell.cell.y);
			}

			for (const auto& placedModel : mapData.placedModels)
			{
				double radius = 0.0;
				switch (placedModel.type)
				{
				case PlaceableModelType::Wall:
					radius = (placedModel.wallLength * 0.5);
					break;
				case PlaceableModelType::Road:
				case PlaceableModelType::TireTrackDecal:
					radius = (placedModel.roadLength * 0.5);
					break;
				default:
					radius = 1.0;
					break;
				}

				ExpandBounds(minCell, maxCell, placedModel.position, radius);
			}

			for (const auto& area : mapData.resourceAreas)
			{
				ExpandBounds(minCell, maxCell, area.position, area.radius + settings.ownedResourceVisionRange);
			}

			for (const auto& sapper : spawnedSappers)
			{
				if (sapper.hitPoints <= 0.0)
				{
					continue;
				}

				ExpandBounds(minCell, maxCell, GetSpawnedSapperBasePosition(sapper), Max(settings.defaultUnitVisionRange, sapper.visionRange));
			}

			const int32 padding = Max(1, settings.mapPaddingCells);
			minCell.x -= padding;
			minCell.y -= padding;
			maxCell.x += padding;
			maxCell.y += padding;
			return Rect{ minCell.x, minCell.y, Max(1, maxCell.x - minCell.x + 1), Max(1, maxCell.y - minCell.y + 1) };
		}

		void StampVisibility(FogOfWarState& fogOfWar, const Vec3& center, const double radius)
		{
			if (radius <= 0.0)
			{
				return;
			}

			const Point minCell = ToTerrainCell(center.movedBy(-radius, 0, -radius));
			const Point maxCell = ToTerrainCell(center.movedBy(radius, 0, radius));
			const double radiusSq = Square(radius);

			for (int32 y = minCell.y; y <= maxCell.y; ++y)
			{
				for (int32 x = minCell.x; x <= maxCell.x; ++x)
				{
					const Point cell{ x, y };
					const Vec3 cellCenter = ToTerrainCellCenter(cell);
					const double distanceSq = Square(cellCenter.x - center.x) + Square(cellCenter.z - center.z);
					if (radiusSq < distanceSq)
					{
						continue;
					}

					FogOfWarCellState& cellState = fogOfWar.cells[ToTerrainCellKey(cell)];
					cellState.explored = true;
					cellState.visibleStrength = Max(cellState.visibleStrength, 1.0);
				}
			}
		}
	}

	void ResetFogOfWar(FogOfWarState& fogOfWar)
	{
		fogOfWar.cells.clear();
		fogOfWar.coverageBounds = Rect{ 0, 0, 1, 1 };
		fogOfWar.revision = 0;
		fogOfWar.enabled = true;
	}

	void UpdateFogOfWar(FogOfWarState& fogOfWar,
		const FogOfWarSettings& settings,
		const MapData& mapData,
		const Array<SpawnedSapper>& spawnedSappers,
		const Array<ResourceAreaState>& resourceAreaStates)
	{
		fogOfWar.enabled = settings.enabled;
		fogOfWar.coverageBounds = BuildCoverageBounds(settings, mapData, spawnedSappers);

		if (not settings.enabled)
		{
			fogOfWar.cells.clear();
			++fogOfWar.revision;
			return;
		}

		for (auto& [key, cellState] : fogOfWar.cells)
		{
			(void)key;
			cellState.visibleStrength = 0.0;
		}

		StampVisibility(fogOfWar, mapData.playerBasePosition, settings.baseVisionRange);

		for (const auto& sapper : spawnedSappers)
		{
			if ((sapper.hitPoints <= 0.0) || IsSapperRetreatedHidden(sapper))
			{
				continue;
			}

			StampVisibility(fogOfWar,
				GetSpawnedSapperBasePosition(sapper),
				Max(0.5, (sapper.visionRange > 0.0) ? sapper.visionRange : settings.defaultUnitVisionRange));
		}

		for (size_t i = 0; i < mapData.resourceAreas.size(); ++i)
		{
			if ((i >= resourceAreaStates.size())
				|| (not resourceAreaStates[i].ownerTeam)
				|| (*resourceAreaStates[i].ownerTeam != UnitTeam::Player))
			{
				continue;
			}

			StampVisibility(fogOfWar,
				mapData.resourceAreas[i].position,
				Max(settings.ownedResourceVisionRange, mapData.resourceAreas[i].radius + 1.0));
		}

		for (const auto& placedModel : mapData.placedModels)
		{
			if (placedModel.type != PlaceableModelType::Mill)
			{
				continue;
			}

			StampVisibility(fogOfWar,
				placedModel.position,
				Max(settings.millVisionRange, placedModel.attackRange + 1.0));
		}

      for (auto it = fogOfWar.cells.begin(); it != fogOfWar.cells.end();)
		{
			if ((not it->second.explored) && (it->second.visibleStrength <= 0.0))
			{
				it = fogOfWar.cells.erase(it);
				continue;
			}

			++it;
		}
		++fogOfWar.revision;
	}

	FogVisibility GetFogVisibilityAt(const FogOfWarState& fogOfWar, const Point& cell)
	{
		if (not fogOfWar.enabled)
		{
			return FogVisibility::Visible;
		}

		const auto it = fogOfWar.cells.find(ToTerrainCellKey(cell));
		if (it == fogOfWar.cells.end())
		{
			return FogVisibility::Hidden;
		}

		if (0.01 < it->second.visibleStrength)
		{
			return FogVisibility::Visible;
		}

		return it->second.explored ? FogVisibility::Explored : FogVisibility::Hidden;
	}

	FogVisibility GetFogVisibilityAt(const FogOfWarState& fogOfWar, const Vec3& position)
	{
		return GetFogVisibilityAt(fogOfWar, ToTerrainCell(position));
	}

	bool IsFogVisibleAt(const FogOfWarState& fogOfWar, const Point& cell)
	{
		return (GetFogVisibilityAt(fogOfWar, cell) == FogVisibility::Visible);
	}

	bool IsFogVisibleAt(const FogOfWarState& fogOfWar, const Vec3& position)
	{
		return IsFogVisibleAt(fogOfWar, ToTerrainCell(position));
	}

	bool IsFogExploredAt(const FogOfWarState& fogOfWar, const Point& cell)
	{
		return (GetFogVisibilityAt(fogOfWar, cell) != FogVisibility::Hidden);
	}

	bool IsFogExploredAt(const FogOfWarState& fogOfWar, const Vec3& position)
	{
		return IsFogExploredAt(fogOfWar, ToTerrainCell(position));
	}
}
