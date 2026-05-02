# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"

namespace LT3
{
    inline void UpdateEnemyDirector(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
     const UnitId unitCount = static_cast<UnitId>(world.units.size());

        world.enemySpawnTimerSec += dt;
        if (world.enemySpawnTimerSec >= 8.0)
        {
            world.enemySpawnTimerSec = 0.0;
            const UnitDefId spawn = RandomBool(0.55) ? defs.unitByTag.at(U"soldier") : defs.unitByTag.at(U"archer");
            AddUnitToBattleWorld(world, spawn, Faction::Enemy, Vec2{ 1325, Random(350.0, 550.0) }, defs);
        }

        for (UnitId unit = 0; unit < unitCount; ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.faction[unit] != Faction::Enemy) continue;
            if (defs.units[world.units.defId[unit]].role == UnitRole::Base) continue;

            const UnitId target = FindNearestEnemy(world, unit, 620.0);
            if (IsValidUnit(world, target))
            {
                const SkillDef& skill = defs.skills[defs.units[world.units.defId[unit]].skill];
                if (world.units.position[unit].distanceFrom(world.units.position[target]) > skill.range * 0.85)
                {
                    IssueMove(world, unit, world.units.position[target]);
                }
                world.units.attackTarget[unit] = target;
            }
        }
    }
}
