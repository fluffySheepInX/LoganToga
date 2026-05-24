# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline bool IsSwingEndSkill(const SkillDef& skill)
    {
        return skill.projectileMotion == SkillProjectileMotion::Swing
            && skill.projectileCenter == SkillProjectileCenter::End;
    }

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
            return Min(skill.range, swingReach);
        }

        return skill.range;
    }

    inline double ResolveAttackStopDistance(const UnitDef& attackerDef, const UnitDef& targetDef, const SkillDef& skill)
    {
        if (IsSwingEndSkill(skill))
        {
            const double swingLength = Max(1.0, skill.projectileHeight);
            const double reach = skill.swingRadius + swingLength;
            const double hitRadius = targetDef.radius + 5.0;
            const double conservativeStop = reach - hitRadius;
            return Clamp(conservativeStop, 10.0, reach);
        }

        return ResolveEffectiveAttackRange(attackerDef, skill);
    }

    inline Vec2 ResolveAttackApproachDestination(const Vec2& attackerPos, const Vec2& targetPos, double stopDistance)
    {
        Vec2 away = attackerPos - targetPos;
        if (away.lengthSq() < 1.0)
        {
            away = Vec2{ -1.0, 0.0 };
        }

        return targetPos + away.normalized() * Max(0.0, stopDistance);
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
