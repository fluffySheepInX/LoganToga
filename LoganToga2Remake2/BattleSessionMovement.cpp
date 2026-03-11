#include "BattleSession.h"
#include "BattleSessionInternal.h"

namespace
{
	enum class EnemyAiObjectiveType
	{
		Rally,
		CaptureResource,
		Defend,
		Assault
	};

	struct EnemyAiDecision
	{
		EnemyAiObjectiveType objective = EnemyAiObjectiveType::Rally;
		Vec2 strategicDestination = Vec2::Zero();
		Optional<int32> targetUnitId;
	};

	struct EnemyAiStrategicTarget
	{
		double score = -Math::Inf;
		Vec2 position = Vec2::Zero();
		Optional<int32> unitId;
	};

	[[nodiscard]] bool IsEnemyCombatUnit(const UnitState& unit)
	{
		return unit.isAlive
			&& (unit.owner == Owner::Enemy)
			&& unit.canMove
			&& (unit.archetype != UnitArchetype::Worker);
	}

	[[nodiscard]] const UnitState* FindOwnerBuilding(const BattleState& state, const Owner owner, const UnitArchetype archetype)
	{
		for (const auto& unit : state.units)
		{
			if (unit.isAlive && (unit.owner == owner) && (unit.archetype == archetype))
			{
				return &unit;
			}
		}

		return nullptr;
	}

	[[nodiscard]] bool IsBaseDefenseTurret(const UnitState& unit, const UnitState& base, const double lockRadius)
	{
		return unit.isAlive
			&& (unit.owner == base.owner)
			&& (unit.archetype == UnitArchetype::Turret)
			&& (unit.position.distanceFrom(base.position) <= lockRadius);
	}

	[[nodiscard]] bool CanAssaultBase(const BattleState& state, const UnitState& base, const EnemyAiConfig& config)
	{
		for (const auto& unit : state.units)
		{
			if (IsBaseDefenseTurret(unit, base, config.baseAssaultLockRadius))
			{
				return false;
			}
		}

		return true;
	}

	void ConsiderStrategicTarget(EnemyAiStrategicTarget& bestTarget, const double score, const Vec2& position, const Optional<int32>& unitId = none)
	{
		if (score <= bestTarget.score)
		{
			return;
		}

		bestTarget.score = score;
		bestTarget.position = position;
		bestTarget.unitId = unitId;
	}

	[[nodiscard]] Vec2 MakeOffsetToward(const Vec2& from, const Vec2& to, const double distance)
	{
		const Vec2 direction = (to - from);
		if (direction.lengthSq() < 1.0)
		{
			return from;
		}

		return from + (direction.normalized() * distance);
	}
}

void BattleSession::updateEnemyAI(const double deltaTime)
{
	m_state.enemyAiDecisionTimer += deltaTime;
	if (m_state.enemyAiDecisionTimer < m_config.enemyAI.decisionInterval)
	{
		return;
	}

	m_state.enemyAiDecisionTimer = 0.0;

	const UnitState* enemyBase = FindOwnerBuilding(m_state, Owner::Enemy, UnitArchetype::Base);
	const UnitState* enemyBarracks = FindOwnerBuilding(m_state, Owner::Enemy, UnitArchetype::Barracks);
	const UnitState* playerBase = FindOwnerBuilding(m_state, Owner::Player, UnitArchetype::Base);
	const Vec2 enemyAnchor = enemyBarracks ? enemyBarracks->position : (enemyBase ? enemyBase->position : m_state.worldBounds.center());
	const bool canAssaultPlayerBase = playerBase && CanAssaultBase(m_state, *playerBase, m_config.enemyAI);

	const UnitState* defenseTarget = nullptr;
	double defenseTargetDistance = m_config.enemyAI.defenseRadius;
	for (const auto& candidate : m_state.units)
	{
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		double nearestAssetDistance = candidate.position.distanceFrom(enemyAnchor);
		for (const auto& building : m_state.buildings)
		{
			const auto* unit = m_state.findUnit(building.unitId);
			if (!(unit && unit->isAlive && (unit->owner == Owner::Enemy)))
			{
				continue;
			}

			nearestAssetDistance = Min(nearestAssetDistance, candidate.position.distanceFrom(unit->position));
		}

		for (const auto& resourcePoint : m_state.resourcePoints)
		{
			if (resourcePoint.owner == Owner::Enemy)
			{
				nearestAssetDistance = Min(nearestAssetDistance, candidate.position.distanceFrom(resourcePoint.position));
			}
		}

		if (nearestAssetDistance <= defenseTargetDistance)
		{
			defenseTargetDistance = nearestAssetDistance;
			defenseTarget = &candidate;
		}
	}

	const ResourcePointState* captureTarget = nullptr;
	double captureTargetDistance = Math::Inf;
	for (const auto& resourcePoint : m_state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Enemy)
		{
			continue;
		}

		const double distance = enemyAnchor.distanceFrom(resourcePoint.position);
		if (distance < captureTargetDistance)
		{
			captureTargetDistance = distance;
			captureTarget = &resourcePoint;
		}
	}

	int32 enemyCombatUnits = 0;
	for (const auto& unit : m_state.units)
	{
		if (IsEnemyCombatUnit(unit))
		{
			++enemyCombatUnits;
		}
	}

	EnemyAiStrategicTarget strategicTarget;
	for (const auto& candidate : m_state.units)
	{
		if (!candidate.isAlive || (candidate.owner != Owner::Player) || !IsBuildingArchetype(candidate.archetype))
		{
			continue;
		}

		const double distancePenalty = enemyAnchor.distanceFrom(candidate.position) * 0.25;
		double score = -Math::Inf;

		if (candidate.archetype == UnitArchetype::Turret)
		{
			score = playerBase && IsBaseDefenseTurret(candidate, *playerBase, m_config.enemyAI.baseAssaultLockRadius)
				? 960.0
				: 520.0;
		}
		else if (candidate.archetype == UnitArchetype::Barracks)
		{
			score = 760.0;
		}
		else if ((candidate.archetype == UnitArchetype::Base) && canAssaultPlayerBase)
		{
			score = 1000.0;
		}

		if (score > -Math::Inf)
		{
			const double hpRate = (candidate.maxHp > 0) ? (static_cast<double>(candidate.hp) / candidate.maxHp) : 1.0;
			ConsiderStrategicTarget(strategicTarget, score - distancePenalty + ((1.0 - hpRate) * 120.0), candidate.position, Optional<int32>{ candidate.id });
		}
	}

	if (captureTarget)
	{
		const double captureScore = 620.0 - (enemyAnchor.distanceFrom(captureTarget->position) * 0.20);
		ConsiderStrategicTarget(strategicTarget, captureScore, captureTarget->position);
	}

	const bool shouldAssault = (enemyCombatUnits >= m_config.enemyAI.assaultUnitThreshold)
		&& (strategicTarget.score > -Math::Inf);
	const Vec2 rallyPoint = shouldAssault
		? MakeOffsetToward(enemyAnchor, strategicTarget.position, m_config.enemyAI.rallyDistance)
		: enemyAnchor;

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.owner != Owner::Enemy) || !unit.canMove)
		{
			continue;
		}

		EnemyAiDecision decision;
		if (defenseTarget)
		{
			decision.objective = EnemyAiObjectiveType::Defend;
			decision.strategicDestination = defenseTarget->position;
			decision.targetUnitId = defenseTarget->id;
		}
		else if ((unit.archetype == UnitArchetype::Worker) && captureTarget)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = captureTarget->position;
		}
		else if (shouldAssault)
		{
			decision.objective = EnemyAiObjectiveType::Assault;
			decision.strategicDestination = strategicTarget.position;
			if (strategicTarget.unitId)
			{
				decision.targetUnitId = strategicTarget.unitId;
			}
		}
		else if (captureTarget)
		{
			decision.objective = EnemyAiObjectiveType::CaptureResource;
			decision.strategicDestination = captureTarget->position;
		}
		else
		{
			decision.objective = EnemyAiObjectiveType::Rally;
			decision.strategicDestination = rallyPoint;
		}

		if (!decision.targetUnitId)
		{
			if (const UnitState* nearbyTarget = findNearestEnemy(unit))
			{
				if (!((nearbyTarget->archetype == UnitArchetype::Base) && !canAssaultPlayerBase))
				{
					decision.targetUnitId = nearbyTarget->id;
					decision.strategicDestination = nearbyTarget->position;
				}
			}
		}

		if (decision.targetUnitId)
		{
			unit.order.type = UnitOrderType::AttackTarget;
			unit.order.targetUnitId = decision.targetUnitId;
			unit.order.targetPoint = decision.strategicDestination;
		}
		else
		{
			unit.order.type = UnitOrderType::Move;
			unit.order.targetUnitId.reset();
			unit.order.targetPoint = decision.strategicDestination;
		}

		unit.moveTarget = unit.order.targetPoint;
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

void BattleSession::assignFormationMove(const Array<int32>& unitIds, const Vec2& destination, const FormationType formation, const Vec2& facingDirection)
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

	const Array<Vec2> offsets = BattleSessionInternal::MakeFormationOffsets(movableUnitIds, m_state, destination, formation, facingDirection);
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
