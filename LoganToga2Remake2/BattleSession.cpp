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
		if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype != UnitArchetype::Base))
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

bool BattleSession::trySpawnPlayerUnit(const UnitArchetype archetype)
{
	return tryQueueUnitProduction(Owner::Player, archetype);
}

bool BattleSession::cancelLastPlayerProduction()
{
	auto* building = findProductionBuilding(Owner::Player);
	if (!(building && !building->productionQueue.isEmpty()))
	{
		return false;
	}

	const UnitArchetype archetype = building->productionQueue.back().archetype;
	building->productionQueue.pop_back();
	m_state.playerGold += getUnitCost(archetype);
	return true;
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
					if (!unit.isAlive || (unit.owner != Owner::Player) || (unit.archetype == UnitArchetype::Base))
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
				const int32 count = static_cast<int32>(value.unitIds.size());
				const double spacing = 26.0;
				const double startOffset = -((count - 1) * spacing * 0.5);

				for (int32 i = 0; i < count; ++i)
				{
					if (auto* unit = m_state.findUnit(value.unitIds[i]))
					{
						if (!unit->isAlive || !unit->canMove)
						{
							continue;
						}

						unit->order.type = UnitOrderType::Move;
						unit->order.targetUnitId.reset();
						unit->order.targetPoint = value.destination + Vec2{ 0, startOffset + spacing * i };
						unit->moveTarget = unit->order.targetPoint;
					}
				}
			}
			else if constexpr (std::is_same_v<T, AttackUnitCommand>)
			{
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
		}, command);
	}

	m_pendingCommands.clear();
}

void BattleSession::updateEconomy(const double deltaTime)
{
	m_state.playerIncomeTimer += deltaTime;
	m_state.enemyIncomeTimer += deltaTime;
	m_state.enemySpawnTimer += deltaTime;

	if (m_state.playerIncomeTimer >= m_config.income.interval)
	{
		m_state.playerIncomeTimer -= m_config.income.interval;
		m_state.playerGold += m_config.income.playerAmount;
	}

	if (m_state.enemyIncomeTimer >= m_config.income.interval)
	{
		m_state.enemyIncomeTimer -= m_config.income.interval;
		m_state.enemyGold += m_config.income.enemyAmount;
	}

	if (m_state.enemySpawnTimer >= m_config.enemySpawn.interval)
	{
		m_state.enemySpawnTimer -= m_config.enemySpawn.interval;
		const UnitArchetype archetype = ((m_state.enemyGold >= getUnitCost(m_config.enemySpawn.advancedArchetype)) && RandomBool(m_config.enemySpawn.advancedProbability))
			? m_config.enemySpawn.advancedArchetype
			: m_config.enemySpawn.basicArchetype;
		tryQueueUnitProduction(Owner::Enemy, archetype);
	}
}

void BattleSession::updateProduction(const double deltaTime)
{
	for (auto& building : m_state.buildings)
	{
		if (building.productionQueue.isEmpty())
		{
			continue;
		}

		const auto* buildingUnit = m_state.findUnit(building.unitId);
		if (!(buildingUnit && buildingUnit->isAlive))
		{
			continue;
		}

		auto& currentItem = building.productionQueue.front();
		currentItem.remainingTime = Max(currentItem.remainingTime - deltaTime, 0.0);

		if (currentItem.remainingTime > 0.0)
		{
			continue;
		}

		const Vec2 spawnPosition = getProductionSpawnPoint(*buildingUnit, currentItem.archetype);
		spawnUnit(buildingUnit->owner, currentItem.archetype, spawnPosition);
		building.productionQueue.remove_at(0);
	}
}

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
		if (velocity.length() >= distance)
		{
			unit.position = destination;
		}
		else
		{
			unit.position += velocity;
		}

		unit.position = ClampToWorld(m_state.worldBounds, unit.position, unit.radius);
	}
}

void BattleSession::updateCombat()
{
	struct DamageEvent
	{
		int32 targetId = -1;
		int32 damage = 0;
	};

	Array<DamageEvent> damageEvents;

	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		const UnitState* target = nullptr;

		if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
		{
			target = m_state.findUnit(*unit.order.targetUnitId);
			if (!(target && target->isAlive && IsEnemy(unit, *target)))
			{
				target = nullptr;
				unit.order.type = UnitOrderType::Idle;
				unit.order.targetUnitId.reset();
			}
		}

		if (!target)
		{
			target = findNearestEnemy(unit);
		}

		if (!target)
		{
			continue;
		}

		if (unit.position.distanceFrom(target->position) > unit.attackRange)
		{
			continue;
		}

		if (unit.attackCooldownRemaining > 0.0)
		{
			continue;
		}

		damageEvents << DamageEvent{ target->id, unit.attackPower };
		unit.attackCooldownRemaining = unit.attackCooldown;
	}

	for (const auto& event : damageEvents)
	{
		if (auto* target = m_state.findUnit(event.targetId))
		{
			target->hp -= event.damage;
			if (target->hp <= 0)
			{
				target->hp = 0;
				target->isAlive = false;
			}
		}
	}
}

void BattleSession::cleanupDeadUnits()
{
	for (auto& unit : m_state.units)
	{
		if (!unit.isAlive)
		{
			unit.isSelected = false;
		}
	}

	m_state.units.remove_if([](const UnitState& unit)
	{
		return !unit.isAlive && (unit.archetype != UnitArchetype::Base);
	});

	m_state.buildings.remove_if([this](const BuildingState& building)
	{
		const auto* unit = m_state.findUnit(building.unitId);
		return !(unit && unit->isAlive);
	});
}

void BattleSession::updateVictoryState()
{
	bool hasPlayerBase = false;
	bool hasEnemyBase = false;

	for (const auto& unit : m_state.units)
	{
		if (!unit.isAlive || (unit.archetype != UnitArchetype::Base))
		{
			continue;
		}

		if (unit.owner == Owner::Player)
		{
			hasPlayerBase = true;
		}
		else
		{
			hasEnemyBase = true;
		}
	}

	if (!hasEnemyBase)
	{
		m_state.winner = Owner::Player;
	}
	else if (!hasPlayerBase)
	{
		m_state.winner = Owner::Enemy;
	}
}

int32 BattleSession::spawnUnit(const Owner owner, const UnitArchetype archetype, const Vec2& position)
{
	const int32 id = m_state.nextUnitId++;
	m_state.units << makeUnit(id, owner, archetype, position);
	if (archetype == UnitArchetype::Base)
	{
		m_state.buildings << BuildingState{ id };
	}
	return id;
}

BuildingState* BattleSession::findProductionBuilding(const Owner owner)
{
	for (auto& building : m_state.buildings)
	{
		if (const auto* unit = m_state.findUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == UnitArchetype::Base))
			{
				return &building;
			}
		}
	}

	return nullptr;
}

const BuildingState* BattleSession::findProductionBuilding(const Owner owner) const
{
	for (const auto& building : m_state.buildings)
	{
		if (const auto* unit = m_state.findUnit(building.unitId))
		{
			if (unit->isAlive && (unit->owner == owner) && (unit->archetype == UnitArchetype::Base))
			{
				return &building;
			}
		}
	}

	return nullptr;
}

bool BattleSession::tryQueueUnitProduction(const Owner owner, const UnitArchetype archetype)
{
	if (m_state.winner)
	{
		return false;
	}

	auto* building = findProductionBuilding(owner);
	if (!building)
	{
		return false;
	}

	const int32 cost = getUnitCost(archetype);
	if (cost <= 0)
	{
		return false;
	}

	int32& gold = (owner == Owner::Player) ? m_state.playerGold : m_state.enemyGold;
	if (gold < cost)
	{
		return false;
	}

	gold -= cost;
	building->productionQueue << ProductionQueueItem{ archetype, getProductionTime(archetype), getProductionTime(archetype) };
	return true;
}

Vec2 BattleSession::getProductionSpawnPoint(const UnitState& buildingUnit, const UnitArchetype archetype) const
{
	const auto& unitDefinition = getUnitDefinition(archetype);
	const double direction = (buildingUnit.owner == Owner::Player) ? 1.0 : -1.0;
	const Vec2 spawnPosition = buildingUnit.position.movedBy(
		direction * (buildingUnit.radius + unitDefinition.radius + 20.0),
		Random(-buildingUnit.radius * 0.6, buildingUnit.radius * 0.6));
	return ClampToWorld(m_state.worldBounds, spawnPosition, unitDefinition.radius);
}

UnitState BattleSession::makeUnit(const int32 id, const Owner owner, const UnitArchetype archetype, const Vec2& position) const
{
	const auto& definition = getUnitDefinition(archetype);

	UnitState unit;
	unit.id = id;
	unit.owner = owner;
	unit.archetype = archetype;
	unit.position = position;
	unit.moveTarget = position;
	unit.radius = definition.radius;
	unit.moveSpeed = definition.moveSpeed;
	unit.attackRange = definition.attackRange;
	unit.attackCooldown = definition.attackCooldown;
	unit.attackPower = definition.attackPower;
	unit.hp = definition.hp;
	unit.maxHp = definition.hp;
	unit.canMove = definition.canMove;

	return unit;
}

const UnitDefinition& BattleSession::getUnitDefinition(const UnitArchetype archetype) const
{
	for (const auto& definition : m_config.unitDefinitions)
	{
		if (definition.archetype == archetype)
		{
			return definition;
		}
	}

	throw Error{ U"Unit definition not found" };
}

int32 BattleSession::getUnitCost(const UnitArchetype archetype) const
{
	return getUnitDefinition(archetype).cost;
}

double BattleSession::getProductionTime(const UnitArchetype archetype) const
{
	return getUnitDefinition(archetype).productionTime;
}

double BattleSession::getAggroRange(const UnitArchetype archetype) const
{
	return getUnitDefinition(archetype).aggroRange;
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
