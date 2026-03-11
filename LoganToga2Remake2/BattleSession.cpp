#include "BattleSession.h"

BattleSession::BattleSession()
	: BattleSession{ LoadBattleConfig(U"config/battle.toml") }

{
}

BattleSession::BattleSession(const BattleConfigData& config)
	: m_config{ config }
{
	setupInitialState();
}

void BattleSession::reset(const BattleConfigData& config)
{
	m_config = config;
	m_state = BattleState{};
	m_pendingCommands.clear();
	setupInitialState();
}

void BattleSession::enqueue(BattleCommand command)
{
	m_pendingCommands << std::move(command);
}

void BattleSession::update(const double deltaTime)
{
	m_state.statusMessageTimer = Max(m_state.statusMessageTimer - deltaTime, 0.0);
	if (m_state.statusMessageTimer <= 0.0)
	{
		m_state.statusMessage.clear();
	}

	if (m_state.winner)
	{
		return;
	}

	processCommands();
	updateEconomy(deltaTime);
	updateProduction(deltaTime);
	updateEnemyAI(deltaTime);
	updateMovement(deltaTime);
	updateConstructionOrders();
	updateResourcePoints(deltaTime);
	updateCombat();
	cleanupDeadUnits();
	updateVictoryState();
}

const BattleState& BattleSession::state() const noexcept
{
	return m_state;
}

BattleState& BattleSession::state() noexcept
{
	return m_state;
}

const BattleConfigData& BattleSession::config() const noexcept
{
	return m_config;
}

Array<int32> BattleSession::getSelectedPlayerUnitIds() const
{
	Array<int32> selected;
	for (const auto& unit : m_state.units)
	{
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && !IsBuildingArchetype(unit.archetype))
		{
			selected << unit.id;
		}
	}

	return selected;
}

Optional<int32> BattleSession::findSelectedPlayerWorkerId() const
{
	for (const auto& unit : m_state.units)
	{
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype == UnitArchetype::Worker))
		{
			return unit.id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findPlayerUnitAt(const Vec2& position) const
{
	for (auto it = m_state.units.rbegin(); it != m_state.units.rend(); ++it)
	{
		if (it->isAlive
			&& (it->owner == Owner::Player)
			&& !IsBuildingArchetype(it->archetype)
			&& Circle{ it->position, it->radius + 4 }.intersects(position))
		{
			return it->id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findPlayerBuildingAt(const Vec2& position) const
{
	for (auto it = m_state.units.rbegin(); it != m_state.units.rend(); ++it)
	{
		if (it->isAlive
			&& (it->owner == Owner::Player)
			&& IsBuildingArchetype(it->archetype)
			&& Circle{ it->position, it->radius + 4 }.intersects(position))
		{
			return it->id;
		}
	}

	return none;
}

Optional<int32> BattleSession::findEnemyAt(const Vec2& position) const
{
	for (auto it = m_state.units.rbegin(); it != m_state.units.rend(); ++it)
	{
		if (it->isAlive && (it->owner == Owner::Enemy) && Circle{ it->position, it->radius + 4 }.intersects(position))
		{
			return it->id;
		}
	}

	return none;
}

void BattleSession::setupInitialState()
{
	m_state.worldBounds = RectF{ 0, 0, m_config.world.width, m_config.world.height };
	m_state.playerGold = m_config.playerGold;
	m_state.enemyGold = m_config.enemyGold;

	for (const auto& placement : m_config.initialUnits)
	{
		spawnUnit(placement.owner, placement.archetype, placement.position);
	}

	for (const auto& resourcePoint : m_config.resourcePoints)
	{
		m_state.resourcePoints << ResourcePointState{
			.label = resourcePoint.label,
			.position = resourcePoint.position,
			.radius = resourcePoint.radius,
			.incomeAmount = resourcePoint.incomeAmount,
			.captureTime = resourcePoint.captureTime,
			.owner = resourcePoint.owner,
		};
	}
}

void BattleSession::processCommands()
{
	for (const auto& command : m_pendingCommands)
	{
		std::visit([this](const auto& value)
		{
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, ClearSelectionCommand>)
			{
				for (auto& unit : m_state.units)
				{
					unit.isSelected = false;
				}
			}
			else if constexpr (std::is_same_v<T, SelectUnitsInRectCommand>)
			{
				if (!value.additive)
				{
					for (auto& unit : m_state.units)
					{
						unit.isSelected = false;
					}
				}

				for (auto& unit : m_state.units)
				{
					if (!unit.isAlive || (unit.owner != Owner::Player) || IsBuildingArchetype(unit.archetype))
					{
						continue;
					}

					if (value.rect.intersects(unit.position))
					{
						unit.isSelected = true;
					}
				}
			}
			else if constexpr (std::is_same_v<T, MoveUnitsCommand>)
			{
				cancelPendingConstructionOrders(value.unitIds, true);
				assignFormationMove(value.unitIds, value.destination, value.formation, value.facingDirection);
			}
			else if constexpr (std::is_same_v<T, AttackUnitCommand>)
			{
				cancelPendingConstructionOrders(value.unitIds, true);
				removeUnitsFromSquads(value.unitIds);

				for (const auto unitId : value.unitIds)
				{
					if (auto* unit = m_state.findUnit(unitId))
					{
						if (!unit->isAlive)
						{
							continue;
						}

						unit->order.type = UnitOrderType::AttackTarget;
						unit->order.targetUnitId = value.targetUnitId;
					}
				}
			}
			else if constexpr (std::is_same_v<T, SetPlayerFormationCommand>)
			{
				m_state.playerFormation = value.formation;
			}
			else if constexpr (std::is_same_v<T, IssueConstructionOrderCommand>)
			{
				const auto* worker = m_state.findUnit(value.workerUnitId);
				if (!(worker && worker->isAlive && (worker->owner == Owner::Player) && (worker->archetype == UnitArchetype::Worker)))
				{
					m_state.statusMessage = U"No worker available";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				const int32 reservedCost = getUnitCost(value.archetype);
				if (reservedCost <= 0)
				{
					m_state.statusMessage = U"Construction unavailable";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				if (m_state.playerGold < reservedCost)
				{
					m_state.statusMessage = U"Not enough gold";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				const Vec2 clampedPosition = ClampToWorld(m_state.worldBounds, value.position, getUnitDefinition(value.archetype).radius);
				if (!canPlaceBuilding(Owner::Player, value.archetype, clampedPosition, value.workerUnitId))
				{
					m_state.statusMessage = U"Build blocked at target";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				cancelPendingConstructionOrders(Array<int32>{ value.workerUnitId }, true);
				m_state.playerGold -= reservedCost;
				m_state.pendingConstructionOrders << PendingConstructionOrder{ value.workerUnitId, value.archetype, clampedPosition, reservedCost };

				removeUnitsFromSquads(Array<int32>{ value.workerUnitId });
				if (auto* builder = m_state.findUnit(value.workerUnitId))
				{
					builder->order.type = UnitOrderType::Move;
					builder->order.targetUnitId.reset();
					builder->order.targetPoint = clampedPosition;
					builder->moveTarget = clampedPosition;
				}
			}
		}, command);
	}

	m_pendingCommands.clear();
}
