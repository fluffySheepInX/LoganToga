# pragma once
# include <Siv3D.hpp>
# include "DamageSystem.h"

namespace LT3
{
    inline void SpawnProjectile(BattleWorld& world, UnitId attacker, UnitId target, SkillDefId skillId)
    {
        const Vec2 start = world.units.position[attacker];
        const Vec2 toTarget = world.units.position[target] - start;
        const Vec2 velocity = toTarget.normalized() * 380.0;
        world.projectiles.add(start, velocity, target, world.units.faction[attacker], skillId);
    }

    inline void UpdateProjectiles(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (size_t i = 0; i < world.projectiles.position.size();)
        {
            world.projectiles.lifeSec[i] -= dt;
            const UnitId target = world.projectiles.target[i];
            if (world.projectiles.lifeSec[i] <= 0.0 || !IsValidUnit(world, target))
            {
                world.projectiles.removeAt(i);
                continue;
            }

            const Vec2 toTarget = world.units.position[target] - world.projectiles.position[i];
            if (toTarget.length() > 1.0)
            {
                world.projectiles.velocity[i] = world.projectiles.velocity[i].lerp(toTarget.normalized() * 380.0, 0.08);
            }

            world.projectiles.position[i] += world.projectiles.velocity[i] * dt;
            const UnitDef& targetDef = defs.units[world.units.defId[target]];
            if (world.projectiles.position[i].distanceFrom(world.units.position[target]) <= targetDef.radius + 5.0)
            {
                ApplyProjectileHit(world, defs, i, target);
                world.projectiles.removeAt(i);
                continue;
            }

            ++i;
        }
    }
}
