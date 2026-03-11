#include "BattleSession.h"
#include "BattleSessionInternal.h"

void BattleSession::updateEnemyAI()
{
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.owner != Owner::Enemy) || !unit.canMove)
		{
			continue;
		}

		const UnitState* target = findNearestEnemy(unit);
		if (target)
		{
			unit.order.type = UnitOrderType::AttackTarget;
			unit.order.targetUnitId = target->id;
			unit.order.targetPoint = target->position;
		}
	}
}

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

		Vec2 destination = unit.position;
		double stopDistance = 4.0;

		if ((unit.order.type == UnitOrderType::Move) || (unit.order.type == UnitOrderType::AttackTarget))
		{
			destination = unit.order.targetPoint;
		}

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			if (const auto* target = m_state.findUnit(*unit.order.targetUnitId))
			{
				if (target->isAlive && IsEnemy(unit, *target))
				{
					destination = target->position;
					stopDistance = Max(unit.attackRange - 2.0, 1.0);
					unit.order.targetPoint = destination;
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

		const Vec2 delta = destination - unit.position;
		const double distance = delta.length();

		if (distance <= stopDistance)
		{
			if (unit.order.type == UnitOrderType::Move)
			{
				unit.order.type = UnitOrderType::Idle;
			}
			continue;
		}

		const Vec2 velocity = delta.normalized() * unit.moveSpeed * deltaTime;
		Vec2 nextPosition = unit.position;
		if (velocity.length() >= distance)
		{
			nextPosition = destination;
		}
		else
		{
			nextPosition += velocity;
		}

		nextPosition = ClampToWorld(m_state.worldBounds, nextPosition, unit.radius);
		unit.position = BattleSessionInternal::ResolveObstacleMove(unit.position, nextPosition, unit.radius, m_config.obstacles);
	}
}

void BattleSession::assignFormationMove(const Array<int32>& unitIds, const Vec2& destination, const FormationType formation)
{
	removeUnitsFromSquads(unitIds);

	Array<int32> movableUnitIds;
	for (const auto unitId : unitIds)
	{
		if (const auto* unit = m_state.findUnit(unitId))
		{
			if (unit->isAlive && unit->canMove)
			{
				movableUnitIds << unitId;
			}
		}
	}

	if (movableUnitIds.isEmpty())
	{
		return;
	}

	const Array<Vec2> offsets = BattleSessionInternal::MakeFormationOffsets(movableUnitIds, m_state, destination, formation);
	const int32 squadId = m_state.nextSquadId++;
	SquadState squad;
	squad.id = squadId;
	squad.owner = Owner::Player;
	squad.formation = formation;
	squad.destination = destination;
	squad.unitIds = movableUnitIds;
	m_state.squads << squad;

	for (int32 i = 0; i < movableUnitIds.size(); ++i)
	{
		if (auto* unit = m_state.findUnit(movableUnitIds[i]))
		{
			unit->squadId = squadId;
			unit->formationOffset = (i < offsets.size()) ? offsets[i] : Vec2::Zero();
			unit->order.type = UnitOrderType::Move;
			unit->order.targetUnitId.reset();
			unit->order.targetPoint = destination + unit->formationOffset;
			unit->moveTarget = unit->order.targetPoint;
		}
	}
}

void BattleSession::removeUnitsFromSquads(const Array<int32>& unitIds)
{
	for (auto& squad : m_state.squads)
	{
		squad.unitIds.remove_if([&unitIds](const int32 squadUnitId)
		{
			return unitIds.contains(squadUnitId);
		});
	}

	for (const auto unitId : unitIds)
	{
		if (auto* unit = m_state.findUnit(unitId))
		{
			unit->squadId.reset();
			unit->formationOffset = Vec2::Zero();
		}
	}

	cleanupSquads();
}

void BattleSession::cleanupSquads()
{
	for (auto& squad : m_state.squads)
	{
		const int32 currentSquadId = squad.id;
		squad.unitIds.remove_if([this, currentSquadId](const int32 unitId)
		{
			const auto* unit = m_state.findUnit(unitId);
			return !(unit && unit->isAlive && unit->squadId && (*unit->squadId == currentSquadId));
		});
	}

	m_state.squads.remove_if([](const SquadState& squad)
	{
		return squad.unitIds.isEmpty();
	});
}

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
