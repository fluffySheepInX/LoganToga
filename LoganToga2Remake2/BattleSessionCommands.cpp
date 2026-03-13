#include "BattleSession.h"
#include "BattleSessionInternal.h"

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
			else if constexpr (std::is_same_v<T, IssueTurretUpgradeCommand>)
			{
				auto* turret = findCachedUnit(value.turretUnitId);
				if (!(turret && turret->isAlive && (turret->owner == Owner::Player) && (turret->archetype == UnitArchetype::Turret)))
				{
					m_state.statusMessage = U"No turret selected";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				auto* building = m_state.findBuildingByUnitId(value.turretUnitId);
				if (!(building && building->isConstructed))
				{
					m_state.statusMessage = U"Turret offline";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				if (building->turretUpgrade)
				{
					m_state.statusMessage = U"Turret already upgraded";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				const auto* definition = FindTurretUpgradeDefinition(m_config, value.type);
				if (!definition)
				{
					m_state.statusMessage = U"Upgrade unavailable";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				if (!ContainsTurretUpgradeType(m_config.playerAvailableTurretUpgrades, value.type))
				{
					m_state.statusMessage = U"Upgrade locked";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				if (m_state.playerGold < definition->cost)
				{
					m_state.statusMessage = U"Not enough gold";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				m_state.playerGold -= definition->cost;
				turret->attackPower += definition->attackPowerDelta;
				turret->attackCooldown = Max(0.15, turret->attackCooldown + definition->attackCooldownDelta);
				turret->attackCooldownRemaining = Min(turret->attackCooldownRemaining, turret->attackCooldown);
				building->turretUpgrade = value.type;
				m_state.statusMessage = GetTurretUpgradeLabel(value.type) + U" upgrade complete";
				m_state.statusMessageTimer = 1.5;
			}
			else if constexpr (std::is_same_v<T, IssueRepairOrderCommand>)
			{
				auto* turret = findCachedUnit(value.targetUnitId);
				if (!(turret && turret->isAlive && (turret->owner == Owner::Player) && (turret->archetype == UnitArchetype::Turret)))
				{
					m_state.statusMessage = U"No turret selected";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				auto* building = m_state.findBuildingByUnitId(value.targetUnitId);
				if (!(building && building->isConstructed))
				{
					m_state.statusMessage = U"Turret offline";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				if (turret->hp >= turret->maxHp)
				{
					m_state.statusMessage = U"Turret already at full HP";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				Array<int32> workerIds;
				for (const auto unitId : value.unitIds)
				{
					const auto* worker = findCachedUnit(unitId);
					if (worker && worker->isAlive && (worker->owner == Owner::Player) && (worker->archetype == UnitArchetype::Worker))
					{
						workerIds << unitId;
					}
				}

				if (workerIds.isEmpty())
				{
					m_state.statusMessage = U"No worker available";
					m_state.statusMessageTimer = 2.0;
					return;
				}

				cancelPendingConstructionOrders(workerIds, true);
				removeUnitsFromSquads(workerIds);

				for (const auto unitId : workerIds)
				{
					if (auto* worker = findCachedUnit(unitId))
					{
						worker->order.type = UnitOrderType::RepairTarget;
						worker->order.targetUnitId = value.targetUnitId;
						worker->order.targetPoint = turret->position;
						worker->moveTarget = turret->position;
						BattleSessionInternal::InvalidateNavigationPath(*worker);
					}
				}

				m_state.statusMessage = U"Repair ordered";
				m_state.statusMessageTimer = 1.5;
			}
			else if constexpr (std::is_same_v<T, IssueGoliathDetonationCommand>)
			{
				Array<int32> armedUnitIds;
				for (const auto unitId : value.unitIds)
				{
					auto* unit = findCachedUnit(unitId);
					if (!(unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == UnitArchetype::Goliath)))
					{
						continue;
					}

					if (unit->isDetonating)
					{
						continue;
					}

					armedUnitIds << unitId;
					unit->isDetonating = true;
					unit->detonationFramesRemaining = 15;
					unit->order.type = UnitOrderType::Idle;
					unit->order.targetUnitId.reset();
					unit->order.targetPoint = unit->position;
					unit->moveTarget = unit->position;
					BattleSessionInternal::ClearNavigationPath(*unit);
				}

				if (armedUnitIds.isEmpty())
				{
					m_state.statusMessage = U"No goliath ready";
					m_state.statusMessageTimer = 1.5;
					return;
				}

				cancelPendingConstructionOrders(armedUnitIds, true);
				removeUnitsFromSquads(armedUnitIds);
				m_state.statusMessage = (armedUnitIds.size() >= 2) ? U"Goliaths armed" : U"Goliath armed";
				m_state.statusMessageTimer = 1.0;
			}
		}, command);
	}

	m_pendingCommands.clear();
}

bool BattleSession::tryUpgradeSelectedTurret(const TurretUpgradeType type)
{
	if (const auto turretId = findSelectedPlayerTurretId())
	{
		enqueue(IssueTurretUpgradeCommand{ *turretId, type });
		return true;
	}

	return false;
}
