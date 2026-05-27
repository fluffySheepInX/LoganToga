# pragma once
# include <Siv3D.hpp>
# include "ProjectileSystem.h"
# include "TargetingSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline void FirePendingBurstShots(BattleWorld& world, const DefinitionStores& defs, UnitId unit, const SkillDef& skill, SkillDefId skillId, double dt)
    {
        if (unit >= world.cooldowns.burstShotsLeft.size())
        {
            return;
        }

        if (world.cooldowns.burstShotsLeft[unit] <= 0)
        {
            return;
        }

        world.cooldowns.burstShotTimerSec[unit] -= dt;
        while (world.cooldowns.burstShotsLeft[unit] > 0 && world.cooldowns.burstShotTimerSec[unit] <= 0.0)
        {
            UnitId burstTarget = world.cooldowns.burstTarget[unit];
            if (!IsValidUnit(world, burstTarget))
            {
                burstTarget = ResolveAttackTarget(world, defs, unit);
            }
            if (!IsValidUnit(world, burstTarget))
            {
                world.cooldowns.burstShotsLeft[unit] = 0;
                world.cooldowns.burstShotTimerSec[unit] = 0.0;
                world.cooldowns.burstTarget[unit] = InvalidUnitId;
                break;
            }

            const int32 burstCount = Max(1, skill.burstCount);
            const int32 shotIndex = burstCount - world.cooldowns.burstShotsLeft[unit];
            const int32 burstIndex = (unit < world.cooldowns.burstOrder.size()
                && shotIndex < static_cast<int32>(world.cooldowns.burstOrder[unit].size()))
                ? world.cooldowns.burstOrder[unit][shotIndex]
                : shotIndex;
            SpawnProjectile(world, defs, unit, burstTarget, skillId, burstIndex);
            --world.cooldowns.burstShotsLeft[unit];
            world.cooldowns.burstTarget[unit] = burstTarget;
            world.cooldowns.burstShotTimerSec[unit] += Max(0.0, skill.burstIntervalSec);
            if (skill.burstIntervalSec <= 0.0)
            {
                world.cooldowns.burstShotTimerSec[unit] = 0.0;
            }
        }

        if (world.cooldowns.burstShotsLeft[unit] <= 0)
        {
            world.cooldowns.burstShotsLeft[unit] = 0;
            world.cooldowns.burstShotTimerSec[unit] = 0.0;
            world.cooldowns.burstTarget[unit] = InvalidUnitId;
            if (unit < world.cooldowns.burstOrder.size())
            {
                world.cooldowns.burstOrder[unit].clear();
            }
        }
    }

    inline void UpdateCombat(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (IsBuildQueueLocked(world, unit)) continue;

            world.cooldowns.attackLeftSec[unit] = Max(0.0, world.cooldowns.attackLeftSec[unit] - dt);
            const UnitDef& unitDef = defs.units[world.units.defId[unit]];
            if (unitDef.skill == InvalidSkillDefId) continue;
            const SkillDef& skill = defs.skills[unitDef.skill];
            FirePendingBurstShots(world, defs, unit, skill, unitDef.skill, dt);
            if (HasUnitFormationFinalTarget(world, unit)) continue;
            if (world.units.task[unit] == UnitTask::Moving
                && unit < world.units.ignoreCombatWhileMoving.size()
                && world.units.ignoreCombatWhileMoving[unit])
            {
                const double remainingMoveDistance = world.units.position[unit].distanceFrom(world.units.targetPosition[unit]);
                if (remainingMoveDistance > Max(8.0, unitDef.speed * dt * 2.0))
                {
                    continue;
                }
            }

            const UnitId target = ResolveAttackTarget(world, defs, unit);
            if (!IsValidUnit(world, target))
            {
                if (world.units.task[unit] == UnitTask::Attacking
                    && world.units.position[unit].distanceFromSq(world.units.targetPosition[unit]) > Square(12.0))
                {
                    SetUnitMoving(world, unit, world.units.targetPosition[unit], true, false);
                    EnqueuePathRequest(world, unit, world.units.targetPosition[unit]);
                }
                continue;
            }

            const UnitDef& targetDef = defs.units[world.units.defId[target]];
            const double stopDistance = ResolveAttackStopDistance(unitDef, targetDef, skill);
            const double distanceToTarget = world.units.position[unit].distanceFrom(world.units.position[target]);
            if (distanceToTarget > stopDistance)
            {
                const Vec2 approach = ResolveAttackApproachDestination(world.units.position[unit], world.units.position[target], stopDistance);
                SetUnitTargetPosition(world, unit, approach);
                SetUnitTask(world, unit, UnitTask::Moving);
                SetUnitIgnoreCombatWhileMoving(world, unit, false);
                EnqueuePathRequest(world, unit, approach);
                continue;
            }

            SetUnitAttacking(world, unit);
            if (world.cooldowns.attackLeftSec[unit] > 0.0) continue;
            if (unit < world.cooldowns.burstShotsLeft.size() && world.cooldowns.burstShotsLeft[unit] > 0) continue;
            if (!TryConsumeSkillResourceCosts(world, defs, world.units.faction[unit], skill))
            {
                if (unit < world.cooldowns.skillCastFailureDisplayLeftSec.size())
                {
                    world.cooldowns.skillCastFailureDisplayLeftSec[unit] = 0.6;
                }
                continue;
            }

            const int32 burstCount = Max(1, skill.burstCount);
            if (unit < world.cooldowns.burstOrder.size())
            {
                Array<int32>& order = world.cooldowns.burstOrder[unit];
                order.clear();
                order.reserve(burstCount);
                for (int32 i = 0; i < burstCount; ++i)
                {
                    order << i;
                }
                if (skill.burstOrderMode == SkillBurstOrderMode::Random)
                {
                    order.shuffle();
                }
            }
            if (skill.burstFireMode == SkillBurstFireMode::Simultaneous)
            {
                for (int32 shotIndex = 0; shotIndex < burstCount; ++shotIndex)
                {
                    const int32 burstIndex = (unit < world.cooldowns.burstOrder.size()
                        && shotIndex < static_cast<int32>(world.cooldowns.burstOrder[unit].size()))
                        ? world.cooldowns.burstOrder[unit][shotIndex]
                        : shotIndex;
                    SpawnProjectile(world, defs, unit, target, unitDef.skill, burstIndex);
                }
                if (unit < world.cooldowns.burstShotsLeft.size())
                {
                    world.cooldowns.burstShotsLeft[unit] = 0;
                    world.cooldowns.burstShotTimerSec[unit] = 0.0;
                    world.cooldowns.burstTarget[unit] = InvalidUnitId;
                    if (unit < world.cooldowns.burstOrder.size())
                    {
                        world.cooldowns.burstOrder[unit].clear();
                    }
                }
            }
            else
            {
                const int32 firstBurstIndex = (unit < world.cooldowns.burstOrder.size()
                    && !world.cooldowns.burstOrder[unit].isEmpty())
                    ? world.cooldowns.burstOrder[unit].front()
                    : 0;
                SpawnProjectile(world, defs, unit, target, unitDef.skill, firstBurstIndex);
                if (unit < world.cooldowns.burstShotsLeft.size())
                {
                    world.cooldowns.burstShotsLeft[unit] = burstCount - 1;
                    world.cooldowns.burstShotTimerSec[unit] = Max(0.0, skill.burstIntervalSec);
                    world.cooldowns.burstTarget[unit] = target;
                    if (world.cooldowns.burstShotsLeft[unit] <= 0 && unit < world.cooldowns.burstOrder.size())
                    {
                        world.cooldowns.burstOrder[unit].clear();
                    }
                }
            }
            world.cooldowns.attackLeftSec[unit] = skill.cooldownSec;
        }

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (unit >= world.cooldowns.skillCastFailureDisplayLeftSec.size())
            {
                break;
            }
            world.cooldowns.skillCastFailureDisplayLeftSec[unit] = Max(0.0, world.cooldowns.skillCastFailureDisplayLeftSec[unit] - dt);
        }
    }
}
