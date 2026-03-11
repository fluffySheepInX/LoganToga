#include "BattleSession.h"
#include "BattleSessionInternal.h"

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
	invalidateUnitIndex();
	setupInitialState();
}

void BattleSession::enqueue(BattleCommand command)
{
	m_pendingCommands << std::move(command);
}

void BattleSession::update(const double deltaTime)
{
	rebuildUnitIndex();

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
	invalidateUnitIndex();
	return m_state;
}

void BattleSession::invalidateUnitIndex() noexcept
{
	m_unitIndexDirty = true;
	m_frameUnitCacheDirty = true;
}

void BattleSession::rebuildUnitIndex() const
{
	if (!m_unitIndexDirty)
	{
		return;
	}

	m_unitIndexById.clear();
	m_unitIndexById.reserve(m_state.units.size());
	for (size_t index = 0; index < m_state.units.size(); ++index)
	{
		m_unitIndexById.emplace(m_state.units[index].id, index);
	}

	m_unitIndexDirty = false;
}

void BattleSession::rebuildFrameUnitCache() const
{
	if (!m_frameUnitCacheDirty)
	{
		return;
	}

	m_playerUnitIndices.clear();
	m_enemyUnitIndices.clear();
	m_playerBuildingIndices.clear();
	m_enemyBuildingIndices.clear();
	m_playerUnitIndices.reserve(m_state.units.size());
	m_enemyUnitIndices.reserve(m_state.units.size());
	m_playerBuildingIndices.reserve(m_state.buildings.size());
	m_enemyBuildingIndices.reserve(m_state.buildings.size());

	for (size_t index = 0; index < m_state.units.size(); ++index)
	{
		const auto& unit = m_state.units[index];
		Array<size_t>* ownerIndices = nullptr;
		Array<size_t>* ownerBuildingIndices = nullptr;

		if (unit.owner == Owner::Player)
		{
			ownerIndices = &m_playerUnitIndices;
			ownerBuildingIndices = &m_playerBuildingIndices;
		}
		else if (unit.owner == Owner::Enemy)
		{
			ownerIndices = &m_enemyUnitIndices;
			ownerBuildingIndices = &m_enemyBuildingIndices;
		}

		if (!ownerIndices)
		{
			continue;
		}

		ownerIndices->push_back(index);
		if (IsBuildingArchetype(unit.archetype))
		{
			ownerBuildingIndices->push_back(index);
		}
	}

	m_frameUnitCacheDirty = false;
}

const Array<size_t>& BattleSession::getOwnerUnitIndices(const Owner owner) const
{
	rebuildFrameUnitCache();
	return (owner == Owner::Enemy) ? m_enemyUnitIndices : m_playerUnitIndices;
}

const Array<size_t>& BattleSession::getOwnerBuildingIndices(const Owner owner) const
{
	rebuildFrameUnitCache();
	return (owner == Owner::Enemy) ? m_enemyBuildingIndices : m_playerBuildingIndices;
}

const UnitState* BattleSession::findOwnerUnitByArchetype(const Owner owner, const UnitArchetype archetype) const
{
	const auto& candidateIndices = IsBuildingArchetype(archetype)
		? getOwnerBuildingIndices(owner)
		: getOwnerUnitIndices(owner);

	for (const auto index : candidateIndices)
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive && (unit.owner == owner) && (unit.archetype == archetype))
		{
			return &unit;
		}
	}

	return nullptr;
}

bool BattleSession::hasBaseDefenseTurret(const UnitState& base, const double lockRadius) const
{
	for (const auto index : getOwnerBuildingIndices(base.owner))
	{
		const auto& unit = m_state.units[index];
		if (unit.isAlive
			&& (unit.archetype == UnitArchetype::Turret)
			&& (unit.position.distanceFrom(base.position) <= lockRadius))
		{
			return true;
		}
	}

	return false;
}

UnitState* BattleSession::findCachedUnit(const int32 id)
{
	rebuildUnitIndex();
	const auto it = m_unitIndexById.find(id);
	if ((it == m_unitIndexById.end()) || (it->second >= m_state.units.size()))
	{
		return nullptr;
	}

	return &m_state.units[it->second];
}

const UnitState* BattleSession::findCachedUnit(const int32 id) const
{
	rebuildUnitIndex();
	const auto it = m_unitIndexById.find(id);
	if ((it == m_unitIndexById.end()) || (it->second >= m_state.units.size()))
	{
		return nullptr;
	}

	return &m_state.units[it->second];
}

const BattleConfigData& BattleSession::config() const noexcept
{
	return m_config;
}

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
					if (auto* unit = findCachedUnit(unitId))
					{
						if (!unit->isAlive)
						{
							continue;
						}

						unit->order.type = UnitOrderType::AttackTarget;
						unit->order.targetUnitId = value.targetUnitId;
						BattleSessionInternal::ClearNavigationPath(*unit);
					}
				}
			}
			else if constexpr (std::is_same_v<T, SetPlayerFormationCommand>)
			{
				m_state.playerFormation = value.formation;
			}
			else if constexpr (std::is_same_v<T, IssueConstructionOrderCommand>)
			{
				const auto* worker = findCachedUnit(value.workerUnitId);
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
				if (auto* builder = findCachedUnit(value.workerUnitId))
				{
					builder->order.type = UnitOrderType::Move;
					builder->order.targetUnitId.reset();
					builder->order.targetPoint = clampedPosition;
					builder->moveTarget = clampedPosition;
					BattleSessionInternal::InvalidateNavigationPath(*builder);
				}
			}
		}, command);
	}

	m_pendingCommands.clear();
}
