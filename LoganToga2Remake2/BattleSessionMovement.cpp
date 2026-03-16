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

	void ApplyAttackPursuitTarget(const UnitState& unit, const UnitState& target, Vec2& strategicDestination, double& stopDistance)
	{
		strategicDestination = target.position;
		stopDistance = Max(BattleSessionInternal::GetEffectiveAttackRange(unit, target) - 2.0, 1.0);
		if (unit.archetype == UnitArchetype::Spinner)
		{
			stopDistance = 1.0;
		}
	}

	[[nodiscard]] bool ShouldReacquireAttackTargetNow(const UnitState& unit)
	{
		const double reacquireRadius = Max(36.0, unit.radius + 16.0);
		return (unit.attackCooldownRemaining <= 0.0)
			|| (unit.position.distanceFromSq(unit.order.targetPoint) <= (reacquireRadius * reacquireRadius));
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
					ApplyAttackPursuitTarget(unit, *target, strategicDestination, stopDistance);
					unit.order.targetPoint = strategicDestination;
				}
				else if (ShouldReacquireAttackTargetNow(unit))
				{
					if (const auto* nextTarget = findNearestEnemy(unit))
					{
						const auto* engagedTarget = TryReacquireAttackTarget(m_state, unit, unit.order, nextTarget);
						if (engagedTarget)
						{
							ApplyAttackPursuitTarget(unit, *engagedTarget, strategicDestination, stopDistance);
						}
					}
					else
					{
						TryReacquireAttackTarget(m_state, unit, unit.order, nullptr);
					}
				}
			}
			else if (ShouldReacquireAttackTargetNow(unit))
			{
				if (const auto* nextTarget = findNearestEnemy(unit))
				{
					const auto* engagedTarget = TryReacquireAttackTarget(m_state, unit, unit.order, nextTarget);
					if (engagedTarget)
					{
						ApplyAttackPursuitTarget(unit, *engagedTarget, strategicDestination, stopDistance);
					}
				}
				else
				{
					TryReacquireAttackTarget(m_state, unit, unit.order, nullptr);
				}
			}
		}
		else if ((unit.order.type == UnitOrderType::RepairTarget) && unit.order.targetUnitId)
		{
			if (const auto* target = findCachedUnit(*unit.order.targetUnitId))
			{
				const auto* building = findCachedBuilding(*unit.order.targetUnitId);
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
		if (useNavigationPath)
		{
			if (unit.movementDistanceLastFrame <= 0.01)
			{
				++unit.pathStuckFrames;
				if (unit.pathStuckFrames >= 3)
				{
					unit.pathDirty = true;
					unit.pathStuckFrames = 0;
				}
			}
			else
			{
				unit.pathStuckFrames = 0;
			}
		}
	}
}

#include "BattleSessionFormation.ipp"

const UnitState* BattleSession::findNearestEnemy(const UnitState& source) const
{
	return findNearestEnemy(source, m_nearbyOpponentIndicesScratch);
}

const UnitState* BattleSession::findNearestEnemy(const UnitState& source, Array<size_t>& nearbyOpponentIndicesScratch) const
{
	const double searchRadius = getAggroRange(source.owner, source.archetype);
	const double searchRadiusSq = (searchRadius * searchRadius);
	gatherNearbyOpponentIndices(source, searchRadius, nearbyOpponentIndicesScratch);

	const UnitState* nearest = nullptr;
	double nearestDistanceSq = searchRadiusSq;

	for (const auto index : nearbyOpponentIndicesScratch)
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
