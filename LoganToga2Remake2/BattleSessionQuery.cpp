#include "BattleSession.h"

Array<int32> BattleSession::getSelectedPlayerUnitIds() const
{
	const auto& playerUnitIndices = getOwnerUnitIndices(Owner::Player);
	Array<int32> selected;
	selected.reserve(playerUnitIndices.size());
	for (const auto index : playerUnitIndices)
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && !IsBuildingArchetype(unit.archetype))
		{
			selected << unit.id;
		}
	}

	return selected;
}

Optional<int32> BattleSession::findSelectedPlayerWorkerId() const
{
	for (const auto index : getOwnerUnitIndices(Owner::Player))
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
		{
			return unit.id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findSelectedPlayerTurretId() const
{
	int32 selectedCount = 0;
	Optional<int32> turretId;

	for (const auto index : getOwnerUnitIndices(Owner::Player))
	{
		const auto& unit = m_state.units[index];
		if (!(unit.isAlive && unit.isSelected && (unit.owner == Owner::Player)))
		{
			continue;
		}

		++selectedCount;
		if (unit.archetype == UnitArchetype::Turret)
		{
			turretId = unit.id;
		}
	}

	if ((selectedCount != 1) || !turretId)
	{
		return none;
	}

	const auto* building = findCachedBuilding(*turretId);
	if (!(building && building->isConstructed))
	{
		return none;
	}

	return turretId;
}

Optional<int32> BattleSession::findPlayerUnitAt(const Vec2& position) const
{
	const auto& playerUnitIndices = getOwnerUnitIndices(Owner::Player);
	for (auto it = playerUnitIndices.rbegin(); it != playerUnitIndices.rend(); ++it)
	{
		const auto& unit = m_state.units[*it];
		if (unit.isAlive
			&& (unit.owner == Owner::Player)
			&& !IsBuildingArchetype(unit.archetype)
			&& Circle{ unit.position, unit.radius + 4 }.intersects(position))
		{
			return unit.id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findPlayerBuildingAt(const Vec2& position) const
{
	const auto& playerBuildingIndices = getOwnerBuildingIndices(Owner::Player);
	for (auto it = playerBuildingIndices.rbegin(); it != playerBuildingIndices.rend(); ++it)
	{
		const auto& unit = m_state.units[*it];
		if (unit.isAlive
			&& (unit.owner == Owner::Player)
			&& IsBuildingArchetype(unit.archetype)
			&& Circle{ unit.position, unit.radius + 4 }.intersects(position))
		{
			return unit.id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findEnemyAt(const Vec2& position) const
{
	const auto& enemyUnitIndices = getOwnerUnitIndices(Owner::Enemy);
	for (auto it = enemyUnitIndices.rbegin(); it != enemyUnitIndices.rend(); ++it)
	{
		const auto& unit = m_state.units[*it];
		if (unit.isAlive && (unit.owner == Owner::Enemy) && Circle{ unit.position, unit.radius + 4 }.intersects(position))
		{
			return unit.id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findEnemyNear(const Vec2& position, const double snapRadius) const
{
	const auto& enemyUnitIndices = getOwnerUnitIndices(Owner::Enemy);
	Optional<int32> nearestId;
	double nearestDistance = Math::Inf;

	for (auto it = enemyUnitIndices.rbegin(); it != enemyUnitIndices.rend(); ++it)
	{
		const auto& unit = m_state.units[*it];
		if (!unit.isAlive || (unit.owner != Owner::Enemy))
		{
			continue;
		}

		const double expandedRadius = (unit.radius + snapRadius);
		if (!Circle{ unit.position, expandedRadius }.intersects(position))
		{
			continue;
		}

		const double distanceToSurface = Max(position.distanceFrom(unit.position) - unit.radius, 0.0);
		if (distanceToSurface < nearestDistance)
		{
			nearestDistance = distanceToSurface;
			nearestId = unit.id;
		}
	}

	return nearestId;
}
