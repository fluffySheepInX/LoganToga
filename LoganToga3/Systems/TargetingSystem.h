# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline UnitId ResolveAttackTarget(BattleWorld& world, const DefinitionStores& defs, UnitId unit)
    {
        if (!IsValidUnit(world, unit))
        {
            return InvalidUnitId;
        }

        const UnitDef& unitDef = defs.units[world.units.defId[unit]];
        if (unitDef.skill == InvalidSkillDefId)
        {
            world.units.attackTarget[unit] = InvalidUnitId;
            return InvalidUnitId;
        }

        const SkillDef& skill = defs.skills[unitDef.skill];
        UnitId target = world.units.attackTarget[unit];
        if (!IsValidUnit(world, target)
            || !IsEnemy(world.units.faction[unit], world.units.faction[target])
            || world.units.position[unit].distanceFrom(world.units.position[target]) > skill.range)
        {
            target = FindNearestEnemy(world, unit, skill.range);
            world.units.attackTarget[unit] = target;
        }

        return target;
    }
}
