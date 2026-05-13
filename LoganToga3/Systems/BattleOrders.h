# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "../UI/QuarterView.h"

namespace LT3
{
    inline void RemoveStoredUnitFromAllCarriers(BattleWorld& world, UnitId unit)
    {
        for (auto& stored : world.carriers.storedUnits)
        {
            stored.remove(unit);
        }
    }

    inline int32 StoreNearbyUnitsInCarrier(BattleWorld& world, const DefinitionStores& defs, UnitId carrier)
    {
        if (!IsValidUnit(world, carrier) || carrier >= world.carriers.storedUnits.size())
        {
            return 0;
        }

        Array<UnitId>& stored = world.carriers.storedUnits[carrier];
        const Vec2 carrierPos = world.units.position[carrier];
        const Faction faction = world.units.faction[carrier];
        int32 storedCount = 0;

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (unit == carrier || !IsValidUnit(world, unit))
            {
                continue;
            }
            if (world.units.faction[unit] != faction)
            {
                continue;
            }

            const UnitDef& unitDef = defs.units[world.units.defId[unit]];
            if (unitDef.role == UnitRole::Base)
            {
                continue;
            }
            if (carrierPos.distanceFromSq(world.units.position[unit]) > Square(84.0))
            {
                continue;
            }

            RemoveStoredUnitFromAllCarriers(world, unit);
            stored << unit;
            world.units.alive[unit] = false;
            world.units.task[unit] = UnitTask::Idle;
            world.units.targetPosition[unit] = carrierPos;
            world.units.attackTarget[unit] = InvalidUnitId;
            if (world.selection.selected == unit || world.selection.selectedUnits.contains(unit))
            {
                ClearSelection(world);
            }
            ++storedCount;
        }

        return storedCount;
    }

    inline bool ReleaseStoredUnitsFromCarrier(BattleWorld& world, UnitId carrier)
    {
        if (!IsValidUnit(world, carrier) || carrier >= world.carriers.storedUnits.size())
        {
            return false;
        }

        Array<UnitId>& stored = world.carriers.storedUnits[carrier];
        if (stored.isEmpty())
        {
            return false;
        }

        const Vec2 carrierPos = world.units.position[carrier];
        for (size_t i = 0; i < stored.size(); ++i)
        {
            const UnitId unit = stored[i];
            if (unit >= world.units.size())
            {
                continue;
            }

            const double angle = (Math::TwoPi * static_cast<double>(i)) / Max(1.0, static_cast<double>(stored.size()));
            const Vec2 offset = Circular{ 42.0 + 10.0 * static_cast<double>(i / 6), angle };
            world.units.alive[unit] = true;
            world.units.position[unit] = carrierPos + offset;
            world.units.targetPosition[unit] = carrierPos + offset;
            world.units.task[unit] = UnitTask::Idle;
            world.units.attackTarget[unit] = InvalidUnitId;
        }

        stored.clear();
        return true;
    }

    inline bool TryExecuteCarrierAction(BattleWorld& world, const DefinitionStores& defs, UnitId builder, const BuildActionDef& action)
    {
        const String actionId = action.id.isEmpty() ? action.tag.lowercased() : action.id.lowercased();
        const String category = action.category.lowercased();
        if (category == U"releaseall" || actionId.includes(U"releaseall"))
        {
            return ReleaseStoredUnitsFromCarrier(world, builder);
        }

        StoreNearbyUnitsInCarrier(world, defs, builder);
        return true;
    }

    inline Optional<UnitId> PickUnitAt(const BattleWorld& world, const DefinitionStores& defs, const Vec2& pos, Faction faction)
    {
        for (int32 i = static_cast<int32>(world.units.size()) - 1; i >= 0; --i)
        {
            const UnitId unit = static_cast<UnitId>(i);
            if (!IsValidUnit(world, unit) || world.units.faction[unit] != faction) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (Circle{ world.units.position[unit], UnitSelectionRadius(def) + 6.0 }.intersects(pos))
            {
                return unit;
            }
        }

        return none;
    }

    inline void IssueMove(BattleWorld& world, UnitId unit, const Vec2& destination)
    {
        if (!IsValidUnit(world, unit)) return;
        world.units.targetPosition[unit] = destination;
        world.units.attackTarget[unit] = InvalidUnitId;
        world.units.resourceTargetNode[unit] = -1;
        world.units.task[unit] = UnitTask::Moving;
    }

    inline void IssueMoveToResourceNode(BattleWorld& world, UnitId unit, size_t nodeIndex)
    {
        if (!IsValidUnit(world, unit) || nodeIndex >= world.resourceNodes.position.size())
        {
            return;
        }

        world.units.targetPosition[unit] = world.resourceNodes.position[nodeIndex];
        world.units.attackTarget[unit] = InvalidUnitId;
        world.units.resourceTargetNode[unit] = static_cast<int32>(nodeIndex);
        world.units.task[unit] = UnitTask::Moving;
    }

    inline bool IsBuildQueueLocked(const BattleWorld& world, UnitId unit)
    {
        return IsValidUnit(world, unit)
            && unit < world.buildQueues.locked.size()
            && world.buildQueues.locked[unit];
    }

    inline bool HasPendingBuildQueueEntry(const BattleWorld& world, UnitId unit)
    {
        return IsValidUnit(world, unit)
            && unit < world.buildQueues.hasPendingEntry.size()
            && world.buildQueues.hasPendingEntry[unit];
    }

    inline bool IsBuilderBusyWithBuildQueue(const BattleWorld& world, UnitId unit)
    {
        return IsBuildQueueLocked(world, unit)
            || HasPendingBuildQueueEntry(world, unit)
            || (!GetQueuedBuildActionEntries(world, unit).isEmpty());
    }

    inline Vec2 SnapWorldPositionToBattleCellCenter(const BattleWorld& world, const Vec2& position)
    {
        const int32 col = Clamp(static_cast<int32>(Math::Round(position.x / QuarterTileStep)), 0, Max(0, world.mapWidth - 1));
        const int32 row = Clamp(static_cast<int32>(Math::Round(position.y / QuarterTileStep)), 0, Max(0, world.mapHeight - 1));
        return Vec2{ col * QuarterTileStep, row * QuarterTileStep };
    }

    inline Point WorldPositionToBattleCell(const BattleWorld& world, const Vec2& position)
    {
        const int32 col = Clamp(static_cast<int32>(Math::Round(position.x / QuarterTileStep)), 0, Max(0, world.mapWidth - 1));
        const int32 row = Clamp(static_cast<int32>(Math::Round(position.y / QuarterTileStep)), 0, Max(0, world.mapHeight - 1));
        return Point{ col, row };
    }

    inline Vec2 BattleCellToWorldPosition(const Point& cell)
    {
        return Vec2{ cell.x * QuarterTileStep, cell.y * QuarterTileStep };
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

    inline Vec2 ComputeUnitGroupCenter(const BattleWorld& world, const Array<UnitId>& units)
    {
        Vec2 center{ 0, 0 };
        int32 count = 0;
        for (const UnitId unit : units)
        {
            if (!IsValidUnit(world, unit))
            {
                continue;
            }

            center += world.units.position[unit];
            ++count;
        }

        return (count > 0) ? (center / static_cast<double>(count)) : Vec2{ 0, 0 };
    }

    inline Vec2 ResolveFormationFacingDirection(const BattleWorld& world, const Array<UnitId>& units, const Vec2& destination, const Vec2& current)
    {
        Vec2 facing = current - destination;
        if (facing.lengthSq() < 16.0)
        {
            facing = destination - ComputeUnitGroupCenter(world, units);
        }
        if (facing.lengthSq() < 0.0001)
        {
            facing = Vec2{ 1.0, 0.0 };
        }
        return facing.normalized();
    }

    inline Array<Vec2> BuildFormationMoveTargets(const BattleWorld& world, const DefinitionStores& defs, const Array<UnitId>& units, const Vec2& destination, const Vec2& facingDirection)
    {
        Array<UnitId> validUnits;
        double maxRadius = 0.0;
        for (const UnitId unit : units)
        {
            if (!IsValidUnit(world, unit))
            {
                continue;
            }

            validUnits << unit;
            maxRadius = Max(maxRadius, defs.units[world.units.defId[unit]].radius);
        }

        Array<Vec2> targets;
        if (validUnits.isEmpty())
        {
            return targets;
        }

        const Vec2 forward = (facingDirection.lengthSq() < 0.0001)
            ? Vec2{ 1.0, 0.0 }
            : facingDirection.normalized();
        const Vec2 lateral{ -forward.y, forward.x };
        const double spacing = Max(32.0, maxRadius * 2.8);
        const int32 columns = Max(1, static_cast<int32>(Math::Ceil(Sqrt(static_cast<double>(validUnits.size())))));

        targets.reserve(validUnits.size());
        size_t startIndex = 0;
        int32 row = 0;
        while (startIndex < validUnits.size())
        {
            const int32 rowCount = Min(columns, static_cast<int32>(validUnits.size() - startIndex));
            const double rowCenter = (static_cast<double>(rowCount) - 1.0) * 0.5;
            for (int32 column = 0; column < rowCount; ++column)
            {
                const double sideOffset = (static_cast<double>(column) - rowCenter) * spacing;
                const double forwardOffset = -static_cast<double>(row) * spacing;
                targets << (destination + (lateral * sideOffset) + (forward * forwardOffset));
            }

            startIndex += rowCount;
            ++row;
        }

        return targets;
    }

    inline void IssueFormationMove(BattleWorld& world, const DefinitionStores& defs, const Array<UnitId>& units, const Vec2& destination, const Vec2& facingDirection)
    {
        Array<UnitId> validUnits;
        for (const UnitId unit : units)
        {
            if (IsValidUnit(world, unit))
            {
                validUnits << unit;
            }
        }

        if (validUnits.isEmpty())
        {
            return;
        }

        const Array<Vec2> targets = BuildFormationMoveTargets(world, defs, validUnits, destination, facingDirection);
        for (size_t i = 0; i < validUnits.size() && i < targets.size(); ++i)
        {
            IssueMove(world, validUnits[i], targets[i]);
        }
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
            IssueMove(world, builder, *resolvedTargetPosition);
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

        const int32 totalCostGold = action.costGold * static_cast<int32>(targetPositions.size());
        if (!world.resources.playerAmounts.isEmpty() && world.resources.playerAmounts[0] < totalCostGold)
        {
            return false;
        }

        if (!world.resources.playerAmounts.isEmpty())
        {
            world.resources.playerAmounts[0] -= totalCostGold;
        }

        Array<QueuedBuildAction>& queue = world.buildQueues.entries[builder];
        const bool wasEmpty = queue.isEmpty();
        for (const Vec2& targetPosition : targetPositions)
        {
            queue << QueuedBuildAction{ actionId, SnapWorldPositionToBattleCellCenter(world, targetPosition), true };
        }

        world.buildQueues.locked[builder] = true;
        if (wasEmpty)
        {
            world.units.task[builder] = UnitTask::Building;
            world.buildQueues.progressSec[builder] = 0.0;
        }
        return true;
    }
}
