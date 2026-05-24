# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline double ResolveEffectiveAttackRange(const UnitDef& unitDef, const SkillDef& skill)
    {
        if (skill.projectileMotion == SkillProjectileMotion::Swing)
        {
            const double swingLength = Max(1.0, skill.projectileHeight);
            const double swingReach = (skill.projectileCenter == SkillProjectileCenter::End)
                ? (skill.swingRadius + swingLength)
                : ((skill.swingRadius > 0.0)
                    ? skill.swingRadius
                    : (swingLength * 0.5));
            return Min(skill.range, unitDef.radius + swingReach + 8.0);
        }

        return skill.range;
    }

    inline UnitId ResolveAttackTarget(BattleWorld& world, const DefinitionStores& defs, UnitId unit)
    {
        if (!IsValidUnit(world, unit))
        {
            return InvalidUnitId;
        }

        const UnitDef& unitDef = defs.units[world.units.defId[unit]];
        if (unitDef.skill == InvalidSkillDefId)
        {
            ClearUnitAttackTarget(world, unit);
            return InvalidUnitId;
        }

        const SkillDef& skill = defs.skills[unitDef.skill];
        const double attackRange = ResolveEffectiveAttackRange(unitDef, skill);
        UnitId target = world.units.attackTarget[unit];
        if (!IsValidUnit(world, target)
            || !IsEnemy(world.units.faction[unit], world.units.faction[target])
            || world.units.position[unit].distanceFrom(world.units.position[target]) > attackRange)
        {
            target = FindNearestEnemy(world, unit, attackRange);
            SetUnitAttackTarget(world, unit, target);
        }

        return target;
    }
}
