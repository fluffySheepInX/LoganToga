# pragma once
# include "SkyAppBattleInternal.hpp"

namespace SkyAppFlow
{
    struct ExplosionSkillRuntimeParameters
    {
        double gunpowderCost;
        double cooldownSeconds;
        double radius;
        double unitDamage;
        double baseDamage;
        double effectLifetime;
        double effectThickness;
        Vec3 effectOffset;
        ColorF effectColor;
    };

    [[nodiscard]] ExplosionSkillRuntimeParameters GetExplosionSkillRuntimeParameters(const MainSupport::ExplosionSkillParameters& explosionSkill);
    void EmitAttackEffect(SkyAppState& state,
        AttackEffectType type,
        const Vec3& startPosition,
        const Vec3& endPosition,
        const ColorF& color,
        double lifetime,
        double thickness,
        double radius = 0.0);
    void ApplyExplosionDamage(Array<MainSupport::SpawnedSapper>& targets, const Vec3& explosionCenter, double radius, double damage);
    void ApplyExplosionDamageToEnemyBase(SkyAppState& state, const MainSupport::SpawnedSapper& sapper, double radius, double baseDamage);
}
