# pragma once
# include <Siv3D.hpp>
# include "BattleCarrierOrders.h"
# include "BattleMoveOrders.h"

namespace LT3
{
	enum class BuildPlacementCellState
	{
		Allowed,
		OutOfBounds,
		BlockedByResource,
		BlockedByOccupancy,
	};

	inline BuildPlacementCellState EvaluateBuildPlacementCell(const BattleWorld& world, const DefinitionStores& defs, const Point& cell)
	{
		if (!world.map.inBounds(cell.y, cell.x))
		{
			return BuildPlacementCellState::OutOfBounds;
		}

		for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
		{
			const Point resourceCell = WorldToBattleCell(world, world.resourceNodes.position[node]);
			if (resourceCell == cell)
			{
				return BuildPlacementCellState::BlockedByResource;
			}
		}

		if (world.map.hasBarrierReservation(cell.y, cell.x))
		{
			return BuildPlacementCellState::BlockedByOccupancy;
		}

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit))
			{
				continue;
			}

			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& unitDef = defs.units[world.units.defId[unit]];
			const bool occupiesCell = (unitDef.role == UnitRole::Base)
				|| (unitDef.role == UnitRole::Barrier)
				|| unitDef.blocksTileMovement;
			if (!occupiesCell)
			{
				continue;
			}

			const Point occupiedCell = WorldToBattleCell(world, world.units.position[unit]);
			if (occupiedCell == cell)
			{
				return BuildPlacementCellState::BlockedByOccupancy;
			}
		}

		for (size_t i = 0; i < world.placedObjects.position.size(); ++i)
		{
			const Point objectCell = WorldToBattleCell(world, world.placedObjects.position[i]);
			if (objectCell == cell)
			{
				return BuildPlacementCellState::BlockedByOccupancy;
			}
		}

		return BuildPlacementCellState::Allowed;
	}

	inline BuildPlacementCellState EvaluateBuildPlacementCell(const BattleWorld& world, const DefinitionStores& defs, const Vec2& worldPosition)
	{
		return EvaluateBuildPlacementCell(world, defs, WorldToBattleCell(world, worldPosition));
	}

	inline Vec2 ResolveBuildApproachPosition(const BattleWorld& world, const Vec2& buildTarget)
	{
		const Point targetCell = WorldToBattleCell(world, buildTarget);
		const Point offsets[4] = {
			Point{ 1, 0 },
			Point{ -1, 0 },
			Point{ 0, 1 },
			Point{ 0, -1 }
		};

		Optional<Point> bestCell;
		double bestDistanceSq = DBL_MAX;
		for (const Point& offset : offsets)
		{
			const Point candidate{ targetCell.x + offset.x, targetCell.y + offset.y };
			if (!world.map.inBounds(candidate.y, candidate.x))
			{
				continue;
			}
			if (!IsPathCellPassable(world, candidate))
			{
				continue;
			}

			const Vec2 worldPos = BattleCellToWorldPosition(candidate);
			const double distanceSq = worldPos.distanceFromSq(buildTarget);
			if (!bestCell || distanceSq < bestDistanceSq)
			{
				bestCell = candidate;
				bestDistanceSq = distanceSq;
			}
		}

		if (bestCell)
		{
			return BattleCellToWorldPosition(*bestCell);
		}

		return buildTarget;
	}

	inline Array<Vec2> BuildLinePlacementTargets(const BattleWorld& world, const BuildActionDef& action, const Vec2& startWorld, const Vec2& currentWorld)
	{
		const Point start = WorldToBattleCell(world, startWorld);
		Point end = WorldToBattleCell(world, currentWorld);

		const int32 dx = end.x - start.x;
		const int32 dy = end.y - start.y;
		const bool horizontalOnly = (action.lineAxisMode == BuildLineAxisMode::HorizontalOnly);
		const bool verticalOnly = (action.lineAxisMode == BuildLineAxisMode::VerticalOnly);
		const bool autoDiagonal = (!horizontalOnly && !verticalOnly && dx != 0 && dy != 0);
		if (horizontalOnly)
		{
			end.y = start.y;
		}
		else if (verticalOnly)
		{
			end.x = start.x;
		}
		else if (!autoDiagonal)
		{
			if (Abs(dx) >= Abs(dy))
			{
				end.y = start.y;
			}
			else
			{
				end.x = start.x;
			}
		}

		Array<Vec2> targets;
		HashSet<Point> uniqueCells;
		const int32 stepX = (end.x == start.x) ? 0 : ((end.x > start.x) ? 1 : -1);
		const int32 stepY = (end.y == start.y) ? 0 : ((end.y > start.y) ? 1 : -1);
		const int32 count = Min(action.maxLineCells, Max(1, Max(Abs(end.x - start.x), Abs(end.y - start.y)) + 1));
		const int32 thickness = Max(1, action.lineThicknessCells);
		const int32 halfThickness = thickness / 2;
		targets.reserve(count * thickness);

		const int32 offsetStepX = (stepX == 0) ? 1 : (autoDiagonal ? 0 : 0);
		const int32 offsetStepY = (stepY == 0) ? 1 : (autoDiagonal ? 0 : 0);

		for (int32 i = 0; i < count; ++i)
		{
			const Point baseCell{ start.x + stepX * i, start.y + stepY * i };
			if (!world.map.inBounds(baseCell.y, baseCell.x))
			{
				break;
			}

			for (int32 t = 0; t < thickness; ++t)
			{
				const int32 offset = t - halfThickness;
				const Point cell{ baseCell.x + offset * offsetStepX, baseCell.y + offset * offsetStepY };
				if (!world.map.inBounds(cell.y, cell.x) || uniqueCells.contains(cell))
				{
					continue;
				}

				uniqueCells.insert(cell);
				targets << BattleCellToWorldPosition(cell);
			}
		}

		return targets;
	}

	inline bool IsBuildingStyleBuildAction(const DefinitionStores& defs, const BuildActionDef& action)
	{
		if (action.resultType == BuildActionResultType::Object)
		{
			return true;
		}

		if (action.resultType == BuildActionResultType::Unit
			&& action.spawnUnit < defs.units.size()
			&& (defs.units[action.spawnUnit].role == UnitRole::Base
				|| defs.units[action.spawnUnit].role == UnitRole::Barrier))
		{
			return true;
		}

		return false;
	}

	inline String ResolveLinePlacementIconByDelta(const BuildActionDef& action, const Point& delta)
	{
		const int32 screenDx = (delta.x - delta.y);
		const int32 screenDy = (delta.x + delta.y);

		// 画面上で水平または垂直に並ぶ方向（対角ワールド移動）は「横」アイコン
		if (screenDy == 0 || screenDx == 0)
		{
			return action.lineIconHorizontal;
		}

		if ((screenDx * screenDy) < 0)
		{
			return action.lineIconDiagUpRight;
		}

		return action.lineIconDiagUpLeft;
	}

	inline String ResolveLinePlacementIconAt(const BattleWorld& world, const BuildActionDef& action, const Array<Vec2>& targets, size_t index, const Point& fallbackDirection)
	{
		if (targets.isEmpty() || index >= targets.size())
		{
			return action.lineIconHorizontal;
		}

		if (targets.size() <= 1)
		{
			return action.lineIconHorizontal;
		}

		Point direction = fallbackDirection;
		const Point current = WorldToBattleCell(world, targets[index]);
		if ((index + 1) < targets.size())
		{
			const Point next = WorldToBattleCell(world, targets[index + 1]);
			direction = Point{ next.x - current.x, next.y - current.y };
		}
		else if (index > 0)
		{
			const Point prev = WorldToBattleCell(world, targets[index - 1]);
			direction = Point{ current.x - prev.x, current.y - prev.y };
		}

		return ResolveLinePlacementIconByDelta(action, direction);
	}

	inline bool TryStartBuild(BattleWorld& world, const DefinitionStores& defs, UnitId builder, BuildActionDefId actionId, const Optional<Vec2>& targetPosition = none)
	{
		if (!CanStartBuildAction(world, defs, builder, actionId)) return false;
		if (IsUniqueBuildActionBlocked(world, defs, actionId)) return false;

		const BuildActionDef& action = defs.buildActions[actionId];
		if (IsBuilderBusyWithBuildQueue(world, builder))
		{
			return false;
		}

		auto consumeBuildActionCost = [&]()
		{
			const ResourceDefId goldResource = FindResourceDefByKind(defs, ResourceKind::Gold);
			const ResourceDefId trustResource = FindResourceDefByKind(defs, ResourceKind::Trust);
			const ResourceDefId foodResource = FindResourceDefByKind(defs, ResourceKind::Food);
			if (goldResource != InvalidResourceDefId && goldResource < world.resources.playerAmounts.size())
			{
				world.resources.playerAmounts[goldResource] -= action.costGold;
			}
			if (trustResource != InvalidResourceDefId && trustResource < world.resources.playerAmounts.size())
			{
				world.resources.playerAmounts[trustResource] -= action.costTrust;
			}
			if (foodResource != InvalidResourceDefId && foodResource < world.resources.playerAmounts.size())
			{
				world.resources.playerAmounts[foodResource] -= action.costFood;
			}
		};

		if (action.resultType == BuildActionResultType::Carrier)
		{
			if (!TryExecuteCarrierAction(world, defs, builder, action))
			{
				return false;
			}
			consumeBuildActionCost();
			return true;
		}

		consumeBuildActionCost();

		const Optional<Vec2> resolvedTargetPosition = targetPosition.has_value()
			? Optional<Vec2>{ SnapWorldToBattleCellCenter(world, *targetPosition) }
			: none;

		if (resolvedTargetPosition && IsBuildingStyleBuildAction(defs, action))
		{
			if (EvaluateBuildPlacementCell(world, defs, *resolvedTargetPosition) != BuildPlacementCellState::Allowed)
			{
				return false;
			}

			world.buildQueues.pendingEntry[builder] = QueuedBuildAction{ actionId, *resolvedTargetPosition, true };
			world.buildQueues.hasPendingEntry[builder] = true;
			SetBuildQueueLocked(world, builder, true);
			ResetBuildQueueProgress(world, builder);
			ClearUnitAttackTarget(world, builder);
			IssueMove(world, builder, ResolveBuildApproachPosition(world, *resolvedTargetPosition));
			return true;
		}

		Array<QueuedBuildAction>& queue = world.buildQueues.entries[builder];
		const bool wasEmpty = queue.isEmpty();
		queue << QueuedBuildAction{ actionId, resolvedTargetPosition.value_or(Vec2{ 0, 0 }), resolvedTargetPosition.has_value() };
		SetBuildQueueLocked(world, builder, true);
		if (wasEmpty)
		{
			SetUnitBuilding(world, builder);
			ResetBuildQueueProgress(world, builder);
		}
		return true;
	}

	inline bool TryStartBuildLine(BattleWorld& world, const DefinitionStores& defs, UnitId builder, BuildActionDefId actionId, const Array<Vec2>& targetPositions)
	{
		if (!CanStartBuildAction(world, defs, builder, actionId)) return false;
		if (IsUniqueBuildActionBlocked(world, defs, actionId)) return false;
		if (targetPositions.isEmpty()) return false;

		const BuildActionDef& action = defs.buildActions[actionId];
		if (IsBuilderBusyWithBuildQueue(world, builder))
		{
			return false;
		}

		Array<Vec2> validTargets;
		validTargets.reserve(targetPositions.size());
		for (const Vec2& target : targetPositions)
		{
			const Vec2 snappedTarget = SnapWorldToBattleCellCenter(world, target);
			if (EvaluateBuildPlacementCell(world, defs, snappedTarget) == BuildPlacementCellState::Allowed)
			{
				validTargets << snappedTarget;
			}
		}

		if (validTargets.isEmpty())
		{
			return false;
		}

		const int32 totalCostGold = action.costGold * static_cast<int32>(validTargets.size());
		const int32 totalCostTrust = action.costTrust * static_cast<int32>(validTargets.size());
		const int32 totalCostFood = action.costFood * static_cast<int32>(validTargets.size());
		const ResourceDefId goldResource = FindResourceDefByKind(defs, ResourceKind::Gold);
		const ResourceDefId trustResource = FindResourceDefByKind(defs, ResourceKind::Trust);
		const ResourceDefId foodResource = FindResourceDefByKind(defs, ResourceKind::Food);

		if (goldResource != InvalidResourceDefId && (goldResource >= world.resources.playerAmounts.size() || world.resources.playerAmounts[goldResource] < totalCostGold))
		{
			return false;
		}
		if (trustResource != InvalidResourceDefId && (trustResource >= world.resources.playerAmounts.size() || world.resources.playerAmounts[trustResource] < totalCostTrust))
		{
			return false;
		}
		if (foodResource != InvalidResourceDefId && (foodResource >= world.resources.playerAmounts.size() || world.resources.playerAmounts[foodResource] < totalCostFood))
		{
			return false;
		}

		if (goldResource != InvalidResourceDefId && goldResource < world.resources.playerAmounts.size())
		{
			world.resources.playerAmounts[goldResource] -= totalCostGold;
		}
		if (trustResource != InvalidResourceDefId && trustResource < world.resources.playerAmounts.size())
		{
			world.resources.playerAmounts[trustResource] -= totalCostTrust;
		}
		if (foodResource != InvalidResourceDefId && foodResource < world.resources.playerAmounts.size())
		{
			world.resources.playerAmounts[foodResource] -= totalCostFood;
		}

		const Vec2 firstTarget = validTargets.front();
		const Point firstCell = WorldToBattleCell(world, firstTarget);
		Point fallbackDirection{ 1, 0 };
		if (validTargets.size() >= 2)
		{
			const Point secondCell = WorldToBattleCell(world, validTargets[1]);
			fallbackDirection = Point{ secondCell.x - firstCell.x, secondCell.y - firstCell.y };
		}

		world.buildQueues.pendingEntry[builder] = QueuedBuildAction{
			actionId,
			firstTarget,
			true,
			ResolveLinePlacementIconAt(world, action, validTargets, 0, fallbackDirection)
		};
		world.buildQueues.hasPendingEntry[builder] = true;

		Array<QueuedBuildAction>& queue = world.buildQueues.entries[builder];
		const bool wasEmpty = queue.isEmpty();
		for (size_t i = 1; i < validTargets.size(); ++i)
		{
			queue << QueuedBuildAction{
				actionId,
				validTargets[i],
				true,
				ResolveLinePlacementIconAt(world, action, validTargets, i, fallbackDirection)
			};
		}

		SetBuildQueueLocked(world, builder, true);
		if (wasEmpty)
		{
			IssueMove(world, builder, ResolveBuildApproachPosition(world, firstTarget));
			ResetBuildQueueProgress(world, builder);
		}
		return true;
	}
}
