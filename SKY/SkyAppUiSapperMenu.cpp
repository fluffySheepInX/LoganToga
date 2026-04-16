# include "SkyAppUiMenusInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        using namespace UiMenusInternal;

        [[nodiscard]] String GetSapperTierUpgradeLabel(const SpawnedSapper& sapper)
        {
            if (sapper.tier >= MaximumSapperTier)
            {
                return U"ティアアップ [最大]";
            }

            return U"ティアアップ (+{}% / {:.0f} 予算)"_fmt(static_cast<int32>(SapperTierStatBonusRate * 100.0), GetTierUpgradeCost(sapper.tier));
        }

        [[nodiscard]] String GetUniqueSkillLabel(const SpawnedSapper& selectedSapper)
        {
            const StringView skillLabel = GetUnitUniqueSkillLabel(selectedSapper.unitType);

            switch (GetUnitUniqueSkillType(selectedSapper.unitType))
            {
            case UniqueSkillType::BuildMill:
                return U"{} [準備中]"_fmt(skillLabel);

            case UniqueSkillType::Heal:
                return U"{} [準備中]"_fmt(skillLabel);

            case UniqueSkillType::Scout:
                if (Scene::Time() < selectedSapper.scoutingSkillUntil)
                {
                    return U"{} [展開中]"_fmt(skillLabel);
                }

                return U"{} ({:.0f} 火薬)"_fmt(skillLabel, Clamp(SapperScoutingSkillGunpowderCost, 0.0, 200.0));
            }

            return String{ skillLabel };
        }

        [[nodiscard]] String GetExplosionSkillLabel(const UnitEditorSettings& unitEditorSettings, const SpawnedSapper& selectedSapper, const bool explosionSkillReady)
        {
            const SapperUnitType selectedUnitType = selectedSapper.unitType;
            if (not CanUnitUseExplosionSkill(selectedUnitType))
            {
                return String{ GetExplosionSkillUnavailableLabel(selectedUnitType) };
            }

            const ExplosionSkillParameters& explosionSkill = GetExplosionSkillParameters(unitEditorSettings, selectedSapper.team, selectedUnitType);

            if (not explosionSkillReady)
            {
                return U"爆破スキル [準備中]";
            }

            return U"爆破スキル ({:.0f} 火薬)"_fmt(Clamp(explosionSkill.gunpowderCost, 0.0, 200.0));
        }
    }

    SapperMenuAction DrawSapperMenu(const SkyAppPanels& panels,
        Array<SpawnedSapper>& spawnedSappers,
        ResourceStock& playerResources,
        const UnitEditorSettings& unitEditorSettings,
        size_t selectedSapperIndex,
        TimedMessage& sapperMenuMessage)
    {
        using namespace UiMenusInternal;

        if (selectedSapperIndex >= spawnedSappers.size())
        {
            return SapperMenuAction::None;
        }

        const Font& font = SimpleGUI::GetFont();

        SpawnedSapper& selectedSapper = spawnedSappers[selectedSapperIndex];
        selectedSapper.tier = Clamp(selectedSapper.tier, MinimumSapperTier, MaximumSapperTier);
        const SapperUnitType selectedUnitType = selectedSapper.unitType;
        const bool explosionSkillReady = (Scene::Time() >= selectedSapper.explosionSkillCooldownUntil);
        const Rect& panelRect = panels.sapperMenu;
        UiInternal::DrawPanelFrame(panelRect, U"兵メニュー", ColorF{ 0.97, 0.95 }, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::Hud);
        font(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 38), ColorF{ 0.12 });
        font(U"魔力: {:.0f} / 選択T {}"_fmt(playerResources.mana, selectedSapper.tier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 60), ColorF{ 0.12 });
        font(U"強化").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 86), ColorF{ 0.22 });
        font(U"スキル").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 144), ColorF{ 0.22 });

        const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, 110);
        const Rect uniqueSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 168);
        const Rect explosionSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 200);
        const Rect retreatButton = SkyAppUiLayout::MenuWideButton(panelRect, 232);

        if (DrawTextButton(tierUpgradeButton, GetSapperTierUpgradeLabel(selectedSapper)))
        {
            switch (TryPurchaseTierUpgrade(playerResources.budget, selectedSapper.tier, MinimumSapperTier, MaximumSapperTier))
            {
            case TierUpgradePurchaseResult::Succeeded:
                ApplySapperTierStatUpgrade(selectedSapper);
                sapperMenuMessage.show(U"{} が Tier {} に上昇 (+{}%)"_fmt(ToUnitDisplayName(selectedUnitType), selectedSapper.tier, static_cast<int32>(SapperTierStatBonusRate * 100.0)));
                break;

            case TierUpgradePurchaseResult::AlreadyMax:
                sapperMenuMessage.show(U"このユニットは最大Tierです");
                break;

            case TierUpgradePurchaseResult::InsufficientBudget:
                sapperMenuMessage.show(U"予算不足");
                break;
            }
        }

        if (DrawTextButton(uniqueSkillButton, GetUniqueSkillLabel(selectedSapper)))
        {
            return SapperMenuAction::UseUniqueSkill;
        }

        if (DrawTextButton(explosionSkillButton, GetExplosionSkillLabel(unitEditorSettings, selectedSapper, explosionSkillReady)))
        {
            return SapperMenuAction::UseExplosionSkill;
        }

        if (DrawTextButton(retreatButton, U"撤退 [3秒後離脱]"))
        {
            return SapperMenuAction::Retreat;
        }

        if (sapperMenuMessage.isVisible())
        {
            font(sapperMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
        }

        return SapperMenuAction::None;
    }
}
