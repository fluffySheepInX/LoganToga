# include "SkyAppUiInternal.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		struct EscMenuItem
		{
			StringView label;
			EscMenuAction action = EscMenuAction::None;
		};

		[[nodiscard]] double GetTierUpgradeCost(const int32 currentTier)
		{
			return (TierUpgradeBaseCost * Max(1, currentTier));
		}

		[[nodiscard]] StringView ToUnitDisplayName(const SapperUnitType unitType)
		{
			switch (unitType)
			{
			case SapperUnitType::ArcaneInfantry:
				return U"魔導兵(仮)";

			case SapperUnitType::Infantry:
			default:
				return U"兵";
			}
		}

        [[nodiscard]] String GetExplosionSkillLabel(const SapperUnitType selectedUnitType, const bool explosionSkillReady)
		{
         if (selectedUnitType != SapperUnitType::Infantry)
			{
				return U"爆破スキル [兵専用]";
			}

            if (!explosionSkillReady)
			{
				return U"爆破スキル [準備中]";
			}

			return U"爆破スキル ({:.0f} 火薬)"_fmt(SapperExplosionGunpowderCost);
		}

		bool TrySpawnPlayerUnit(Array<SpawnedSapper>& spawnedSappers,
         const MapData& mapData,
			const Vec3& playerBasePosition,
			const Vec3& rallyPoint,
			ResourceStock& playerResources,
          const UnitEditorSettings& unitEditorSettings,
			const SapperUnitType unitType,
			TimedMessage& message)
		{
           const UnitParameters& unitParameters = GetUnitParameters(unitEditorSettings, UnitTeam::Player, unitType);
			const double manaCost = unitParameters.manaCost;

			if (manaCost <= playerResources.mana)
			{
              SpawnSapper(spawnedSappers, playerBasePosition, rallyPoint, mapData, unitType);
               ApplyUnitParameters(spawnedSappers.back(), unitParameters);
				playerResources.mana -= manaCost;
				message.show(U"{}を出撃"_fmt(ToUnitDisplayName(unitType)));
				return true;
			}

			message.show(U"魔力不足");
			return false;
		}
	}

	EscMenuAction DrawEscMenu(const Rect& panelRect)
	{
		static constexpr EscMenuItem Items[]
		{
			{ U"Restart", EscMenuAction::Restart },
           { U"Title", EscMenuAction::Title },
			{ U"1280 x 720", EscMenuAction::Resize1280x720 },
			{ U"1600 x 900", EscMenuAction::Resize1600x900 },
			{ U"1920 x 1080", EscMenuAction::Resize1920x1080 },
		};

		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.35 });
		UiInternal::DrawPanelFrame(panelRect, U"ESC Menu", ColorF{ 0.98, 0.96, 0.94, 0.98 });

		for (size_t i = 0; i < std::size(Items); ++i)
		{
			const Rect buttonRect = SkyAppUiLayout::MenuWideButton(panelRect, (48 + static_cast<int32>(i) * 36));

			if (DrawTextButton(buttonRect, Items[i].label))
			{
				return Items[i].action;
			}
		}

        SimpleGUI::GetFont()(U"Window: {} x {}"_fmt(Scene::Width(), Scene::Height())).draw((panelRect.x + 16), (panelRect.y + 236), ColorF{ 0.18 });
		SimpleGUI::GetFont()(U"Press ESC to close").draw((panelRect.x + 16), (panelRect.y + 258), ColorF{ 0.18 });
		return EscMenuAction::None;
	}

	void DrawBlacksmithMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
     const MapData& mapData,
		const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
		ResourceStock& playerResources,
		int32& playerTier,
        const UnitEditorSettings& unitEditorSettings,
		TimedMessage& blacksmithMenuMessage)
	{
      const double sapperCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, SapperUnitType::Infantry).manaCost;
		const double arcaneInfantryCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, SapperUnitType::ArcaneInfantry).manaCost;
		const double tierUpgradeCost = GetTierUpgradeCost(playerTier);
		const Rect& panelRect = panels.blacksmithMenu;
		UiInternal::DrawPanelFrame(panelRect, U"兵生産メニュー");
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 34), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"魔力: {:.0f} / Tier {}"_fmt(playerResources.mana, playerTier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 54), ColorF{ 0.12 });

		const Rect produceSapperButton = SkyAppUiLayout::MenuWideButton(panelRect, 84);
		const Rect produceArcaneButton = SkyAppUiLayout::MenuWideButton(panelRect, 116);
		const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, 148);

		if (DrawTextButton(produceSapperButton, U"兵を出撃 ({:.0f} 魔力)"_fmt(sapperCost)))
		{
           TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, SapperUnitType::Infantry, blacksmithMenuMessage);
		}

     if (DrawTextButton(produceArcaneButton, U"魔導兵(仮) ({:.0f} 魔力)"_fmt(arcaneInfantryCost)))
		{
           TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, SapperUnitType::ArcaneInfantry, blacksmithMenuMessage);
		}

		if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード ({:.0f} 予算)"_fmt(tierUpgradeCost)))
		{
			if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
				++playerTier;
				blacksmithMenuMessage.show(U"Tier {} に上昇"_fmt(playerTier));
			}
			else
			{
				blacksmithMenuMessage.show(U"予算不足");
			}
		}

		if (blacksmithMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(blacksmithMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
		}
	}

     SapperMenuAction DrawSapperMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
     const MapData& mapData,
		const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
		ResourceStock& playerResources,
		int32& playerTier,
     const SapperUnitType selectedUnitType,
		const bool explosionSkillReady,
        const UnitEditorSettings& unitEditorSettings,
		TimedMessage& sapperMenuMessage)
	{
      const double sapperCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, SapperUnitType::Infantry).manaCost;
		const double arcaneInfantryCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, SapperUnitType::ArcaneInfantry).manaCost;
		const double tierUpgradeCost = GetTierUpgradeCost(playerTier);
		const Rect& panelRect = panels.sapperMenu;
		UiInternal::DrawPanelFrame(panelRect, U"兵メニュー", ColorF{ 0.97, 0.95 });
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 38), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"魔力: {:.0f} / Tier {}"_fmt(playerResources.mana, playerTier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 60), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"生産").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 86), ColorF{ 0.22 });
		SimpleGUI::GetFont()(U"スキル").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 208), ColorF{ 0.22 });

		const Rect produceSapperButton = SkyAppUiLayout::MenuWideButton(panelRect, 110);
		const Rect produceArcaneButton = SkyAppUiLayout::MenuWideButton(panelRect, 142);
		const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, 174);
		const Rect scoutingSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 232);
		const Rect fortifySkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 264);
			const Rect explosionSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 296);

		if (DrawTextButton(produceSapperButton, U"兵を出撃 ({:.0f} 魔力)"_fmt(sapperCost)))
		{
           TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, SapperUnitType::Infantry, sapperMenuMessage);
		}

     if (DrawTextButton(produceArcaneButton, U"魔導兵(仮) ({:.0f} 魔力)"_fmt(arcaneInfantryCost)))
		{
           TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, unitEditorSettings, SapperUnitType::ArcaneInfantry, sapperMenuMessage);
		}

		if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード ({:.0f} 予算)"_fmt(tierUpgradeCost)))
		{
			if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
				++playerTier;
				sapperMenuMessage.show(U"Tier {} に上昇"_fmt(playerTier));
			}
			else
			{
				sapperMenuMessage.show(U"予算不足");
			}
		}

		if (DrawTextButton(scoutingSkillButton, U"索敵スキル"))
		{
			sapperMenuMessage.show(U"兵が索敵スキルを準備");
		}

		if (DrawTextButton(fortifySkillButton, U"陣地化スキル"))
		{
			sapperMenuMessage.show(U"兵が陣地化スキルを準備");
		}

       if (DrawTextButton(explosionSkillButton, GetExplosionSkillLabel(selectedUnitType, explosionSkillReady)))
			{
				return SapperMenuAction::UseExplosionSkill;
			}

		if (sapperMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(sapperMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
		}

			return SapperMenuAction::None;
	}
}
