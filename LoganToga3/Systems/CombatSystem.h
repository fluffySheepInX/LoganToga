# pragma once
# include <Siv3D.hpp>
# include "ProjectileSystem.h"
# include "TargetingSystem.h"

namespace LT3
{
    inline void UpdateCombat(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;

            world.cooldowns.attackLeftSec[unit] = Max(0.0, world.cooldowns.attackLeftSec[unit] - dt);
            const UnitDef& unitDef = defs.units[world.units.defId[unit]];
            if (unitDef.skill == InvalidSkillDefId) continue;

            const SkillDef& skill = defs.skills[unitDef.skill];
            const UnitId target = ResolveAttackTarget(world, defs, unit);
            if (!IsValidUnit(world, target)) continue;

            world.units.task[unit] = UnitTask::Attacking;
            if (world.cooldowns.attackLeftSec[unit] > 0.0) continue;

            SpawnProjectile(world, unit, target, unitDef.skill);
            world.cooldowns.attackLeftSec[unit] = skill.cooldownSec;
        }
    }
}
