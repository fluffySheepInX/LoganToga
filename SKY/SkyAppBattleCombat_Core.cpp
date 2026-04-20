# include "SkyAppBattleCombatInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow::BattleDetail
{
    void UpdateBaseCombat(Array<SpawnedSapper>& attackers, const Vec3& basePosition, double& baseHitPoints)
    {
        if (baseHitPoints <= 0.0)
        {
            return;
        }

        for (auto& attacker : attackers)
        {
            if (not IsSpawnedSapperCombatActive(attacker))
            {
                continue;
            }

            if ((attacker.team == UnitTeam::Enemy) && (attacker.aiRole != UnitAiRole::AssaultBase))
            {
                continue;
            }

            const double attackDistance = (attacker.attackRange + BaseCombatRadius);
            const double distanceSq = GetSpawnedSapperBasePosition(attacker).distanceFromSq(basePosition);
            if (Square(attackDistance) < distanceSq)
            {
                continue;
            }

            if ((Scene::Time() - attacker.lastAttackAt) < GetEffectiveSapperAttackInterval(attacker))
            {
                continue;
            }

            attacker.lastAttackAt = Scene::Time();
            baseHitPoints = Max(0.0, (baseHitPoints - GetEffectiveSapperAttackDamage(attacker)));
        }
    }

    void SpawnEnemyUnit(SkyAppState& state, const SapperUnitType unitType, const bool moveImmediately)
    {
        static const Array<Vec3> spawnOffsets{
            Vec3{ -2.5, 0, -4.2 },
            Vec3{ 0.0, 0, -3.0 },
            Vec3{ 2.3, 0, -4.0 },
        };

        const Vec3 spawnPosition = state.world.mapData.enemyBasePosition.movedBy(spawnOffsets[state.enemyAi.reinforcementCount % spawnOffsets.size()]);
        SpawnEnemySapper(state.battle.enemySappers, spawnPosition, 180_deg, unitType);
        ApplyUnitParameters(state.battle.enemySappers.back(), GetUnitParameters(state.editor.unitEditorSettings, UnitTeam::Enemy, unitType));

        if (moveImmediately)
        {
            const size_t enemyIndex = (state.battle.enemySappers.size() - 1);
            SetSpawnedSapperTarget(state.battle.enemySappers[enemyIndex], GetSapperPopTargetPosition(state.world.mapData.playerBasePosition, enemyIndex), state.world.mapData, state.editor.modelHeightSettings);
        }

        ++state.enemyAi.reinforcementCount;
    }

    void SpawnEnemyReinforcement(SkyAppState& state, const bool moveImmediately)
    {
        SpawnEnemyUnit(state, SapperUnitType::Infantry, moveImmediately);
    }
}

namespace SkyAppFlow
{
    [[nodiscard]] ExplosionSkillRuntimeParameters GetExplosionSkillRuntimeParameters(const ExplosionSkillParameters& explosionSkill)
    {
        return ExplosionSkillRuntimeParameters{
            .gunpowderCost = Clamp(explosionSkill.gunpowderCost, 0.0, 200.0),
            .cooldownSeconds = Clamp(explosionSkill.cooldownSeconds, 0.1, 30.0),
            .radius = Clamp(explosionSkill.radius, 0.5, 12.0),
            .unitDamage = Clamp(explosionSkill.unitDamage, 0.0, 300.0),
            .baseDamage = Clamp(explosionSkill.baseDamage, 0.0, 300.0),
            .effectLifetime = Clamp(explosionSkill.effectLifetime, 0.05, 2.0),
            .effectThickness = Clamp(explosionSkill.effectThickness, 1.0, 20.0),
            .effectOffset = Vec3{ 0.0, Clamp(explosionSkill.effectOffsetY, 0.0, 4.0), 0.0 },
            .effectColor = explosionSkill.effectColor,
        };
    }

    void EmitAttackEffect(SkyAppState& state,
        const AttackEffectType type,
        const Vec3& startPosition,
        const Vec3& endPosition,
        const ColorF& color,
        const double lifetime,
        const double thickness,
        const double radius)
    {
        state.battle.attackEffects << AttackEffectInstance{
            .type = type,
            .startPosition = startPosition,
            .endPosition = endPosition,
            .color = color,
            .startedAt = Scene::Time(),
            .lifetime = lifetime,
            .thickness = thickness,
            .radius = radius,
        };
    }

    void ApplyExplosionDamage(Array<SpawnedSapper>& targets, const Vec3& explosionCenter, const double radius, const double damage)
    {
        const double radiusSq = Square(Max(0.1, radius));

        for (auto& target : targets)
        {
            if (target.hitPoints <= 0.0)
            {
                continue;
            }

            if (radiusSq < GetSpawnedSapperBasePosition(target).distanceFromSq(explosionCenter))
            {
                continue;
            }

            target.hitPoints = Max(0.0, (target.hitPoints - damage));
        }
    }

    void ApplyExplosionDamageToEnemyBase(SkyAppState& state, const SpawnedSapper& sapper, const double radius, const double baseDamage)
    {
        if (GetSpawnedSapperBasePosition(sapper).distanceFromSq(state.world.mapData.enemyBasePosition) <= Square(radius + BaseCombatRadius))
        {
            state.battle.enemyBaseHitPoints = Max(0.0, (state.battle.enemyBaseHitPoints - baseDamage));
        }
    }
}
