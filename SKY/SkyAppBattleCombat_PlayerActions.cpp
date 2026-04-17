# include "SkyAppBattleCombatInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
    namespace
    {
        constexpr double MillBuildSpacingRadius = 4.0;
        constexpr double MillBuildBaseClearance = (BaseCombatRadius + 2.2);
        constexpr double MillBuildResourcePadding = 1.0;
        constexpr double ArcaneHealEffectLifetime = 0.45;
        constexpr double ArcaneHealEffectThickness = 6.0;
        constexpr Vec3 ArcaneHealEffectOffset{ 0.0, 1.2, 0.0 };
        constexpr ColorF ArcaneHealEffectColor{ 0.42, 1.0, 0.66, 0.96 };

        [[nodiscard]] SpawnedSapper* TryGetActionablePlayerSapper(SkyAppState& state, const size_t selectedSapperIndex)
        {
            if (selectedSapperIndex >= state.spawnedSappers.size())
            {
                return nullptr;
            }

            SpawnedSapper& sapper = state.spawnedSappers[selectedSapperIndex];
            if (sapper.hitPoints <= 0.0)
            {
                state.blacksmithMenuMessage.show(U"兵が行動不能");
                return nullptr;
            }

            return &sapper;
        }

        [[nodiscard]] Vec3 GetInfantryMillBuildPosition(const SpawnedSapper& sapper, const double forwardOffset)
        {
            const Vec3 origin = GetSpawnedSapperBasePosition(sapper);
            const double yaw = GetSpawnedSapperYaw(sapper);
            const Vec2 forward{ Math::Sin(yaw), Math::Cos(yaw) };
            return Vec3{ origin.x + forward.x * forwardOffset, 0.0, origin.z + forward.y * forwardOffset };
        }

        [[nodiscard]] bool CanBuildMillAt(const SkyAppState& state, const Vec3& buildPosition)
        {
            const double spacingRadiusSq = Square(MillBuildSpacingRadius);

            for (const PlacedModel& placedModel : state.mapData.placedModels)
            {
                if (buildPosition.distanceFromSq(placedModel.position) < spacingRadiusSq)
                {
                    return false;
                }
            }

            if (buildPosition.distanceFromSq(state.mapData.playerBasePosition) < Square(MillBuildBaseClearance)
                || buildPosition.distanceFromSq(state.mapData.enemyBasePosition) < Square(MillBuildBaseClearance))
            {
                return false;
            }

            for (const ResourceArea& area : state.mapData.resourceAreas)
            {
                const double blockedRadius = Max(0.5, area.radius + MillBuildResourcePadding);
                if (buildPosition.distanceFromSq(area.position) < Square(blockedRadius))
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] bool TryUsePlayerInfantryBuildMillSkill(SkyAppState& state, const size_t selectedSapperIndex)
        {
            SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
            if (sapper == nullptr)
            {
                return false;
            }

            if (sapper->unitType != SapperUnitType::Infantry)
            {
                state.blacksmithMenuMessage.show(GetUnitUniqueSkillDeniedMessage(sapper->unitType));
                return false;
            }

            const BuildMillSkillParameters& skillParameters = GetBuildMillSkillParameters(state.unitEditorSettings, sapper->team, sapper->unitType);
            const double manaCost = Clamp(skillParameters.manaCost, 0.0, 200.0);
            const double gunpowderCost = Clamp(skillParameters.gunpowderCost, 0.0, 200.0);
            const double forwardOffset = Clamp(skillParameters.forwardOffset, 1.0, 10.0);
            if (state.playerResources.mana < manaCost)
            {
                state.blacksmithMenuMessage.show(U"魔力不足");
                return false;
            }

            if (state.playerResources.gunpowder < gunpowderCost)
            {
                state.blacksmithMenuMessage.show(U"火薬不足");
                return false;
            }

            const Vec3 buildPosition = GetInfantryMillBuildPosition(*sapper, forwardOffset);
            if (not CanBuildMillAt(state, buildPosition))
            {
                state.blacksmithMenuMessage.show(U"そこには Mill を建てられない");
                return false;
            }

            state.playerResources.mana -= manaCost;
            state.playerResources.gunpowder -= gunpowderCost;
            state.mapData.placedModels << PlacedModel{ .type = PlaceableModelType::Mill, .position = buildPosition, .ownerTeam = UnitTeam::Player, .yaw = GetSpawnedSapperYaw(*sapper) };
            state.selectedMillIndex = (state.mapData.placedModels.size() - 1);
            state.blacksmithMenuMessage.show(U"{}が Mill を建築"_fmt(GetUnitDisplayName(sapper->unitType)));
            return true;
        }

        [[nodiscard]] bool TryUsePlayerVehicleScoutingSkill(SkyAppState& state, const size_t selectedSapperIndex)
        {
            SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
            if (sapper == nullptr)
            {
                return false;
            }

            if (sapper->unitType != SapperUnitType::SugoiCar)
            {
                state.blacksmithMenuMessage.show(GetUnitUniqueSkillDeniedMessage(sapper->unitType));
                return false;
            }

            if (Scene::Time() < sapper->scoutingSkillUntil)
            {
                state.blacksmithMenuMessage.show(U"偵察スキルは展開中");
                return false;
            }

            const ScoutSkillParameters& skillParameters = GetScoutSkillParameters(state.unitEditorSettings, sapper->team, sapper->unitType);
            const double gunpowderCost = Clamp(skillParameters.gunpowderCost, 0.0, 200.0);
            const double durationSeconds = Clamp(skillParameters.durationSeconds, 0.1, 30.0);
            const double visionMultiplier = Clamp(skillParameters.visionMultiplier, 1.0, 4.0);
            if (state.playerResources.gunpowder < gunpowderCost)
            {
                state.blacksmithMenuMessage.show(U"火薬不足");
                return false;
            }

            state.playerResources.gunpowder -= gunpowderCost;
            sapper->scoutingSkillUntil = (Scene::Time() + durationSeconds);
            sapper->scoutingSkillVisionMultiplier = visionMultiplier;
            state.blacksmithMenuMessage.show(U"{}が偵察スキルを使用"_fmt(GetUnitDisplayName(sapper->unitType)));
            return true;
        }

        [[nodiscard]] bool TryUsePlayerArcaneHealSkill(SkyAppState& state, const size_t selectedSapperIndex)
        {
            SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
            if (sapper == nullptr)
            {
                return false;
            }

            if (sapper->unitType != SapperUnitType::ArcaneInfantry)
            {
                state.blacksmithMenuMessage.show(GetUnitUniqueSkillDeniedMessage(sapper->unitType));
                return false;
            }

            const HealSkillParameters& skillParameters = GetHealSkillParameters(state.unitEditorSettings, sapper->team, sapper->unitType);
            const double manaCost = Clamp(skillParameters.manaCost, 0.0, 200.0);
            if (state.playerResources.mana < manaCost)
            {
                state.blacksmithMenuMessage.show(U"魔力不足");
                return false;
            }

            const Vec3 healCenter = GetSpawnedSapperBasePosition(*sapper);
            const double healRadius = Clamp(skillParameters.radius, 0.5, 12.0);
            const double healRadiusSq = Square(healRadius);
            const double healAmount = Clamp(skillParameters.amount, 1.0, 200.0);
            int32 healedCount = 0;

            for (auto& ally : state.spawnedSappers)
            {
                if (not IsSpawnedSapperCombatActive(ally))
                {
                    continue;
                }

                if (healRadiusSq < GetSpawnedSapperBasePosition(ally).distanceFromSq(healCenter))
                {
                    continue;
                }

                if (ally.hitPoints >= ally.maxHitPoints)
                {
                    continue;
                }

                ally.hitPoints = Min(ally.maxHitPoints, (ally.hitPoints + healAmount));
                ++healedCount;
            }

            if (healedCount <= 0)
            {
                state.blacksmithMenuMessage.show(U"回復対象がいない");
                return false;
            }

            state.playerResources.mana -= manaCost;
            sapper->lastAttackAt = Scene::Time();
            EmitAttackEffect(state,
                AttackEffectType::Explosion,
                (healCenter + ArcaneHealEffectOffset),
                (healCenter + ArcaneHealEffectOffset),
                ArcaneHealEffectColor,
                ArcaneHealEffectLifetime,
                ArcaneHealEffectThickness,
                healRadius);
            state.blacksmithMenuMessage.show(U"{}が {} 体を回復"_fmt(GetUnitDisplayName(sapper->unitType), healedCount));
            return true;
        }
    }

    bool TryUsePlayerSapperUniqueSkill(SkyAppState& state, const size_t selectedSapperIndex)
    {
        SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
        if (sapper == nullptr)
        {
            return false;
        }

        switch (GetUnitUniqueSkillType(sapper->unitType))
        {
        case UniqueSkillType::BuildMill:
            return TryUsePlayerInfantryBuildMillSkill(state, selectedSapperIndex);

        case UniqueSkillType::Heal:
            return TryUsePlayerArcaneHealSkill(state, selectedSapperIndex);

        case UniqueSkillType::Scout:
            return TryUsePlayerVehicleScoutingSkill(state, selectedSapperIndex);
        }

        return false;
    }

    bool TryUsePlayerSapperExplosionSkill(SkyAppState& state, const size_t selectedSapperIndex)
    {
        SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
        if (sapper == nullptr)
        {
            return false;
        }

        if (not CanUnitUseExplosionSkill(sapper->unitType))
        {
            state.blacksmithMenuMessage.show(GetExplosionSkillDeniedMessage(sapper->unitType));
            return false;
        }

        const ExplosionSkillParameters& explosionSkill = GetExplosionSkillParameters(state.unitEditorSettings, sapper->team, sapper->unitType);
        const ExplosionSkillRuntimeParameters runtimeParameters = GetExplosionSkillRuntimeParameters(explosionSkill);

        if (state.playerResources.gunpowder < runtimeParameters.gunpowderCost)
        {
            state.blacksmithMenuMessage.show(U"火薬不足");
            return false;
        }

        if (Scene::Time() < sapper->explosionSkillCooldownUntil)
        {
            state.blacksmithMenuMessage.show(U"爆破スキルは準備中");
            return false;
        }

        const Vec3 explosionCenter = GetSpawnedSapperBasePosition(*sapper);
        state.playerResources.gunpowder -= runtimeParameters.gunpowderCost;
        const double currentTime = Scene::Time();
        sapper->explosionSkillCooldownUntil = (currentTime + runtimeParameters.cooldownSeconds);
        sapper->lastAttackAt = currentTime;

        EmitAttackEffect(state,
            AttackEffectType::Explosion,
            (explosionCenter + runtimeParameters.effectOffset),
            (explosionCenter + runtimeParameters.effectOffset),
            runtimeParameters.effectColor,
            runtimeParameters.effectLifetime,
            runtimeParameters.effectThickness,
            runtimeParameters.radius);

        ApplyExplosionDamage(state.enemySappers, explosionCenter, runtimeParameters.radius, runtimeParameters.unitDamage);
        ApplyExplosionDamageToEnemyBase(state, *sapper, runtimeParameters.radius, runtimeParameters.baseDamage);

        state.blacksmithMenuMessage.show(U"兵が爆破スキルを使用");
        return true;
    }

    bool TryOrderPlayerSapperRetreat(SkyAppState& state, const size_t selectedSapperIndex)
    {
        SpawnedSapper* sapper = TryGetActionablePlayerSapper(state, selectedSapperIndex);
        if (sapper == nullptr)
        {
            return false;
        }

        if (IsSapperRetreatOrdered(*sapper))
        {
            state.blacksmithMenuMessage.show(U"すでに撤退中");
            return false;
        }

        OrderSapperRetreat(*sapper, state.mapData.sapperRallyPoint);
        state.blacksmithMenuMessage.show(U"撤退命令: 3秒後に離脱");
        state.selectedSapperIndices.clear();
        return true;
    }
}
