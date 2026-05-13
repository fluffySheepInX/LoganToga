# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateBuildQueues(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        const UnitId unitCount = static_cast<UnitId>(world.units.size());
        for (UnitId unit = 0; unit < unitCount; ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;

            if (unit < world.buildQueues.hasPendingEntry.size() && world.buildQueues.hasPendingEntry[unit])
            {
                if (world.units.task[unit] == UnitTask::Idle || world.units.position[unit].distanceFromSq(world.units.targetPosition[unit]) <= Square(9.0))
                {
                    Array<QueuedBuildAction>& queue = world.buildQueues.entries[unit];
                    const bool wasEmpty = queue.isEmpty();
                    queue << world.buildQueues.pendingEntry[unit];
                    world.buildQueues.hasPendingEntry[unit] = false;
                    world.buildQueues.pendingEntry[unit] = QueuedBuildAction{};
                    if (wasEmpty)
                    {
                        world.units.task[unit] = UnitTask::Building;
                        world.buildQueues.progressSec[unit] = 0.0;
                    }
                }
                else
                {
                    continue;
                }
            }

            Array<QueuedBuildAction>& queue = world.buildQueues.entries[unit];
            if (queue.isEmpty())
            {
                if (world.units.task[unit] == UnitTask::Building)
                {
                    world.units.task[unit] = UnitTask::Idle;
                    world.buildQueues.progressSec[unit] = 0.0;
                }
                if (unit < world.buildQueues.locked.size())
                {
                    world.buildQueues.locked[unit] = false;
                }
                continue;
            }

            if (world.units.task[unit] != UnitTask::Building)
            {
                world.units.task[unit] = UnitTask::Building;
            }

            const QueuedBuildAction queuedAction = queue.front();
            const BuildActionDefId actionId = queuedAction.actionId;
            if (actionId >= defs.buildActions.size())
            {
                queue.remove_at(0);
                world.buildQueues.progressSec[unit] = 0.0;
                if (queue.isEmpty())
                {
                    world.units.task[unit] = UnitTask::Idle;
                }
                continue;
            }

            const BuildActionDef& action = defs.buildActions[actionId];
            world.buildQueues.progressSec[unit] += dt;
            if (world.buildQueues.progressSec[unit] >= action.buildTimeSec)
            {
                queue.remove_at(0);
                world.buildQueues.progressSec[unit] = 0.0;

               if (action.resultType == BuildActionResultType::Unit && action.spawnUnit != InvalidUnitDefId)
                {
                    const Vec2 spawnOrigin = queuedAction.hasTargetPosition ? queuedAction.targetPosition : world.units.position[unit];
                  const bool isStaticPlacement = action.spawnUnit < defs.units.size()
                        && queuedAction.hasTargetPosition
                        && (defs.units[action.spawnUnit].role == UnitRole::Base
                            || defs.units[action.spawnUnit].role == UnitRole::Barrier
                            || defs.units[action.spawnUnit].blocksTileMovement);
                    if (isStaticPlacement)
                    {
                        AddUnitToBattleWorld(world, action.spawnUnit, world.units.faction[unit], spawnOrigin, defs);
                    }
                    else
                    {
                        for (int32 count = 0; count < action.createCount; ++count)
                        {
                            const Vec2 rally = spawnOrigin + Vec2{ 74 + count * 18.0, Random(-48.0, 48.0) };
                            AddUnitToBattleWorld(world, action.spawnUnit, world.units.faction[unit], rally, defs);
                        }
                    }
                }
                else if (action.resultType == BuildActionResultType::Object)
                {
                    const Vec2 placedPos = queuedAction.hasTargetPosition ? queuedAction.targetPosition : world.units.position[unit];
                    AddPlacedObjectToBattleWorld(world, placedPos, action.resultTag, action.icon);
                }

                if (action.resultType == BuildActionResultType::Unit && action.spawnUnit < defs.units.size())
                {
                    const UnitDef& spawnedDef = defs.units[action.spawnUnit];
                    if (spawnedDef.blocksTileMovement)
                    {
                        const Point cell = WorldPositionToBattleCell(world, queuedAction.hasTargetPosition ? queuedAction.targetPosition : world.units.position[unit]);
                        world.map.setBarrierReservation(cell.y, cell.x, true);
                    }
                }

                if (queue.isEmpty())
                {
                    world.units.task[unit] = UnitTask::Idle;
                    if (unit < world.buildQueues.locked.size())
                    {
                        world.buildQueues.locked[unit] = false;
                    }
                }
            }
        }
    }
}
