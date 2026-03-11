#include "BattleSession.h"
#include "BattleSessionInternal.h"

#include "BattleSessionEnemyAI.ipp"

void BattleSession::updateMovement(const double deltaTime)
{
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		unit.attackCooldownRemaining = Max(unit.attackCooldownRemaining - deltaTime, 0.0);

		if (!unit.canMove)
		{
			continue;
		}

		Vec2 strategicDestination = unit.position;
		double stopDistance = 4.0;

		if ((unit.order.type == UnitOrderType::Move) || (unit.order.type == UnitOrderType::AttackTarget))
		{
			strategicDestination = unit.order.targetPoint;
		}

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			if (const auto* target = m_state.findUnit(*unit.order.targetUnitId))
			{
				if (target->isAlive && IsEnemy(unit, *target))
				{
					strategicDestination = target->position;
					stopDistance = Max(unit.attackRange - 2.0, 1.0);
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
			unit.moveTarget = unit.position;
			if (unit.order.type == UnitOrderType::Move)
			{
				unit.order.type = UnitOrderType::Idle;
			}
			continue;
		}

		unit.moveTarget = BattleSessionInternal::ResolveNavigationWaypoint(unit.position, strategicDestination, unit.radius, m_config.obstacles);
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
	}
}

#include "BattleSessionFormation.ipp"

const UnitState* BattleSession::findNearestEnemy(const UnitState& source) const
{
	const UnitState* nearest = nullptr;
	double nearestDistance = getAggroRange(source.archetype);

	for (const auto& candidate : m_state.units)
	{
		if (!candidate.isAlive || !IsEnemy(source, candidate))
		{
			continue;
		}

		const double distance = source.position.distanceFrom(candidate.position);
		if (distance <= nearestDistance)
		{
			nearestDistance = distance;
			nearest = &candidate;
		}
	}

	return nearest;
}
