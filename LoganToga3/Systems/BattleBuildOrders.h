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

	inline BuildPlacementCellState EvaluateBuildPlacementCell(const BattleWorld& world, const Point& cell)
	{
		if (!world.map.inBounds(cell.y, cell.x))
		{
			return BuildPlacementCellState::OutOfBounds;
		}

		for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
		{
			const Point resourceCell = WorldPositionToBattleCell(world, world.resourceNodes.position[node]);
			if (resourceCell == cell)
			{
				return BuildPlacementCellState::BlockedByResource;
			}
		}

		if (world.map.hasBarrierReservation(cell.y, cell.x))
		{
			return BuildPlacementCellState::BlockedByOccupancy;
		}

		for (size_t i = 0; i < world.placedObjects.position.size(); ++i)
		{
			const Point objectCell = WorldPositionToBattleCell(world, world.placedObjects.position[i]);
			if (objectCell == cell)
			{
				return BuildPlacementCellState::BlockedByOccupancy;
			}
		}

		return BuildPlacementCellState::Allowed;
	}

	inline BuildPlacementCellState EvaluateBuildPlacementCell(const BattleWorld& world, const Vec2& worldPosition)
	{
		return EvaluateBuildPlacementCell(world, WorldPositionToBattleCell(world, worldPosition));
	}

	inline Vec2 ResolveBuildApproachPosition(const BattleWorld& world, const Vec2& buildTarget)
	{
		const Point targetCell = WorldPositionToBattleCell(world, buildTarget);
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
		const Point start = WorldPositionToBattleCell(world, startWorld);
		Point end = WorldPositionToBattleCell(world, currentWorld);

		const int32 dx = end.x - start.x;
		const int32 dy = end.y - start.y;
		const bool horizontal = (action.lineAxisMode == BuildLineAxisMode::HorizontalOnly)
			|| (action.lineAxisMode == BuildLineAxisMode::Auto && Abs(dx) >= Abs(dy));

		if (horizontal)
		{
			end.y = start.y;
		}
		else
		{
			end.x = start.x;
		}

		Array<Vec2> targets;
		HashSet<Point> uniqueCells;
		const int32 stepX = (end.x == start.x) ? 0 : ((end.x > start.x) ? 1 : -1);
		const int32 stepY = (end.y == start.y) ? 0 : ((end.y > start.y) ? 1 : -1);
		const int32 count = Min(action.maxLineCells, Max(1, Max(Abs(end.x - start.x), Abs(end.y - start.y)) + 1));
		const int32 thickness = Max(1, action.lineThicknessCells);
		const int32 halfThickness = thickness / 2;
		targets.reserve(count * thickness);

		const int32 offsetStepX = (stepX == 0) ? 1 : 0;
		const int32 offsetStepY = (stepY == 0) ? 1 : 0;

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

	inline bool TryStartBuild(BattleWorld& world, const DefinitionStores& defs, UnitId builder, BuildActionDefId actionId, const Optional<Vec2>& targetPosition = none)
	{
		if (!CanStartBuildAction(world, defs, builder, actionId)) return false;

		const BuildActionDef& action = defs.buildActions[actionId];
		if (IsBuilderBusyWithBuildQueue(world, builder))
		{
			return false;
		}

		if (action.resultType == BuildActionResultType::Carrier)
		{
			if (!TryExecuteCarrierAction(world, defs, builder, action))
			{
				return false;
			}
			if (!world.resources.playerAmounts.isEmpty())
			{
				world.resources.playerAmounts[0] -= action.costGold;
			}
			return true;
		}

		if (!world.resources.playerAmounts.isEmpty())
		{
			world.resources.playerAmounts[0] -= action.costGold;
		}

		const Optional<Vec2> resolvedTargetPosition = targetPosition.has_value()
			? Optional<Vec2>{ SnapWorldPositionToBattleCellCenter(world, *targetPosition) }
			: none;

		if (resolvedTargetPosition && IsBuildingStyleBuildAction(defs, action))
		{
			world.buildQueues.pendingEntry[builder] = QueuedBuildAction{ actionId, *resolvedTargetPosition, true };
			world.buildQueues.hasPendingEntry[builder] = true;
			world.buildQueues.locked[builder] = true;
			world.buildQueues.progressSec[builder] = 0.0;
			world.units.attackTarget[builder] = InvalidUnitId;
			IssueMove(world, builder, ResolveBuildApproachPosition(world, *resolvedTargetPosition));
			return true;
		}

		Array<QueuedBuildAction>& queue = world.buildQueues.entries[builder];
		const bool wasEmpty = queue.isEmpty();
		queue << QueuedBuildAction{ actionId, resolvedTargetPosition.value_or(Vec2{ 0, 0 }), resolvedTargetPosition.has_value() };
		world.buildQueues.locked[builder] = true;
		if (wasEmpty)
		{
			world.units.task[builder] = UnitTask::Building;
			world.buildQueues.progressSec[builder] = 0.0;
		}
		return true;
	}

	inline bool TryStartBuildLine(BattleWorld& world, const DefinitionStores& defs, UnitId builder, BuildActionDefId actionId, const Array<Vec2>& targetPositions)
	{
		if (!CanStartBuildAction(world, defs, builder, actionId)) return false;
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
			const Vec2 snappedTarget = SnapWorldPositionToBattleCellCenter(world, target);
			if (EvaluateBuildPlacementCell(world, snappedTarget) == BuildPlacementCellState::Allowed)
			{
				validTargets << snappedTarget;
			}
		}

		if (validTargets.isEmpty())
		{
			return false;
		}

		const int32 totalCostGold = action.costGold * static_cast<int32>(validTargets.size());
		if (!world.resources.playerAmounts.isEmpty() && world.resources.playerAmounts[0] < totalCostGold)
		{
			return false;
		}

		if (!world.resources.playerAmounts.isEmpty())
		{
			world.resources.playerAmounts[0] -= totalCostGold;
		}

		const Vec2 firstTarget = validTargets.front();
		world.buildQueues.pendingEntry[builder] = QueuedBuildAction{ actionId, firstTarget, true };
		world.buildQueues.hasPendingEntry[builder] = true;

		Array<QueuedBuildAction>& queue = world.buildQueues.entries[builder];
		const bool wasEmpty = queue.isEmpty();
		for (size_t i = 1; i < validTargets.size(); ++i)
		{
			queue << QueuedBuildAction{ actionId, validTargets[i], true };
		}

		world.buildQueues.locked[builder] = true;
		if (wasEmpty)
		{
			IssueMove(world, builder, ResolveBuildApproachPosition(world, firstTarget));
			world.buildQueues.progressSec[builder] = 0.0;
		}
		return true;
	}
}
