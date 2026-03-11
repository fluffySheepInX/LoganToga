#include "BattleSession.h"

BattleSession::BattleSession()
	: m_config{ LoadBattleConfig(U"config/battle.toml") }
{
	setupInitialState();
}

void BattleSession::enqueue(BattleCommand command)
{
	m_pendingCommands << std::move(command);
}

void BattleSession::update(const double deltaTime)
{
	if (m_state.winner)
	{
		return;
	}

	processCommands();
	updateEconomy(deltaTime);
	updateProduction(deltaTime);
	updateEnemyAI();
	updateMovement(deltaTime);
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
				assignFormationMove(value.unitIds, value.destination, value.formation);
			}
			else if constexpr (std::is_same_v<T, AttackUnitCommand>)
			{
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
			else if constexpr (std::is_same_v<T, PlaceBuildingCommand>)
			{
				[[maybe_unused]] const bool placed = tryPlaceBuilding(Owner::Player, value.archetype, value.position);
			}
		}, command);
	}

	m_pendingCommands.clear();
}
