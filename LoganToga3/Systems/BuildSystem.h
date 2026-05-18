# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline UnitDefId ResolvePrimarySpawnUnit(const BuildActionDef& action, const DefinitionStores& defs)
    {
        if (action.spawnUnit != InvalidUnitDefId)
        {
            return action.spawnUnit;
        }

        for (const auto spawnUnit : action.spawnUnits)
        {
            if (spawnUnit < defs.units.size())
            {
                return spawnUnit;
            }
        }

        for (const auto& spawnTag : action.spawnTags)
        {
            if (defs.unitByTag.contains(spawnTag))
            {
                return defs.unitByTag.at(spawnTag);
            }
        }

        if (!action.spawnTag.isEmpty() && defs.unitByTag.contains(action.spawnTag))
        {
            return defs.unitByTag.at(action.spawnTag);
        }

        return InvalidUnitDefId;
    }

    inline void UpdateBuildQueues(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        const UnitId unitCount = static_cast<UnitId>(world.units.size());
        for (UnitId unit = 0; unit < unitCount; ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;

            if (unit < world.buildQueues.hasPendingEntry.size() && world.buildQueues.hasPendingEntry[unit])
            {
                const Vec2 pendingTarget = world.buildQueues.pendingEntry[unit].targetPosition;
                const bool nearPendingBuildTarget = world.units.position[unit].distanceFromSq(pendingTarget) <= Square(QuarterTileStep * 1.1);
                if (world.units.task[unit] == UnitTask::Idle || nearPendingBuildTarget)
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
            const UnitDefId primarySpawnUnit = ResolvePrimarySpawnUnit(action, defs);
            world.buildQueues.progressSec[unit] += dt;
            if (world.buildQueues.progressSec[unit] >= action.buildTimeSec)
            {
                const Vec2 completionTarget = queuedAction.hasTargetPosition ? queuedAction.targetPosition : world.units.position[unit];
                const bool requiresPlacementValidation = queuedAction.hasTargetPosition
                    && (action.resultType == BuildActionResultType::Object
                        || (action.resultType == BuildActionResultType::Unit
                            && primarySpawnUnit < defs.units.size()
                            && (defs.units[primarySpawnUnit].role == UnitRole::Base
                                || defs.units[primarySpawnUnit].role == UnitRole::Barrier
                                || defs.units[primarySpawnUnit].blocksTileMovement)));
                if (requiresPlacementValidation
                    && EvaluateBuildPlacementCell(world, defs, completionTarget) != BuildPlacementCellState::Allowed)
                {
                    queue.remove_at(0);
                    world.buildQueues.progressSec[unit] = 0.0;
                    if (queue.isEmpty())
                    {
                        world.units.task[unit] = UnitTask::Idle;
                        if (unit < world.buildQueues.locked.size())
                        {
                            world.buildQueues.locked[unit] = false;
                        }
                    }
                    continue;
                }

                queue.remove_at(0);
                world.buildQueues.progressSec[unit] = 0.0;

                if (action.resultType == BuildActionResultType::Unit && primarySpawnUnit != InvalidUnitDefId)
                {
                    const Vec2 spawnOrigin = completionTarget;
                  const bool isStaticPlacement = primarySpawnUnit < defs.units.size()
                        && queuedAction.hasTargetPosition
                        && (defs.units[primarySpawnUnit].role == UnitRole::Base
                            || defs.units[primarySpawnUnit].role == UnitRole::Barrier
                            || defs.units[primarySpawnUnit].blocksTileMovement);
                    if (isStaticPlacement)
                    {
                        AddUnitToBattleWorld(world, primarySpawnUnit, world.units.faction[unit], spawnOrigin, defs, queuedAction.iconOverride);
                    }
                    else
                    {
                        for (int32 count = 0; count < action.createCount; ++count)
                        {
                            const Vec2 rally = spawnOrigin + Vec2{ 74 + count * 18.0, Random(-48.0, 48.0) };
                            AddUnitToBattleWorld(world, primarySpawnUnit, world.units.faction[unit], rally, defs, queuedAction.iconOverride);
                        }
                    }
                }
                else if (action.resultType == BuildActionResultType::Object)
                {
                    const Vec2 placedPos = completionTarget;
                    AddPlacedObjectToBattleWorld(world, placedPos, action.resultTag, action.icon);
                }

                if (action.resultType == BuildActionResultType::Unit && primarySpawnUnit < defs.units.size())
                {
                    const UnitDef& spawnedDef = defs.units[primarySpawnUnit];
                    if (spawnedDef.blocksTileMovement)
                    {
                        const Point cell = WorldToBattleCell(world, queuedAction.hasTargetPosition ? queuedAction.targetPosition : world.units.position[unit]);
                        world.map.setBarrierReservation(cell.y, cell.x, true);
                        ++world.map.revision;
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
