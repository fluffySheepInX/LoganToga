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
            Array<BuildActionDefId>& queue = world.buildQueues.actionIds[unit];
            if (queue.isEmpty())
            {
                if (world.units.task[unit] == UnitTask::Building)
                {
                    world.units.task[unit] = UnitTask::Idle;
                    world.buildQueues.progressSec[unit] = 0.0;
                }
                continue;
            }

            if (world.units.task[unit] != UnitTask::Building)
            {
                world.units.task[unit] = UnitTask::Building;
            }

            const BuildActionDefId actionId = world.buildQueues.actionIds[unit].front();
            if (actionId >= defs.buildActions.size())
            {
                world.buildQueues.actionIds[unit].remove_at(0);
                world.buildQueues.progressSec[unit] = 0.0;
                if (world.buildQueues.actionIds[unit].isEmpty())
                {
                    world.units.task[unit] = UnitTask::Idle;
                }
                continue;
            }

            const BuildActionDef& action = defs.buildActions[actionId];
            world.buildQueues.progressSec[unit] += dt;
            if (world.buildQueues.progressSec[unit] >= action.buildTimeSec)
            {
                world.buildQueues.actionIds[unit].remove_at(0);
                world.buildQueues.progressSec[unit] = 0.0;

                for (int32 count = 0; count < action.createCount; ++count)
                {
                    const Vec2 rally = world.units.position[unit] + Vec2{ 74 + count * 18.0, Random(-48.0, 48.0) };
                    AddUnitToBattleWorld(world, action.spawnUnit, world.units.faction[unit], rally, defs);
                }

                if (world.buildQueues.actionIds[unit].isEmpty())
                {
                    world.units.task[unit] = UnitTask::Idle;
                }
            }
        }
    }
}
