# include "SkyAppUiInternal.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
      constexpr int32 MinimumSapperTier = 1;

		struct EscMenuItem
		{
			StringView label;
			EscMenuAction action = EscMenuAction::None;
		};

		[[nodiscard]] double GetTierUpgradeCost(const int32 currentTier)
		{
			return (TierUpgradeBaseCost * Max(1, currentTier));
		}

		void ApplySapperTierStatUpgrade(SpawnedSapper& sapper)
		{
			constexpr double multiplier = (1.0 + SapperTierStatBonusRate);
			sapper.maxHitPoints = Max(1.0, (sapper.maxHitPoints * multiplier));
			sapper.hitPoints = Min(sapper.maxHitPoints, Max(0.0, (sapper.hitPoints * multiplier)));
			sapper.moveSpeed = Max(0.1, (sapper.moveSpeed * multiplier));
			sapper.attackRange = Max(0.5, (sapper.attackRange * multiplier));
			sapper.baseAttackDamage = Max(0.0, (sapper.baseAttackDamage * multiplier));
			sapper.baseAttackInterval = Max(0.05, (sapper.baseAttackInterval / multiplier));
		}

		void SetSapperTier(SpawnedSapper& sapper, const int32 tier)
		{
			const int32 clampedTier = Clamp(tier, MinimumSapperTier, MaximumSapperTier);
			sapper.tier = MinimumSapperTier;

			for (int32 currentTier = MinimumSapperTier; currentTier < clampedTier; ++currentTier)
			{
				ApplySapperTierStatUpgrade(sapper);
				++sapper.tier;
			}
		}

		[[nodiscard]] String GetProductionTierUpgradeLabel(const int32 playerTier)
		{
			if (playerTier >= MaximumSapperTier)
			{
				return U"生産ティアアップ [最大]";
			}

			return U"生産ティアアップ ({:.0f} 予算)"_fmt(GetTierUpgradeCost(playerTier));
		}

		[[nodiscard]] String GetSapperTierUpgradeLabel(const SpawnedSapper& sapper)
		{
			if (sapper.tier >= MaximumSapperTier)
			{
				return U"ティアアップ [最大]";
			}

			return U"ティアアップ (+{}% / {:.0f} 予算)"_fmt(static_cast<int32>(SapperTierStatBonusRate * 100.0), GetTierUpgradeCost(sapper.tier));
		}

		[[nodiscard]] String GetProductionUnitLabel(const UnitEditorSettings& unitEditorSettings, const SapperUnitType unitType)
		{
			const double manaCost = GetUnitParameters(unitEditorSettings, UnitTeam::Player, unitType).manaCost;
         return U"{}を出撃 ({:.0f} 魔力)"_fmt(GetUnitDisplayName(unitType), manaCost);
		}

		[[nodiscard]] StringView ToUnitDisplayName(const SapperUnitType unitType)
		{
           return GetUnitDisplayName(unitType);
		}

        [[nodiscard]] String GetExplosionSkillLabel(const SapperUnitType selectedUnitType, const bool explosionSkillReady)
		{
          if (not CanUnitUseExplosionSkill(selectedUnitType))
			{
              return String{ GetExplosionSkillUnavailableLabel(selectedUnitType) };
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
         const int32 playerTier,
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
               SetSapperTier(spawnedSappers.back(), playerTier);
               SetSpawnedSapperTarget(spawnedSappers.back(), rallyPoint, mapData, modelHeightSettings);
				playerResources.mana -= manaCost;
                message.show(U"{}を出撃 (Tier {})"_fmt(ToUnitDisplayName(unitType), spawnedSappers.back().tier));
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
        const ModelHeightSettings& modelHeightSettings,
		TimedMessage& blacksmithMenuMessage)
	{
     playerTier = Clamp(playerTier, MinimumSapperTier, MaximumSapperTier);
		const double tierUpgradeCost = GetTierUpgradeCost(playerTier);
		const Rect& panelRect = panels.blacksmithMenu;
		UiInternal::DrawPanelFrame(panelRect, U"兵生産メニュー");
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 34), ColorF{ 0.12 });
      SimpleGUI::GetFont()(U"魔力: {:.0f} / 生産T {}/{}"_fmt(playerResources.mana, playerTier, MaximumSapperTier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 54), ColorF{ 0.12 });

     const int32 productionButtonTop = 84;
		const int32 menuButtonStep = 32;
		for (size_t index = 0; index < GetUnitDefinitions().size(); ++index)
		{
			const SapperUnitType unitType = GetUnitDefinitions()[index].unitType;
			const Rect produceButton = SkyAppUiLayout::MenuWideButton(panelRect, (productionButtonTop + static_cast<int32>(index) * menuButtonStep));

			if (DrawTextButton(produceButton, GetProductionUnitLabel(unitEditorSettings, unitType)))
			{
				TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, playerTier, unitEditorSettings, modelHeightSettings, unitType, blacksmithMenuMessage);
			}
		}

		const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, (productionButtonTop + static_cast<int32>(GetUnitDefinitions().size()) * menuButtonStep));

      if (DrawTextButton(tierUpgradeButton, GetProductionTierUpgradeLabel(playerTier)))
		{
          if (playerTier >= MaximumSapperTier)
			{
				blacksmithMenuMessage.show(U"生産Tierは最大です");
			}
			else if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
				++playerTier;
             blacksmithMenuMessage.show(U"生産Tier {} に上昇"_fmt(playerTier));
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
		ResourceStock& playerResources,
     const size_t selectedSapperIndex,
		TimedMessage& sapperMenuMessage)
	{
     if (selectedSapperIndex >= spawnedSappers.size())
		{
			return SapperMenuAction::None;
		}

		SpawnedSapper& selectedSapper = spawnedSappers[selectedSapperIndex];
		selectedSapper.tier = Clamp(selectedSapper.tier, MinimumSapperTier, MaximumSapperTier);
		const SapperUnitType selectedUnitType = selectedSapper.unitType;
		const bool explosionSkillReady = (Scene::Time() >= selectedSapper.explosionSkillCooldownUntil);
      const double tierUpgradeCost = GetTierUpgradeCost(selectedSapper.tier);
		const Rect& panelRect = panels.sapperMenu;
		UiInternal::DrawPanelFrame(panelRect, U"兵メニュー", ColorF{ 0.97, 0.95 });
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 38), ColorF{ 0.12 });
       SimpleGUI::GetFont()(U"魔力: {:.0f} / 選択T {}"_fmt(playerResources.mana, selectedSapper.tier)).draw(SkyAppUiLayout::MenuTextPosition(panelRect, 60), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"強化").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 86), ColorF{ 0.22 });
		SimpleGUI::GetFont()(U"スキル").draw(SkyAppUiLayout::MenuTextPosition(panelRect, 144), ColorF{ 0.22 });

        const Rect tierUpgradeButton = SkyAppUiLayout::MenuWideButton(panelRect, 110);
		const Rect scoutingSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 168);
		const Rect fortifySkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 200);
		const Rect explosionSkillButton = SkyAppUiLayout::MenuWideButton(panelRect, 232);
		const Rect retreatButton = SkyAppUiLayout::MenuWideButton(panelRect, 264);

      if (DrawTextButton(tierUpgradeButton, GetSapperTierUpgradeLabel(selectedSapper)))
		{
          if (selectedSapper.tier >= MaximumSapperTier)
			{
				sapperMenuMessage.show(U"このユニットは最大Tierです");
			}
			else if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
               ApplySapperTierStatUpgrade(selectedSapper);
				++selectedSapper.tier;
				sapperMenuMessage.show(U"{} が Tier {} に上昇 (+{}%)"_fmt(ToUnitDisplayName(selectedUnitType), selectedSapper.tier, static_cast<int32>(SapperTierStatBonusRate * 100.0)));
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

		if (DrawTextButton(retreatButton, U"撤退 [3秒後離脱]"))
		{
			return SapperMenuAction::Retreat;
		}

		if (sapperMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(sapperMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
		}

			return SapperMenuAction::None;
	}
}
