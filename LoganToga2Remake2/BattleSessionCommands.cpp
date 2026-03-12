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
		}, command);
	}

	m_pendingCommands.clear();
}
