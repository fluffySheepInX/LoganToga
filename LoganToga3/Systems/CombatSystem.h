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

            const SkillDef& skill = defs.skills[unitDef.skill];
            const UnitId target = ResolveAttackTarget(world, defs, unit);
            if (!IsValidUnit(world, target)) continue;

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
