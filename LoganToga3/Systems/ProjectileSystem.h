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

    inline double ResolveProjectileStartAngleRad(const SkillDef& skill, const Vec2& dir)
    {
        const double targetAngle = Math::Atan2(dir.y, dir.x);
        double startDegree = skill.projectileStartDegree;
        if (skill.projectileStartDegreeType == 6 && dir.x >= 0.0)
        {
            startDegree = -startDegree;
        }
        return targetAngle + Math::ToRadians(startDegree);
    }

    inline Vec2 ResolveProjectileSpawnPosition(const SkillDef& skill, const Vec2& ownerPos, const Vec2& dir)
    {
        if (skill.projectileCenter == SkillProjectileCenter::End)
        {
            return ownerPos + dir * (skill.range * 0.5);
        }
        return ownerPos;
    }

    inline double ResolveProjectileMaxLife(const SkillDef& skill, double distance)
    {
        switch (skill.projectileMotion)
        {
        case SkillProjectileMotion::Static:
            return skill.orbitDurationSec;
        case SkillProjectileMotion::Drop:
            return Max(0.05, Max(1.0, skill.arcHeight) / Max(1.0, Abs(skill.projectileSpeed)));
        case SkillProjectileMotion::Orbit:
            return skill.orbitDurationSec;
        case SkillProjectileMotion::Swing:
            return Max(0.05, Abs(skill.swingAngleDeg) / Max(1.0, Abs(skill.projectileSpeed)));
        default:
            return Max(0.15, distance / Max(1.0, Abs(skill.projectileSpeed)));
        }
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
        const double maxLife = ResolveProjectileMaxLife(skill, distance);
        const Vec2 velocity = (skill.projectileMotion == SkillProjectileMotion::Static) ? Vec2{ 0.0, 0.0 } : (dir * skill.projectileSpeed);
        Vec2 spawnPos = ResolveProjectileSpawnPosition(skill, start, dir);
        const double startAngleRad = ResolveProjectileStartAngleRad(skill, dir);
        if (skill.projectileMotion == SkillProjectileMotion::Orbit || skill.projectileMotion == SkillProjectileMotion::Swing)
        {
            const double radius = (skill.projectileMotion == SkillProjectileMotion::Swing) ? skill.swingRadius : skill.orbitRadius;
            spawnPos = start + Vec2{ Cos(startAngleRad), Sin(startAngleRad) } * radius;
        }
        else if (skill.projectileMotion == SkillProjectileMotion::Drop)
        {
            spawnPos = targetPos;
        }
        world.projectiles.add(spawnPos, velocity, start, targetPos, target, attacker, world.units.faction[attacker], skillId, skill.projectileMotion, maxLife, startAngleRad);
        if (skill.projectileMotion == SkillProjectileMotion::Drop && !world.projectiles.height.isEmpty())
        {
            world.projectiles.height.back() = (skill.projectileSpeed >= 0.0) ? Max(1.0, skill.arcHeight) : 0.0;
        }
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
            if (skill.projectileHoming && IsValidUnit(world, world.projectiles.owner[i]))
            {
                const Vec2 ownerDelta = world.units.position[world.projectiles.owner[i]] - world.projectiles.startPosition[i];
                world.projectiles.startPosition[i] += ownerDelta;
                world.projectiles.endPosition[i] += ownerDelta;
                if (world.projectiles.motion[i] == SkillProjectileMotion::Static)
                {
                    world.projectiles.position[i] += ownerDelta;
                }
            }

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
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Swing)
            {
                const UnitId owner = world.projectiles.owner[i];
                if (!IsValidUnit(world, owner))
                {
                    world.projectiles.removeAt(i);
                    continue;
                }

                const double sign = (skill.swingAngleDeg < 0.0) ? -1.0 : 1.0;
                const double progressDeg = Min(Abs(skill.swingAngleDeg), world.projectiles.ageSec[i] * Abs(skill.projectileSpeed));
                world.projectiles.angleRad[i] = world.projectiles.baseAngleRad[i] + Math::ToRadians(progressDeg * sign);
                const Vec2 center = world.units.position[owner];
                world.projectiles.position[i] = center + Vec2{ Cos(world.projectiles.angleRad[i]), Sin(world.projectiles.angleRad[i]) } * skill.swingRadius;
                world.projectiles.height[i] = 0.0;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Arc)
            {
                const double t = Clamp(world.projectiles.ageSec[i] / Max(0.05, world.projectiles.maxLifeSec[i]), 0.0, 1.0);
                world.projectiles.endPosition[i] = world.units.position[target];
                const Vec2 linePos = world.projectiles.startPosition[i].lerp(world.projectiles.endPosition[i], t);
                const Vec2 delta = world.projectiles.endPosition[i] - world.projectiles.startPosition[i];
                const Vec2 normal = (delta.lengthSq() > 1.0) ? Vec2{ -delta.y, delta.x }.normalized() : Vec2{ 0.0, -1.0 };
                world.projectiles.position[i] = linePos + normal * (Sin(t * Math::Pi) * skill.arcHeight);
                world.projectiles.height[i] = 0.0;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Parabola)
            {
                const double t = Clamp(world.projectiles.ageSec[i] / Max(0.05, world.projectiles.maxLifeSec[i]), 0.0, 1.0);
                world.projectiles.endPosition[i] = world.units.position[target];
                world.projectiles.position[i] = world.projectiles.startPosition[i].lerp(world.projectiles.endPosition[i], t);
                world.projectiles.height[i] = Sin(t * Math::Pi) * skill.arcHeight;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Drop)
            {
                world.projectiles.endPosition[i] = world.units.position[target];
                world.projectiles.position[i] = world.projectiles.endPosition[i];
                world.projectiles.height[i] = (skill.projectileSpeed >= 0.0)
                    ? Max(0.0, Max(1.0, skill.arcHeight) - Abs(skill.projectileSpeed) * world.projectiles.ageSec[i])
                    : Abs(skill.projectileSpeed) * world.projectiles.ageSec[i];
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Static)
            {
                world.projectiles.height[i] = 0.0;
            }
            else
            {
                if (skill.projectileHoming)
                {
                    const Vec2 toTarget = world.units.position[target] - world.projectiles.position[i];
                    if (toTarget.length() > 1.0)
                    {
                        world.projectiles.velocity[i] = world.projectiles.velocity[i].lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
                    }
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
