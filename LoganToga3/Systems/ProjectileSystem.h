# pragma once
# include <Siv3D.hpp>
# include "DamageSystem.h"

namespace LT3
{
    inline Vec2 RotateVector(const Vec2& value, double angleRad)
    {
        const double c = Cos(angleRad);
        const double s = Sin(angleRad);
        return Vec2{ value.x * c - value.y * s, value.x * s + value.y * c };
    }

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId, int32 burstIndex)
    {
        if (skillId >= defs.skills.size() || !IsValidUnit(world, attacker) || !IsValidUnit(world, target))
        {
            return;
        }

        const SkillDef& skill = defs.skills[skillId];
        const Vec2 start = world.units.position[attacker];
        const Vec2 targetPos = world.units.position[target];
        const Vec2 toTarget = targetPos - start;
        const Vec2 baseDir = (toTarget.lengthSq() > 1.0) ? toTarget.normalized() : Vec2{ 1.0, 0.0 };
        const double centeredIndex = static_cast<double>(burstIndex) - (Max(1, skill.burstCount) - 1) * 0.5;
        const double spreadRad = Math::ToRadians(skill.spreadDeg * centeredIndex / Max(1, skill.burstCount));
        const Vec2 dir = RotateVector(baseDir, spreadRad);
        const double distance = Max(1.0, toTarget.length());
        const double maxLife = (skill.projectileMotion == SkillProjectileMotion::Orbit)
            ? skill.orbitDurationSec
            : Max(0.15, distance / Max(1.0, skill.projectileSpeed));
        const Vec2 velocity = dir * skill.projectileSpeed;
        const Vec2 spawnPos = (skill.projectileMotion == SkillProjectileMotion::Orbit)
            ? start + Vec2{ skill.orbitRadius, 0.0 }
            : start;
        world.projectiles.add(spawnPos, velocity, start, targetPos, target, attacker, world.units.faction[attacker], skillId, skill.projectileMotion, maxLife, spreadRad);
    }

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId)
    {
        SpawnProjectile(world, defs, attacker, target, skillId, 0);
    }

    inline void UpdateProjectiles(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (size_t i = 0; i < world.projectiles.position.size();)
        {
            world.projectiles.lifeSec[i] -= dt;
            world.projectiles.ageSec[i] += dt;
            const UnitId target = world.projectiles.target[i];
            if (world.projectiles.lifeSec[i] <= 0.0 || !IsValidUnit(world, target))
            {
                world.projectiles.removeAt(i);
                continue;
            }

            const SkillDef& skill = defs.skills[world.projectiles.skill[i]];
            if (world.projectiles.motion[i] == SkillProjectileMotion::Orbit)
            {
                const UnitId owner = world.projectiles.owner[i];
                if (!IsValidUnit(world, owner))
                {
                    world.projectiles.removeAt(i);
                    continue;
                }

                world.projectiles.angleRad[i] += Math::ToRadians(skill.orbitAngularSpeedDeg) * dt;
                const Vec2 center = world.units.position[owner];
                world.projectiles.position[i] = center + Vec2{ Cos(world.projectiles.angleRad[i]), Sin(world.projectiles.angleRad[i]) } * skill.orbitRadius;
                world.projectiles.height[i] = 0.0;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Parabola)
            {
                const double t = Clamp(world.projectiles.ageSec[i] / Max(0.05, world.projectiles.maxLifeSec[i]), 0.0, 1.0);
                world.projectiles.endPosition[i] = world.units.position[target];
                world.projectiles.position[i] = world.projectiles.startPosition[i].lerp(world.projectiles.endPosition[i], t);
                world.projectiles.height[i] = Sin(t * Math::Pi) * skill.arcHeight;
            }
            else
            {
                const Vec2 toTarget = world.units.position[target] - world.projectiles.position[i];
                if (toTarget.length() > 1.0)
                {
                    world.projectiles.velocity[i] = world.projectiles.velocity[i].lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
                }

                world.projectiles.position[i] += world.projectiles.velocity[i] * dt;
                world.projectiles.height[i] = 0.0;
            }

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
