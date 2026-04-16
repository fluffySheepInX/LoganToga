# include "SkyAppUiMenusInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        using namespace UiMenusInternal;

        [[nodiscard]] String GetProductionTierUpgradeLabel(const int32 playerTier)
        {
            if (playerTier >= MaximumSapperTier)
            {
                return U"生産ティアアップ [最大]";
            }

            return U"生産ティアアップ ({:.0f} 予算)"_fmt(GetTierUpgradeCost(playerTier));
        }

        [[nodiscard]] String GetProductionUnitLabel(const UnitEditorSettings& unitEditorSettings, const SapperUnitType unitType)
        {
            const double manaCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, unitType).manaCost;
            return U"{}を出撃 ({:.0f} 魔力)"_fmt(GetUnitDisplayName(unitType), manaCost);
        }
    }

    void DrawBlacksmithMenu(const SkyAppPanels& panels,
        Array<SpawnedSapper>& spawnedSappers,
        const MapData& mapData,
        const Vec3& playerBasePosition,
        const Vec3& rallyPoint,
        ResourceStock& playerResources,
        int32& playerTier,
        const UnitEditorSettings& unitEditorSettings,
        const ModelHeightSettings& modelHeightSettings,
        TimedMessage& blacksmithMenuMessage)
    {
        using namespace UiMenusInternal;

        const Font& font = SimpleGUI::GetFont();

        playerTier = Clamp(playerTier, MinimumSapperTier, MaximumSapperTier);
        const Rect& panelRect = panels.blacksmithMenu;
        UiInternal::DrawPanelFrame(panelRect, U"兵生産メニュー", UiInternal::DefaultPanelBackgroundColor, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::Hud);
        font(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 34), ColorF{ 0.12 });
        font(U"魔力: {:.0f} / 生産T {}/{}"_fmt(playerResources.mana, playerTier, MaximumSapperTier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 54), ColorF{ 0.12 });

        const int32 productionButtonTop = 84;
        const int32 menuButtonStep = 32;
        for (size_t index = 0; index < GetUnitDefinitions().size(); ++index)
        {
            const SapperUnitType unitType = GetUnitDefinitions()[index].unitType;
            const Rect produceButton = SkyAppUiLayout::MenuWideButton(panelRect, (productionButtonTop + static_cast<int32>(index) * menuButtonStep));

            if (DrawTextButton(produceButton, GetProductionUnitLabel(unitEditorSettings, unitType)))
            {
                TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, modelHeightSettings, unitType, blacksmithMenuMessage);
            }
        }

        const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, (productionButtonTop + static_cast<int32>(GetUnitDefinitions().size()) * menuButtonStep));

        if (DrawTextButton(tierUpgradeButton, GetProductionTierUpgradeLabel(playerTier)))
        {
            switch (TryPurchaseTierUpgrade(playerResources.budget, playerTier, MinimumSapperTier, MaximumSapperTier))
            {
            case TierUpgradePurchaseResult::Succeeded:
                blacksmithMenuMessage.show(U"生産Tier {} に上昇"_fmt(playerTier));
                break;

            case TierUpgradePurchaseResult::AlreadyMax:
                blacksmithMenuMessage.show(U"生産Tierは最大です");
                break;

            case TierUpgradePurchaseResult::InsufficientBudget:
                blacksmithMenuMessage.show(U"予算不足");
                break;
            }
        }

        if (blacksmithMenuMessage.isVisible())
        {
            font(blacksmithMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
        }
    }
}
