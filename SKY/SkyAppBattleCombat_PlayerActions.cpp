# include "SkyAppBattleCombatInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
    namespace
    {
        constexpr double SapperScoutingSkillDuration = 5.0;

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

            const double gunpowderCost = Clamp(SapperScoutingSkillGunpowderCost, 0.0, 200.0);
            if (state.playerResources.gunpowder < gunpowderCost)
            {
                state.blacksmithMenuMessage.show(U"火薬不足");
                return false;
            }

            state.playerResources.gunpowder -= gunpowderCost;
            sapper->scoutingSkillUntil = (Scene::Time() + SapperScoutingSkillDuration);
            state.blacksmithMenuMessage.show(U"{}が偵察スキルを使用"_fmt(GetUnitDisplayName(sapper->unitType)));
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
            state.blacksmithMenuMessage.show(U"建築スキルは未実装");
            return false;

        case UniqueSkillType::Heal:
            state.blacksmithMenuMessage.show(U"回復スキルは未実装");
            return false;

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
