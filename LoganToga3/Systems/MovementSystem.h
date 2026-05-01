# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateMovement(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.task[unit] != UnitTask::Moving && world.units.task[unit] != UnitTask::Gathering) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (def.speed <= 0.0) continue;

            const Vec2 toTarget = world.units.targetPosition[unit] - world.units.position[unit];
            const double distance = toTarget.length();
            if (distance <= Max(3.0, def.speed * dt))
            {
                world.units.position[unit] = world.units.targetPosition[unit];
                world.units.task[unit] = UnitTask::Idle;
                continue;
            }

            world.units.position[unit] += toTarget.normalized() * def.speed * dt;
        }
    }
}
