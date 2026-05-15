# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"
# include "PathfindingSystem.h"

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
                    world.units.task[unit] = UnitTask::Idle;
                    world.units.targetPosition[unit] = world.units.position[unit];
                    world.units.resourceTargetNode[unit] = -1;
                    world.pathing.clearUnitPath(unit);
                }
                continue;
            }

            const bool hasPathData = (unit < world.pathing.hasPath.size() && unit < world.pathing.pathMapRevision.size());
            const bool pathOutdated = hasPathData
                && world.pathing.hasPath[unit]
                && (world.pathing.pathMapRevision[unit] != world.map.revision);
            if (pathOutdated)
            {
                world.pathing.clearUnitPath(unit);
            }

            const bool needsPath = (unit < world.pathing.hasPath.size())
                && !world.pathing.hasPath[unit]
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
                    world.pathing.clearUnitPath(unit);
                    if (world.units.resourceTargetNode[unit] >= 0)
                    {
                        world.units.task[unit] = UnitTask::Gathering;
                    }
                    else
                    {
                        world.units.task[unit] = UnitTask::Idle;
                    }
                }

                continue;
            }

            const bool reachedFallbackTarget = MoveUnitToward(world, unit, def, world.units.targetPosition[unit], dt);
            if (reachedFallbackTarget)
            {
                if (world.units.resourceTargetNode[unit] >= 0)
                {
                    world.units.task[unit] = UnitTask::Gathering;
                }
                else
                {
                    world.units.task[unit] = UnitTask::Idle;
                }
            }
        }
    }
}
