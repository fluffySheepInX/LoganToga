# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"
# include "PathfindingSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline bool MoveUnitToward(BattleWorld& world, UnitId unit, const UnitDef& def, const Vec2& destination, double dt)
    {
        const Vec2 toTarget = destination - world.units.position[unit];
        const double distance = toTarget.length();
        if (distance <= Max(3.0, def.speed * dt))
        {
            world.units.position[unit] = destination;
            return true;
        }

        world.units.position[unit] += toTarget.normalized() * def.speed * dt;
        return false;
    }

    inline bool CanMoveDirectlyToPosition(const BattleWorld& world, const Vec2& position)
    {
        const Point cell = PathWorldToBattleCell(world, position);
        return IsPathCellPassable(world, cell);
    }

    inline bool MoveUnitTowardFormationFinalTarget(BattleWorld& world, UnitId unit, const UnitDef& def, double dt)
    {
        const Vec2 destination = GetUnitFormationFinalTarget(world, unit);
        SetUnitTargetPosition(world, unit, destination);

        const Vec2 toTarget = destination - world.units.position[unit];
        const double distance = toTarget.length();
        if (distance <= Max(3.0, def.speed * dt))
        {
            if (CanMoveDirectlyToPosition(world, destination))
            {
                world.units.position[unit] = destination;
            }
            ClearUnitFormationFinalTarget(world, unit);
            SetUnitTargetPosition(world, unit, world.units.position[unit]);
            return true;
        }

        const Vec2 nextPosition = world.units.position[unit] + (toTarget.normalized() * def.speed * dt);
        if (!CanMoveDirectlyToPosition(world, nextPosition))
        {
            ClearUnitFormationFinalTarget(world, unit);
            SetUnitTargetPosition(world, unit, world.units.position[unit]);
            return true;
        }

        world.units.position[unit] = nextPosition;
        return false;
    }

    inline bool TryContinueFormationFinalMove(BattleWorld& world, UnitId unit, const UnitDef& def, double dt)
    {
        if (!HasUnitFormationFinalTarget(world, unit))
        {
            return false;
        }

        if (unit < world.pathing.requestPending.size() && world.pathing.requestPending[unit])
        {
            return true;
        }

        const bool reachedFinalTarget = MoveUnitTowardFormationFinalTarget(world, unit, def, dt);
        if (reachedFinalTarget)
        {
            SetUnitIdle(world, unit);
        }
        else
        {
            SetUnitTask(world, unit, UnitTask::Moving);
        }
        return true;
    }

    inline void UpdateMovement(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.task[unit] != UnitTask::Moving && world.units.task[unit] != UnitTask::Gathering) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (def.speed <= 0.0)
            {
                if (world.units.task[unit] == UnitTask::Moving || world.units.task[unit] == UnitTask::Gathering)
                {
                    ClearUnitFormationFinalTarget(world, unit);
                    SetUnitIdle(world, unit);
                    SetUnitTargetPosition(world, unit, world.units.position[unit]);
                    ClearUnitResourceTarget(world, unit);
                    ClearUnitPath(world, unit);
                }
                continue;
            }

            const bool hasPathData = (unit < world.pathing.hasPath.size() && unit < world.pathing.pathMapRevision.size());
            const bool pathOutdated = hasPathData
                && world.pathing.hasPath[unit]
                && (world.pathing.pathMapRevision[unit] != world.map.revision);
            if (pathOutdated)
            {
                ClearUnitPath(world, unit);
            }

            const bool needsPath = (unit < world.pathing.hasPath.size())
                && !world.pathing.hasPath[unit]
                && !HasUnitFormationFinalTarget(world, unit)
                && world.units.position[unit].distanceFromSq(world.units.targetPosition[unit]) > Square(12.0);
            if (needsPath)
            {
                EnqueuePathRequest(world, unit, world.units.targetPosition[unit]);
            }

            if (unit < world.pathing.hasPath.size() && world.pathing.hasPath[unit] && unit < world.pathing.waypoints.size())
            {
                Array<Vec2>& waypoints = world.pathing.waypoints[unit];
                int32& waypointIndex = world.pathing.waypointIndex[unit];
                waypointIndex = Clamp(waypointIndex, 0, static_cast<int32>(waypoints.size()));

                bool reachedPathEnd = false;
                while (waypointIndex < static_cast<int32>(waypoints.size()))
                {
                    const bool reachedWaypoint = MoveUnitToward(world, unit, def, waypoints[waypointIndex], dt);
                    if (!reachedWaypoint)
                    {
                        break;
                    }

                    ++waypointIndex;
                    if (waypointIndex >= static_cast<int32>(waypoints.size()))
                    {
                        reachedPathEnd = true;
                    }
                }

                if (reachedPathEnd)
                {
                    ClearUnitPath(world, unit);
                    if (TryContinueFormationFinalMove(world, unit, def, dt))
                    {
                        continue;
                    }
                    if (world.units.resourceTargetNode[unit] >= 0)
                    {
                        SetUnitGathering(world, unit);
                    }
                    else
                    {
                        SetUnitIdle(world, unit);
                    }
                }

                continue;
            }

            if (TryContinueFormationFinalMove(world, unit, def, dt))
            {
                continue;
            }

            const bool reachedFallbackTarget = MoveUnitToward(world, unit, def, world.units.targetPosition[unit], dt);
            if (reachedFallbackTarget)
            {
                if (world.units.resourceTargetNode[unit] >= 0)
                {
                    SetUnitGathering(world, unit);
                }
                else
                {
                    SetUnitIdle(world, unit);
                }
            }
        }
    }
}
