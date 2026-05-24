# pragma once
# include <Siv3D.hpp>
# include "ProjectileSystem.h"
# include "TargetingSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline void UpdateCombat(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (IsBuildQueueLocked(world, unit)) continue;

            world.cooldowns.attackLeftSec[unit] = Max(0.0, world.cooldowns.attackLeftSec[unit] - dt);
            const UnitDef& unitDef = defs.units[world.units.defId[unit]];
            if (unitDef.skill == InvalidSkillDefId) continue;
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

            const SkillDef& skill = defs.skills[unitDef.skill];
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

            const int32 burstCount = Max(1, skill.burstCount);
            for (int32 burstIndex = 0; burstIndex < burstCount; ++burstIndex)
            {
                SpawnProjectile(world, defs, unit, target, unitDef.skill, burstIndex);
            }
            world.cooldowns.attackLeftSec[unit] = skill.cooldownSec;
        }
    }
}
