# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateBuildQueues(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.task[unit] != UnitTask::Building) continue;

            const BuildActionDefId actionId = world.units.buildAction[unit];
            if (actionId >= defs.buildActions.size())
            {
                world.units.task[unit] = UnitTask::Idle;
                continue;
            }

            const BuildActionDef& action = defs.buildActions[actionId];
            world.units.buildProgressSec[unit] += dt;
            if (world.units.buildProgressSec[unit] >= action.buildTimeSec)
            {
                const Vec2 rally = world.units.position[unit] + Vec2{ 74, Random(-48.0, 48.0) };
                world.units.add(action.spawnUnit, world.units.faction[unit], rally, defs);
                world.units.task[unit] = UnitTask::Idle;
                world.units.buildAction[unit] = InvalidBuildActionDefId;
                world.units.buildProgressSec[unit] = 0.0;
            }
        }
    }
}
