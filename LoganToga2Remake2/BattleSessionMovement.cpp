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

		for (const auto& candidate : state.units)
		{
			if (!candidate.isAlive || !IsEnemy(source, candidate))
			{
				continue;
			}

			const double distance = source.position.distanceFrom(candidate.position);
			if (distance > BattleSessionInternal::GetEffectiveAttackRange(source, candidate))
			{
				continue;
			}

			order.type = UnitOrderType::AttackTarget;
			order.targetUnitId = candidate.id;
			order.targetPoint = candidate.position;
			return &candidate;
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

		if (!unit.canMove)
		{
			continue;
		}

		Vec2 strategicDestination = unit.position;
		double stopDistance = 4.0;
		const Vec2 currentPosition = unit.position;

		if ((unit.order.type == UnitOrderType::Move) || (unit.order.type == UnitOrderType::AttackTarget))
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

		if (unit.order.type == UnitOrderType::Move)
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
		if ((unit.order.type == UnitOrderType::Move) && (unit.position.distanceFrom(currentPosition) <= 0.01))
		{
			unit.pathDirty = true;
		}
	}
}

#include "BattleSessionFormation.ipp"

const UnitState* BattleSession::findNearestEnemy(const UnitState& source) const
{
	const auto& candidateIndices = (source.owner == Owner::Player)
		? getOwnerUnitIndices(Owner::Enemy)
		: getOwnerUnitIndices(Owner::Player);
	const UnitState* nearest = nullptr;
	double nearestDistance = getAggroRange(source.owner, source.archetype);

	for (const auto index : candidateIndices)
	{
		const auto& candidate = m_state.units[index];
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
