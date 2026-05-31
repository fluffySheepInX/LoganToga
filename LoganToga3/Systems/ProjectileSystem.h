# pragma once
# include <Siv3D.hpp>
# include "DamageSystem.h"
# include "TargetingSystem.h"

namespace LT3
{
    inline constexpr int32 MaxSkillNextChainDepth = 12;

    struct SkillNextSpawnContext
    {
        Optional<Vec2> targetPosition;
        Optional<double> imageAngleRad;
        Optional<Vec2> originPosition;
        int32 chainDepth = 0;
    };

    inline UnitId FindNearestSkillTargetFromOrigin(const BattleWorld& world, const DefinitionStores& defs, UnitId caster, const SkillDef& skill, const Vec2& originPos)
    {
        if (!IsValidUnit(world, caster))
        {
            return InvalidUnitId;
        }

        const UnitDef& casterDef = defs.units[world.units.defId[caster]];
        const double maxRange = ResolveEffectiveAttackRange(casterDef, skill);
        const double minRange = ResolveEffectiveAttackRangeMin(casterDef, skill);
        UnitId best = InvalidUnitId;
        double bestDistanceSq = Math::Inf;
        for (UnitId other = 0; other < world.units.size(); ++other)
        {
            if (!IsSkillTargetFactionMatch(world, caster, other, skill))
            {
                continue;
            }

            const double distanceSq = originPos.distanceFromSq(world.units.position[other]);
            const double distance = Sqrt(distanceSq);
            if (distance < minRange || distance > maxRange)
            {
                continue;
            }

            if (distanceSq < bestDistanceSq)
            {
                bestDistanceSq = distanceSq;
                best = other;
            }
        }

        return best;
    }

    inline Vec2 ResolveSwingEndProjectileTipWorld(const SkillDef& skill, const Vec2& rootPos, double angleRad)
    {
        const double length = Max(1.0, skill.projectileHeight);
        return rootPos + Vec2{ Cos(angleRad), Sin(angleRad) } * length;
    }

    inline bool IntersectsCircleWithSegment(const Vec2& point, double radius, const Vec2& segmentStart, const Vec2& segmentEnd)
    {
        const Vec2 segment = segmentEnd - segmentStart;
        const double segmentLengthSq = segment.lengthSq();
        if (segmentLengthSq <= 0.0001)
        {
            return point.distanceFrom(segmentStart) <= radius;
        }

        const double t = Clamp(((point - segmentStart).dot(segment)) / segmentLengthSq, 0.0, 1.0);
        const Vec2 closest = segmentStart + segment * t;
        return point.distanceFrom(closest) <= radius;
    }

    inline bool IsSwingEndProjectileHit(const SkillDef& skill, const Vec2& rootPos, double angleRad, const Vec2& targetPos, double hitRadius)
    {
        if (skill.projectileMotion != SkillProjectileMotion::Swing || skill.projectileCenter != SkillProjectileCenter::End)
        {
            return false;
        }

        const Vec2 tipPos = ResolveSwingEndProjectileTipWorld(skill, rootPos, angleRad);
        return IntersectsCircleWithSegment(targetPos, hitRadius, rootPos, tipPos);
    }

    inline bool CanSwingProjectileHitTarget(const BattleWorld& world, size_t projectileIndex, UnitId target, const SkillDef& skill)
    {
        if (skill.projectileMotion != SkillProjectileMotion::Swing || projectileIndex >= world.projectiles.swingHitUnits.size())
        {
            return true;
        }

        if (skill.swingHitMode == SkillSwingHitMode::MultiHitOnce)
        {
            return !world.projectiles.swingHitUnits[projectileIndex].contains(target);
        }

        return true;
    }

    inline Array<UnitId> CollectSwingProjectileHitTargets(const BattleWorld& world, const DefinitionStores& defs, size_t projectileIndex, const SkillDef& skill)
    {
        Array<UnitId> targets;
        if (skill.projectileMotion != SkillProjectileMotion::Swing || projectileIndex >= world.projectiles.position.size())
        {
            return targets;
        }

        const UnitId owner = world.projectiles.owner[projectileIndex];
        if (!IsValidUnit(world, owner))
        {
            return targets;
        }

        for (UnitId other = 0; other < world.units.size(); ++other)
        {
            if (!IsSkillTargetFactionMatch(world, owner, other, skill))
            {
                continue;
            }
            if (!CanSwingProjectileHitTarget(world, projectileIndex, other, skill))
            {
                continue;
            }

            const UnitDef& targetDef = defs.units[world.units.defId[other]];
            const double hitRadius = targetDef.radius + 5.0;
            const Vec2 targetPos = world.units.position[other];
            if (IsSwingEndProjectileHit(skill, world.projectiles.position[projectileIndex], world.projectiles.angleRad[projectileIndex], targetPos, hitRadius)
                || (world.projectiles.position[projectileIndex].distanceFrom(targetPos) <= hitRadius))
            {
                targets << other;
            }
        }

        return targets;
    }

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

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId, int32 burstIndex, const SkillNextSpawnContext& nextContext);

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId, int32 burstIndex)
    {
        SpawnProjectile(world, defs, attacker, target, skillId, burstIndex, SkillNextSpawnContext{});
    }

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId, int32 burstIndex, const SkillNextSpawnContext& nextContext)
    {
        if (skillId >= defs.skills.size() || !IsValidUnit(world, attacker) || (!IsValidUnit(world, target) && !nextContext.targetPosition.has_value()))
        {
            return;
        }

        const SkillDef& skill = defs.skills[skillId];
        const Vec2 start = nextContext.originPosition.value_or(world.units.position[attacker]);
        const Vec2 targetPos = nextContext.targetPosition.value_or(world.units.position[target]);
        const Vec2 toTarget = targetPos - start;
        const Vec2 baseDir = (toTarget.lengthSq() > 1.0) ? toTarget.normalized() : Vec2{ 1.0, 0.0 };
        const double centeredIndex = static_cast<double>(burstIndex) - (Max(1, skill.burstCount) - 1) * 0.5;
        const double spreadRad = Math::ToRadians(skill.spreadDeg * centeredIndex / Max(1, skill.burstCount));
        const Vec2 dir = RotateVector(baseDir, spreadRad);
        const double distance = Max(1.0, toTarget.length());
        const double maxLife = ResolveProjectileMaxLife(skill, distance);
        const Vec2 velocity = (skill.projectileMotion == SkillProjectileMotion::Static) ? Vec2{ 0.0, 0.0 } : (dir * skill.projectileSpeed);
        Vec2 spawnPos = ResolveProjectileSpawnPosition(skill, start, dir);
        const double startAngleRad = nextContext.imageAngleRad.value_or(ResolveProjectileStartAngleRad(skill, dir));
        if (skill.projectileMotion == SkillProjectileMotion::Orbit || skill.projectileMotion == SkillProjectileMotion::Swing)
        {
            const double radius = (skill.projectileMotion == SkillProjectileMotion::Swing) ? skill.swingRadius : skill.orbitRadius;
            spawnPos = start + Vec2{ Cos(startAngleRad), Sin(startAngleRad) } * radius;
        }
        else if (skill.projectileMotion == SkillProjectileMotion::Drop)
        {
            spawnPos = targetPos;
        }
        PlayBattleSkillSoundIfRelevant(skill, spawnPos);
        world.projectiles.add(spawnPos, velocity, start, targetPos, target, attacker, world.units.faction[attacker], skillId, skill.projectileMotion, maxLife, startAngleRad, targetPos, nextContext.imageAngleRad.has_value(), startAngleRad, nextContext.chainDepth);
        if (skill.projectileMotion == SkillProjectileMotion::Drop && !world.projectiles.height.isEmpty())
        {
            world.projectiles.height.back() = (skill.projectileSpeed >= 0.0) ? Max(1.0, skill.arcHeight) : 0.0;
        }
    }

    inline void SpawnProjectile(BattleWorld& world, const DefinitionStores& defs, UnitId attacker, UnitId target, SkillDefId skillId)
    {
        SpawnProjectile(world, defs, attacker, target, skillId, 0);
    }

    inline void SpawnNextSkillFromProjectile(BattleWorld& world, const DefinitionStores& defs, size_t projectileIndex, bool fromLast, const Array<ProjectileHitTargetInfo>& hitTargets = {})
    {
        if (projectileIndex >= world.projectiles.position.size())
        {
            return;
        }

        const SkillDefId sourceSkillId = world.projectiles.skill[projectileIndex];
        if (sourceSkillId >= defs.skills.size())
        {
            return;
        }

        const SkillDef& sourceSkill = defs.skills[sourceSkillId];
        if (sourceSkill.nextSkill == InvalidSkillDefId || sourceSkill.nextSkill >= defs.skills.size())
        {
            return;
        }
        if (fromLast && !sourceSkill.nextLast)
        {
            return;
        }
        if (world.projectiles.chainDepth[projectileIndex] >= MaxSkillNextChainDepth)
        {
            return;
        }

        const UnitId attacker = world.projectiles.owner[projectileIndex];
        if (!IsValidUnit(world, attacker))
        {
            return;
        }

        const SkillDef& nextSkill = defs.skills[sourceSkill.nextSkill];

        Array<ProjectileHitTargetInfo> spawnSources = hitTargets;
        if (spawnSources.isEmpty())
        {
            spawnSources << ProjectileHitTargetInfo{ world.projectiles.target[projectileIndex], world.projectiles.position[projectileIndex] };
        }

        for (const auto& source : spawnSources)
        {
            SkillNextSpawnContext context;
            context.chainDepth = world.projectiles.chainDepth[projectileIndex] + 1;
            context.originPosition = source.position;
            if (sourceSkill.sendTarget)
            {
                if (IsValidUnit(world, source.unit))
                {
                    context.targetPosition = world.units.position[source.unit];
                }
                else
                {
                    context.targetPosition = world.projectiles.endPosition[projectileIndex];
                }
            }
            else
            {
                context.targetPosition = world.projectiles.firedTargetPosition[projectileIndex];
            }
            if (sourceSkill.sendImageDegree)
            {
                context.imageAngleRad = world.projectiles.angleRad[projectileIndex];
            }

            UnitId target = source.unit;
            if (!IsValidUnit(world, target) || !IsSkillTargetFactionMatch(world, attacker, target, nextSkill))
            {
                target = FindNearestSkillTargetFromOrigin(world, defs, attacker, nextSkill, source.position);
            }

            SpawnProjectile(world, defs, attacker, target, sourceSkill.nextSkill, 0, context);
        }
    }

    inline void UpdateProjectiles(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (size_t effectIndex = 0; effectIndex < world.bomVisualEffects.position.size();)
        {
            world.bomVisualEffects.leftSec[effectIndex] -= dt;
            if (world.bomVisualEffects.leftSec[effectIndex] <= 0.0)
            {
                world.bomVisualEffects.removeAt(effectIndex);
                continue;
            }
            ++effectIndex;
        }

        for (size_t i = 0; i < world.projectiles.position.size();)
        {
            world.projectiles.lifeSec[i] -= dt;
            world.projectiles.ageSec[i] += dt;
            const UnitId target = world.projectiles.target[i];
            const SkillDef& skill = defs.skills[world.projectiles.skill[i]];
            const bool usesFixedImpactPoint = (world.projectiles.motion[i] == SkillProjectileMotion::Arc)
                || (world.projectiles.motion[i] == SkillProjectileMotion::Parabola)
                || (world.projectiles.motion[i] == SkillProjectileMotion::Drop);
            if (!usesFixedImpactPoint && IsValidUnit(world, target))
            {
                world.projectiles.endPosition[i] = world.units.position[target];
            }
            if (world.projectiles.lifeSec[i] <= 0.0)
            {
                SpawnNextSkillFromProjectile(world, defs, i, true);
                world.projectiles.removeAt(i);
                continue;
            }

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
                const Vec2 linePos = world.projectiles.startPosition[i].lerp(world.projectiles.endPosition[i], t);
                const Vec2 delta = world.projectiles.endPosition[i] - world.projectiles.startPosition[i];
                const Vec2 normal = (delta.lengthSq() > 1.0) ? Vec2{ -delta.y, delta.x }.normalized() : Vec2{ 0.0, -1.0 };
                world.projectiles.position[i] = linePos + normal * (Sin(t * Math::Pi) * skill.arcHeight);
                world.projectiles.height[i] = 0.0;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Parabola)
            {
                const double t = Clamp(world.projectiles.ageSec[i] / Max(0.05, world.projectiles.maxLifeSec[i]), 0.0, 1.0);
                world.projectiles.position[i] = world.projectiles.startPosition[i].lerp(world.projectiles.endPosition[i], t);
                world.projectiles.height[i] = Sin(t * Math::Pi) * skill.arcHeight;
            }
            else if (world.projectiles.motion[i] == SkillProjectileMotion::Drop)
            {
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
                    const Vec2 toTarget = world.projectiles.endPosition[i] - world.projectiles.position[i];
                    if (toTarget.length() > 1.0)
                    {
                        world.projectiles.velocity[i] = world.projectiles.velocity[i].lerp(toTarget.normalized() * skill.projectileSpeed, 0.08);
                    }
                }

                world.projectiles.position[i] += world.projectiles.velocity[i] * dt;
                world.projectiles.height[i] = 0.0;
            }

            Array<ProjectileHitTargetInfo> hitResults;
            bool hit = false;
            if (skill.projectileMotion == SkillProjectileMotion::Swing && skill.swingHitMode == SkillSwingHitMode::MultiHitOnce)
            {
                const Array<UnitId> swingTargets = CollectSwingProjectileHitTargets(world, defs, i, skill);
                for (const UnitId swingTarget : swingTargets)
                {
                    if (i < world.projectiles.swingHitUnits.size())
                    {
                        world.projectiles.swingHitUnits[i] << swingTarget;
                    }

                    const Array<ProjectileHitTargetInfo> targetHitResults = ApplyProjectileHit(world, defs, i, swingTarget, world.projectiles.position[i]);
                    for (const auto& hitResult : targetHitResults)
                    {
                        hitResults << hitResult;
                    }
                }

                hit = !swingTargets.isEmpty();
            }
            else if (IsValidUnit(world, target))
            {
                const UnitDef& targetDef = defs.units[world.units.defId[target]];
                const double hitRadius = targetDef.radius + 5.0;
                hit = CanSwingProjectileHitTarget(world, i, target, skill)
                    && (IsSwingEndProjectileHit(skill, world.projectiles.position[i], world.projectiles.angleRad[i], world.units.position[target], hitRadius)
                        || (world.projectiles.position[i].distanceFrom(world.units.position[target]) <= hitRadius));
                if (hit)
                {
                    if (skill.projectileMotion == SkillProjectileMotion::Swing && i < world.projectiles.swingHitUnits.size())
                    {
                        world.projectiles.swingHitUnits[i] << target;
                    }

                    hitResults = ApplyProjectileHit(world, defs, i, target, world.projectiles.position[i]);
                }
            }

            if (hit)
            {
                SpawnNextSkillFromProjectile(world, defs, i, false, hitResults);
                if (!(skill.projectileMotion == SkillProjectileMotion::Swing && skill.swingHitMode == SkillSwingHitMode::MultiHitOnce))
                {
                    world.projectiles.removeAt(i);
                    continue;
                }
            }

            ++i;
        }
    }
}
