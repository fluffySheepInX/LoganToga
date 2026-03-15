#include "BattleSession.h"
#include "BattleSessionInternal.h"

void BattleSession::invalidateUnitIndex() noexcept
{
	m_unitIndexDirty = true;
	m_frameUnitCacheDirty = true;
	m_spatialQueryCacheDirty = true;
}

void BattleSession::invalidateBuildingIndex() noexcept
{
	m_buildingIndexDirty = true;
}

void BattleSession::invalidateSpatialQueryCache() noexcept
{
	m_spatialQueryCacheDirty = true;
}

void BattleSession::invalidateNavigationGrid() noexcept
{
	m_navigationGridDirty = true;
}

void BattleSession::rebuildUnitIndex() const
{
	if (!m_unitIndexDirty)
	{
		return;
	}

	m_unitIndexById.clear();
	m_unitIndexById.reserve(m_state.units.size());
	for (size_t index = 0; index < m_state.units.size(); ++index)
	{
		m_unitIndexById.emplace(m_state.units[index].id, index);
	}

	m_unitIndexDirty = false;
}

void BattleSession::rebuildBuildingIndex() const
{
	if (!m_buildingIndexDirty)
	{
		return;
	}

	m_buildingIndexByUnitId.clear();
	m_buildingIndexByUnitId.reserve(m_state.buildings.size());
	for (size_t index = 0; index < m_state.buildings.size(); ++index)
	{
		m_buildingIndexByUnitId.emplace(m_state.buildings[index].unitId, index);
	}

	m_buildingIndexDirty = false;
}

void BattleSession::rebuildFrameUnitCache() const
{
	if (!m_frameUnitCacheDirty)
	{
		return;
	}

	m_playerUnitIndices.clear();
	m_enemyUnitIndices.clear();
	m_playerBuildingIndices.clear();
	m_enemyBuildingIndices.clear();
	m_playerUnitIndices.reserve(m_state.units.size());
	m_enemyUnitIndices.reserve(m_state.units.size());
	m_playerBuildingIndices.reserve(m_state.buildings.size());
	m_enemyBuildingIndices.reserve(m_state.buildings.size());

	for (size_t index = 0; index < m_state.units.size(); ++index)
	{
		const auto& unit = m_state.units[index];
		Array<size_t>* ownerIndices = nullptr;
		Array<size_t>* ownerBuildingIndices = nullptr;

		if (unit.owner == Owner::Player)
		{
			ownerIndices = &m_playerUnitIndices;
			ownerBuildingIndices = &m_playerBuildingIndices;
		}
		else if (unit.owner == Owner::Enemy)
		{
			ownerIndices = &m_enemyUnitIndices;
			ownerBuildingIndices = &m_enemyBuildingIndices;
		}

		if (!ownerIndices)
		{
			continue;
		}

		ownerIndices->push_back(index);
		if (IsBuildingArchetype(unit.archetype))
		{
			ownerBuildingIndices->push_back(index);
		}
	}

	m_frameUnitCacheDirty = false;
}

void BattleSession::rebuildSpatialQueryCache() const
{
	if (!m_spatialQueryCacheDirty)
	{
		return;
	}

	m_spatialQueryBounds = m_state.worldBounds;
	m_spatialQueryColumns = Max(1, static_cast<int32>(std::ceil(m_spatialQueryBounds.w / m_spatialQueryCellSize)));
	m_spatialQueryRows = Max(1, static_cast<int32>(std::ceil(m_spatialQueryBounds.h / m_spatialQueryCellSize)));

	const int32 cellCount = (m_spatialQueryColumns * m_spatialQueryRows);
	m_playerSpatialUnitIndices.clear();
	m_enemySpatialUnitIndices.clear();
	m_playerSpatialUnitIndices.resize(cellCount);
	m_enemySpatialUnitIndices.resize(cellCount);

	for (size_t index = 0; index < m_state.units.size(); ++index)
	{
		const auto& unit = m_state.units[index];
		if (!unit.isAlive)
		{
			continue;
		}

		const int32 cellX = Clamp(static_cast<int32>((unit.position.x - m_spatialQueryBounds.leftX()) / m_spatialQueryCellSize), 0, m_spatialQueryColumns - 1);
		const int32 cellY = Clamp(static_cast<int32>((unit.position.y - m_spatialQueryBounds.topY()) / m_spatialQueryCellSize), 0, m_spatialQueryRows - 1);
		const int32 cellIndex = (cellY * m_spatialQueryColumns) + cellX;

		if (unit.owner == Owner::Player)
		{
			m_playerSpatialUnitIndices[cellIndex] << index;
		}
		else if (unit.owner == Owner::Enemy)
		{
			m_enemySpatialUnitIndices[cellIndex] << index;
		}
	}

	m_spatialQueryCacheDirty = false;
}

void BattleSession::rebuildNavigationGrid() const
{
	if (!m_navigationGridDirty)
	{
		return;
	}

	double maxMoverRadius = 12.0;
	for (const auto& definition : m_config.unitDefinitions)
	{
		if (definition.canMove)
		{
			maxMoverRadius = Max(maxMoverRadius, definition.radius);
		}
	}

	m_navigationGridBounds = m_state.worldBounds;
	m_navigationGridCellSize = Max(maxMoverRadius * 2.0, 24.0);
	m_navigationGridColumns = Max(1, static_cast<int32>(std::ceil(m_navigationGridBounds.w / m_navigationGridCellSize)));
	m_navigationGridRows = Max(1, static_cast<int32>(std::ceil(m_navigationGridBounds.h / m_navigationGridCellSize)));
	m_navigationGridBlocked.resize(m_navigationGridColumns * m_navigationGridRows);

	const double clearanceRadius = (maxMoverRadius + 2.0);
	for (int32 y = 0; y < m_navigationGridRows; ++y)
	{
		for (int32 x = 0; x < m_navigationGridColumns; ++x)
		{
			const int32 index = (y * m_navigationGridColumns) + x;
			const Vec2 center{
				Min(m_navigationGridBounds.leftX() + ((x + 0.5) * m_navigationGridCellSize), m_navigationGridBounds.rightX() - (m_navigationGridCellSize * 0.5)),
				Min(m_navigationGridBounds.topY() + ((y + 0.5) * m_navigationGridCellSize), m_navigationGridBounds.bottomY() - (m_navigationGridCellSize * 0.5))
			};

			bool blocked = BattleSessionInternal::IsBlockedByObstacle(center, clearanceRadius, m_config.obstacles);
			if (!blocked)
			{
				for (const auto& unit : m_state.units)
				{
					if (!unit.isAlive || !IsBuildingArchetype(unit.archetype))
					{
						continue;
					}

					if (BattleSessionInternal::IntersectsBuilding(center, clearanceRadius, unit))
					{
						blocked = true;
						break;
					}
				}
			}

			m_navigationGridBlocked[index] = blocked ? 1 : 0;
		}
	}

	m_navigationGridDirty = false;
}

void BattleSession::gatherNearbyUnitIndices(const Owner owner, const Vec2& center, const double searchRadius, Array<size_t>& indices) const
{
	rebuildSpatialQueryCache();
	indices.clear();

	const auto& spatialCells = (owner == Owner::Enemy)
		? m_enemySpatialUnitIndices
		: m_playerSpatialUnitIndices;

	const double clampedRadius = Max(searchRadius, 0.0);
	const int32 minCellX = Clamp(static_cast<int32>(((center.x - clampedRadius) - m_spatialQueryBounds.leftX()) / m_spatialQueryCellSize), 0, m_spatialQueryColumns - 1);
	const int32 maxCellX = Clamp(static_cast<int32>(((center.x + clampedRadius) - m_spatialQueryBounds.leftX()) / m_spatialQueryCellSize), 0, m_spatialQueryColumns - 1);
	const int32 minCellY = Clamp(static_cast<int32>(((center.y - clampedRadius) - m_spatialQueryBounds.topY()) / m_spatialQueryCellSize), 0, m_spatialQueryRows - 1);
	const int32 maxCellY = Clamp(static_cast<int32>(((center.y + clampedRadius) - m_spatialQueryBounds.topY()) / m_spatialQueryCellSize), 0, m_spatialQueryRows - 1);

	for (int32 y = minCellY; y <= maxCellY; ++y)
	{
		for (int32 x = minCellX; x <= maxCellX; ++x)
		{
			const auto& cellIndices = spatialCells[(y * m_spatialQueryColumns) + x];
			for (const auto index : cellIndices)
			{
				indices << index;
			}
		}
	}
}

void BattleSession::gatherNearbyOpponentIndices(const UnitState& source, const double searchRadius, Array<size_t>& indices) const
{
	const Owner opponentOwner = (source.owner == Owner::Enemy) ? Owner::Player : Owner::Enemy;
	gatherNearbyUnitIndices(opponentOwner, source.position, searchRadius, indices);
}

const Array<size_t>& BattleSession::getOwnerUnitIndices(const Owner owner) const
{
	rebuildFrameUnitCache();
	return (owner == Owner::Enemy) ? m_enemyUnitIndices : m_playerUnitIndices;
}

const Array<size_t>& BattleSession::getOwnerBuildingIndices(const Owner owner) const
{
	rebuildFrameUnitCache();
	return (owner == Owner::Enemy) ? m_enemyBuildingIndices : m_playerBuildingIndices;
}

const UnitState* BattleSession::findOwnerUnitByArchetype(const Owner owner, const UnitArchetype archetype) const
{
	const auto& candidateIndices = IsBuildingArchetype(archetype)
		? getOwnerBuildingIndices(owner)
		: getOwnerUnitIndices(owner);

	for (const auto index : candidateIndices)
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive && (unit.owner == owner) && (unit.archetype == archetype))
		{
			return &unit;
		}
	}

	return nullptr;
}

bool BattleSession::hasBaseDefenseTurret(const UnitState& base, const double lockRadius) const
{
	for (const auto index : getOwnerBuildingIndices(base.owner))
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive
			&& (unit.archetype == UnitArchetype::Turret)
			&& (unit.position.distanceFrom(base.position) <= lockRadius))
		{
			return true;
		}
	}

	return false;
}

UnitState* BattleSession::findCachedUnit(const int32 id)
{
	rebuildUnitIndex();
	const auto it = m_unitIndexById.find(id);
	if ((it == m_unitIndexById.end()) || (it->second >= m_state.units.size()))
	{
		return nullptr;
	}

	return &m_state.units[it->second];
}

BuildingState* BattleSession::findCachedBuilding(const int32 unitId)
{
	rebuildBuildingIndex();
	const auto it = m_buildingIndexByUnitId.find(unitId);
	if ((it == m_buildingIndexByUnitId.end()) || (it->second >= m_state.buildings.size()))
	{
		return nullptr;
	}

	return &m_state.buildings[it->second];
}

const BuildingState* BattleSession::findCachedBuilding(const int32 unitId) const
{
	rebuildBuildingIndex();
	const auto it = m_buildingIndexByUnitId.find(unitId);
	if ((it == m_buildingIndexByUnitId.end()) || (it->second >= m_state.buildings.size()))
	{
		return nullptr;
	}

	return &m_state.buildings[it->second];
}

const UnitState* BattleSession::findCachedUnit(const int32 id) const
{
	rebuildUnitIndex();
	const auto it = m_unitIndexById.find(id);
	if ((it == m_unitIndexById.end()) || (it->second >= m_state.units.size()))
	{
		return nullptr;
	}

	return &m_state.units[it->second];
}
