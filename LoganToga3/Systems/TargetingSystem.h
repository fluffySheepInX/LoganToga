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

    inline double ResolveEffectiveAttackRangeMin(const UnitDef& unitDef, const SkillDef& skill)
    {
        return Min(skill.rangeMin, ResolveEffectiveAttackRange(unitDef, skill));
    }

    inline bool IsWithinEffectiveAttackBand(const UnitDef& attackerDef, const SkillDef& skill, double distance)
    {
        const double maxRange = ResolveEffectiveAttackRange(attackerDef, skill);
        const double minRange = ResolveEffectiveAttackRangeMin(attackerDef, skill);
        return (minRange <= distance) && (distance <= maxRange);
    }

    inline Array<SkillDefId> ResolveUnitSkillIds(const UnitDef& unitDef, const DefinitionStores& defs)
    {
        Array<SkillDefId> skillIds;
        for (const SkillDefId skillId : unitDef.skills)
        {
            if (skillId != InvalidSkillDefId && skillId < defs.skills.size() && !skillIds.contains(skillId))
            {
                skillIds << skillId;
            }
        }

        if (skillIds.isEmpty() && unitDef.skill != InvalidSkillDefId && unitDef.skill < defs.skills.size())
        {
            skillIds << unitDef.skill;
        }

        skillIds.sort();
        return skillIds;
    }

    inline bool UnitDefHasSkill(const UnitDef& unitDef, const DefinitionStores& defs, SkillDefId skillId)
    {
        return ResolveUnitSkillIds(unitDef, defs).contains(skillId);
    }

    inline bool DoesSkillTargetAllies(const SkillDef& skill)
    {
        return skill.kind == SkillKind::Heal;
    }

    inline bool DoesSkillConsumeResources(const SkillDef& skill)
    {
        return !skill.resourceCosts.isEmpty();
    }

    inline bool DoesSkillMatchBattleFilter(const SkillDef& skill, BattleSkillFilterKind filter)
    {
        switch (filter)
        {
        case BattleSkillFilterKind::Heal:
            return DoesSkillTargetAllies(skill);
        case BattleSkillFilterKind::ResourceCost:
            return DoesSkillConsumeResources(skill);
        default:
            return true;
        }
    }

    inline bool IsSkillTargetFactionMatch(const BattleWorld& world, UnitId caster, UnitId other, const SkillDef& skill)
    {
        if (!IsValidUnit(world, caster) || !IsValidUnit(world, other) || caster == other)
        {
            return false;
        }

        const bool targetAllies = DoesSkillTargetAllies(skill);
        const bool sameFaction = (world.units.faction[caster] == world.units.faction[other]);
        return targetAllies ? sameFaction : IsEnemy(world.units.faction[caster], world.units.faction[other]);
    }

    inline bool IsValidSkillTarget(const BattleWorld& world, const DefinitionStores& defs, UnitId caster, UnitId target, const SkillDef& skill)
    {
        if (!IsSkillTargetFactionMatch(world, caster, target, skill))
        {
            return false;
        }

        const UnitDef& casterDef = defs.units[world.units.defId[caster]];
        const double distance = world.units.position[caster].distanceFrom(world.units.position[target]);
        return IsWithinEffectiveAttackBand(casterDef, skill, distance);
    }

    inline Array<UnitId> CollectSkillTargets(const BattleWorld& world, const DefinitionStores& defs, UnitId caster, const SkillDef& skill)
    {
        Array<UnitId> targets;
        if (!IsValidUnit(world, caster))
        {
            return targets;
        }

        const UnitDef& casterDef = defs.units[world.units.defId[caster]];
        const double maxRange = ResolveEffectiveAttackRange(casterDef, skill);
        const double maxRangeSq = maxRange * maxRange;
        const Vec2 casterPos = world.units.position[caster];
        for (UnitId other = 0; other < world.units.size(); ++other)
        {
            if (!IsSkillTargetFactionMatch(world, caster, other, skill))
            {
                continue;
            }

            const double distanceSq = casterPos.distanceFromSq(world.units.position[other]);
            if (distanceSq > maxRangeSq)
            {
                continue;
            }

            const double distance = Sqrt(distanceSq);
            if (!IsWithinEffectiveAttackBand(casterDef, skill, distance))
            {
                continue;
            }

            targets << other;
        }

        return targets;
    }

    inline UnitId FindNearestSkillTarget(const BattleWorld& world, const DefinitionStores& defs, UnitId caster, const SkillDef& skill)
    {
        if (!IsValidUnit(world, caster))
        {
            return InvalidUnitId;
        }

        const Vec2 casterPos = world.units.position[caster];
        UnitId best = InvalidUnitId;
        double bestDistanceSq = Math::Inf;
        for (UnitId other = 0; other < world.units.size(); ++other)
        {
            if (!IsValidSkillTarget(world, defs, caster, other, skill))
            {
                continue;
            }

            const double distanceSq = casterPos.distanceFromSq(world.units.position[other]);
            if (distanceSq < bestDistanceSq)
            {
                bestDistanceSq = distanceSq;
                best = other;
            }
        }

        return best;
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
        UnitId target = world.units.attackTarget[unit];
        if (!IsValidSkillTarget(world, defs, unit, target, skill))
        {
            target = FindNearestSkillTarget(world, defs, unit, skill);
            SetUnitAttackTarget(world, unit, target);
        }

        return target;
    }

    inline UnitId ResolveAttackTargetForSkill(BattleWorld& world, const DefinitionStores& defs, UnitId unit, SkillDefId skillId, bool updateStoredTarget)
    {
        if (!IsValidUnit(world, unit) || skillId == InvalidSkillDefId || skillId >= defs.skills.size())
        {
            return InvalidUnitId;
        }

        const SkillDef& skill = defs.skills[skillId];
        UnitId target = world.units.attackTarget[unit];
        if (!IsValidSkillTarget(world, defs, unit, target, skill))
        {
            target = FindNearestSkillTarget(world, defs, unit, skill);
        }

        if (updateStoredTarget)
        {
            SetUnitAttackTarget(world, unit, target);
        }

        return target;
    }

    inline SkillDefId ResolveExecutableAttackSkill(BattleWorld& world, const DefinitionStores& defs, UnitId unit, UnitId& target)
    {
        target = InvalidUnitId;
        if (!IsValidUnit(world, unit) || world.units.defId[unit] >= defs.units.size())
        {
            return InvalidSkillDefId;
        }

        const UnitDef& unitDef = defs.units[world.units.defId[unit]];
        if (world.selection.selectedUnits.contains(unit)
            && world.selection.selectedSkill != InvalidSkillDefId
            && UnitDefHasSkill(unitDef, defs, world.selection.selectedSkill))
        {
            const UnitId selectedSkillTarget = ResolveAttackTargetForSkill(world, defs, unit, world.selection.selectedSkill, false);
            if (IsValidUnit(world, selectedSkillTarget))
            {
                target = selectedSkillTarget;
                SetUnitAttackTarget(world, unit, target);
                return world.selection.selectedSkill;
            }
        }

        for (const SkillDefId skillId : ResolveUnitSkillIds(unitDef, defs))
        {
            const UnitId skillTarget = ResolveAttackTargetForSkill(world, defs, unit, skillId, false);
            if (IsValidUnit(world, skillTarget))
            {
                target = skillTarget;
                SetUnitAttackTarget(world, unit, target);
                return skillId;
            }
        }

        ClearUnitAttackTarget(world, unit);
        return InvalidSkillDefId;
    }
}
