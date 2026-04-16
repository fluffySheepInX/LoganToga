# pragma once
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MainUi.hpp"

namespace SkyAppSupport::UiMenusInternal
{
    using namespace MainSupport;

    constexpr int32 MinimumSapperTier = 1;

    enum class TierUpgradePurchaseResult
    {
        Succeeded,
        AlreadyMax,
        InsufficientBudget,
    };

    [[nodiscard]] inline double GetTierUpgradeCost(const int32 currentTier)
    {
        return (TierUpgradeBaseCost * Max(1, currentTier));
    }

    [[nodiscard]] inline TierUpgradePurchaseResult TryPurchaseTierUpgrade(double& budget,
        int32& currentValue,
        const int32 minimumValue,
        const int32 maximumValue)
    {
        currentValue = Clamp(currentValue, minimumValue, maximumValue);

        if (currentValue >= maximumValue)
        {
            return TierUpgradePurchaseResult::AlreadyMax;
        }

        const double upgradeCost = GetTierUpgradeCost(currentValue);

        if (upgradeCost > budget)
        {
            return TierUpgradePurchaseResult::InsufficientBudget;
        }

        budget -= upgradeCost;
        ++currentValue;
        return TierUpgradePurchaseResult::Succeeded;
    }

    inline void ApplySapperTierStatUpgrade(SpawnedSapper& sapper)
    {
        constexpr double multiplier = (1.0 + SapperTierStatBonusRate);
        sapper.maxHitPoints = Max(1.0, (sapper.maxHitPoints * multiplier));
        sapper.hitPoints = Min(sapper.maxHitPoints, Max(0.0, (sapper.hitPoints * multiplier)));
        sapper.moveSpeed = Max(0.1, (sapper.moveSpeed * multiplier));
        sapper.attackRange = Max(0.5, (sapper.attackRange * multiplier));
        sapper.baseAttackDamage = Max(0.0, (sapper.baseAttackDamage * multiplier));
        sapper.baseAttackInterval = Max(0.05, (sapper.baseAttackInterval / multiplier));
    }

    inline void SetSapperTier(SpawnedSapper& sapper, const int32 tier)
    {
        const int32 clampedTier = Clamp(tier, MinimumSapperTier, MaximumSapperTier);
        sapper.tier = MinimumSapperTier;

        for (int32 currentTier = MinimumSapperTier; currentTier < clampedTier; ++currentTier)
        {
            ApplySapperTierStatUpgrade(sapper);
            ++sapper.tier;
        }
    }

    [[nodiscard]] inline StringView ToUnitDisplayName(const SapperUnitType unitType)
    {
        return GetUnitDisplayName(unitType);
    }

    inline bool TrySpawnPlayerUnit(Array<SpawnedSapper>& spawnedSappers,
        const MapData& mapData,
        const Vec3& playerBasePosition,
        const Vec3& rallyPoint,
        ResourceStock& playerResources,
        const UnitEditorSettings& unitEditorSettings,
        const ModelHeightSettings& modelHeightSettings,
        const SapperUnitType unitType,
        TimedMessage& message)
    {
        const UnitParameters& unitParameters = GetUnitParameters(unitEditorSettings, UnitTeam::Player, unitType);
        const double manaCost = unitParameters.manaCost;

        if (manaCost <= playerResources.mana)
        {
            SpawnSapper(spawnedSappers, playerBasePosition, rallyPoint, mapData, unitType);
            ApplyUnitParameters(spawnedSappers.back(), unitParameters);
            SetSapperTier(spawnedSappers.back(), MinimumSapperTier);
            SetSpawnedSapperTarget(spawnedSappers.back(), rallyPoint, mapData, modelHeightSettings);
            playerResources.mana -= manaCost;
            message.show(U"{}を出撃"_fmt(ToUnitDisplayName(unitType)));
            return true;
        }

        message.show(U"魔力不足");
        return false;
    }
}
