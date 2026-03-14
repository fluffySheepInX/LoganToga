#include "BattleSession.h"
#include "BattleSessionInternal.h"

#include "BattleSessionEnemyAI.ipp"

namespace
{
	const UnitState* TryReacquireAttackTarget(BattleState& state, const UnitState& source, UnitOrder& order, const UnitState* fallbackTarget)
	{
		if (fallbackTarget)
		{
			order.type = UnitOrderType::AttackTarget;
			order.targetUnitId = fallbackTarget->id;
			order.targetPoint = fallbackTarget->position;
			return fallbackTarget;
		}

		order.type = UnitOrderType::Idle;
		order.targetUnitId.reset();
		return nullptr;
	}
}

void BattleSession::updateMovement(const double deltaTime)
{
	invalidateSpatialQueryCache();
	rebuildNavigationGrid();

	const auto navigationGrid = BattleSessionInternal::MakeNavigationGrid(
		m_navigationGridBounds,
		m_navigationGridCellSize,
		m_navigationGridColumns,
		m_navigationGridRows,
		m_navigationGridBlocked);

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		unit.attackCooldownRemaining = Max(unit.attackCooldownRemaining - deltaTime, 0.0);
		unit.movementDistanceLastFrame = 0.0;

		if (!unit.canMove)
		{
			continue;
		}

		Vec2 strategicDestination = unit.position;
		double stopDistance = 4.0;
		const Vec2 currentPosition = unit.position;

		if ((unit.order.type == UnitOrderType::Move) || (unit.order.type == UnitOrderType::AttackTarget) || (unit.order.type == UnitOrderType::RepairTarget))
		{
			strategicDestination = unit.order.targetPoint;
		}

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			if (const auto* target = findCachedUnit(*unit.order.targetUnitId))
			{
				if (target->isAlive && IsEnemy(unit, *target))
				{
					strategicDestination = target->position;
					stopDistance = Max(BattleSessionInternal::GetEffectiveAttackRange(unit, *target) - 2.0, 1.0);
					if (unit.archetype == UnitArchetype::Spinner)
					{
						stopDistance = 1.0;
					}
		else if ((unit.order.type == UnitOrderType::RepairTarget) && unit.order.targetUnitId)
		{
			if (const auto* target = findCachedUnit(*unit.order.targetUnitId))
			{
				const auto* building = m_state.findBuildingByUnitId(*unit.order.targetUnitId);
				if (target->isAlive
					&& (target->owner == unit.owner)
					&& (target->archetype == UnitArchetype::Turret)
					&& building
					&& building->isConstructed
					&& (target->hp < target->maxHp))
				{
					strategicDestination = target->position;
					stopDistance = (unit.radius + target->radius + 8.0);
					unit.order.targetPoint = strategicDestination;
				}
				else
				{
					unit.order.type = UnitOrderType::Idle;
					unit.order.targetUnitId.reset();
				}
			}
			else
			{
				unit.order.type = UnitOrderType::Idle;
				unit.order.targetUnitId.reset();
			}
		}
					unit.order.targetPoint = strategicDestination;
				}
				else
				{
					if (const auto* nextTarget = findNearestEnemy(unit))
					{
						const auto* engagedTarget = TryReacquireAttackTarget(m_state, unit, unit.order, nextTarget);
						if (engagedTarget)
						{
							strategicDestination = engagedTarget->position;
							stopDistance = Max(BattleSessionInternal::GetEffectiveAttackRange(unit, *engagedTarget) - 2.0, 1.0);
							if (unit.archetype == UnitArchetype::Spinner)
							{
								stopDistance = 1.0;
							}
						}
					}
					else
					{
						TryReacquireAttackTarget(m_state, unit, unit.order, nullptr);
					}
				}
			}
			else
			{
				if (const auto* nextTarget = findNearestEnemy(unit))
				{
					const auto* engagedTarget = TryReacquireAttackTarget(m_state, unit, unit.order, nextTarget);
					if (engagedTarget)
					{
						strategicDestination = engagedTarget->position;
						stopDistance = Max(BattleSessionInternal::GetEffectiveAttackRange(unit, *engagedTarget) - 2.0, 1.0);
						if (unit.archetype == UnitArchetype::Spinner)
						{
							stopDistance = 1.0;
						}
					}
				}
				else
				{
					TryReacquireAttackTarget(m_state, unit, unit.order, nullptr);
				}
			}
		}

		const double strategicDistance = unit.position.distanceFrom(strategicDestination);
		if (strategicDistance <= stopDistance)
		{
			BattleSessionInternal::ClearNavigationPath(unit);
			unit.moveTarget = unit.position;
			if (unit.order.type == UnitOrderType::Move)
			{
				unit.order.type = UnitOrderType::Idle;
			}
			continue;
		}

		const bool useNavigationPath = (unit.order.type == UnitOrderType::Move)
			|| ((unit.order.type == UnitOrderType::AttackTarget)
				&& (unit.owner == Owner::Enemy)
				&& m_config.enemyAI.usePathfindingForAttackTarget);

		if (useNavigationPath)
		{
			unit.moveTarget = BattleSessionInternal::ResolveNavigationWaypoint(navigationGrid, unit, strategicDestination);
		}
		else
		{
			BattleSessionInternal::ClearNavigationPath(unit);
			unit.moveTarget = strategicDestination;
		}

		const Vec2 delta = unit.moveTarget - unit.position;
		const double distance = delta.length();

		if (distance <= 0.001)
		{
			continue;
		}

		const Vec2 velocity = delta.normalized() * unit.moveSpeed * deltaTime;
		Vec2 nextPosition = unit.position;
		if (velocity.length() >= distance)
		{
			nextPosition = unit.moveTarget;
		}
		else
		{
			nextPosition += velocity;
		}

		nextPosition = ClampToWorld(m_state.worldBounds, nextPosition, unit.radius);
		unit.position = BattleSessionInternal::ResolveObstacleMove(unit.position, nextPosition, unit.radius, m_config.obstacles);
		unit.movementDistanceLastFrame = unit.position.distanceFrom(currentPosition);
		if (useNavigationPath && (unit.position.distanceFrom(currentPosition) <= 0.01))
		{
			unit.pathDirty = true;
		}
	}
}

#include "BattleSessionFormation.ipp"

const UnitState* BattleSession::findNearestEnemy(const UnitState& source) const
{
	const double searchRadius = getAggroRange(source.owner, source.archetype);
	const double searchRadiusSq = (searchRadius * searchRadius);
	gatherNearbyOpponentIndices(source, searchRadius, m_nearbyOpponentIndicesScratch);

	const UnitState* nearest = nullptr;
	double nearestDistanceSq = searchRadiusSq;

	for (const auto index : m_nearbyOpponentIndicesScratch)
	{
		const auto& candidate = m_state.units[index];
		if (!candidate.isAlive || !IsEnemy(source, candidate))
		{
			continue;
		}

		const double distanceSq = source.position.distanceFromSq(candidate.position);
		if (distanceSq <= nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;
			nearest = &candidate;
		}
	}

	return nearest;
}
