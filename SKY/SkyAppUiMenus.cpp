# include "SkyAppUiInternal.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
      constexpr int32 MinimumSapperTier = 1;
		constexpr int32 BattleCommandSlotCount = 4;

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

		[[nodiscard]] Optional<SapperUnitType> GetBattleCommandUnitType(const size_t slotIndex)
		{
			switch (slotIndex)
			{
			case 0:
				return SapperUnitType::Infantry;

			case 1:
				return SapperUnitType::ArcaneInfantry;

			case 2:
				return SapperUnitType::SugoiCar;

			default:
				return none;
			}
		}

     [[nodiscard]] String GetBattleCommandTierUpLabel(const int32 unlockedSlotCount)
		{
         if (unlockedSlotCount >= BattleCommandSlotCount)
			{
				return U"Tier up [MAX]";
			}

         return U"Tier up ({:.0f})"_fmt(GetTierUpgradeCost(unlockedSlotCount));
		}

		[[nodiscard]] ColorF GetBattleCommandAccentColor(const Optional<SapperUnitType>& unitType)
		{
			if (not unitType)
			{
				return ColorF{ 0.70, 0.74, 0.82, 1.0 };
			}

			switch (*unitType)
			{
			case SapperUnitType::Infantry:
				return ColorF{ 0.74, 0.84, 0.98, 1.0 };

			case SapperUnitType::ArcaneInfantry:
				return ColorF{ 0.76, 0.68, 0.98, 1.0 };

			case SapperUnitType::SugoiCar:
			default:
				return ColorF{ 0.96, 0.78, 0.42, 1.0 };
			}
		}

      [[nodiscard]] String GetBattleCommandPortraitLabel(const Optional<SapperUnitType>& unitType)
		{
			if (not unitType)
			{
				return U"Reserve";
			}

         return String{ GetUnitDisplayName(*unitType) };
		}

		[[nodiscard]] bool DrawBattleCommandTabButton(const Rect& rect,
			const String& label,
			const bool unlocked,
			const bool selected)
		{
			static const Font tabFont{ 18, Typeface::Bold };
			const bool hovered = unlocked && rect.mouseOver();
			const ColorF fillColor = unlocked
				? (selected ? ColorF{ 0.40, 0.54, 0.82, 0.96 } : (hovered ? ColorF{ 0.86, 0.90, 0.98, 0.98 } : ColorF{ 0.95, 0.96, 0.98, 0.94 }))
				: ColorF{ 0.32, 0.36, 0.44, 0.90 };
			const ColorF frameColor = selected ? ColorF{ 0.16, 0.26, 0.48, 1.0 } : ColorF{ 0.26, 0.30, 0.36, 0.90 };
			rect.rounded(6).draw(fillColor).drawFrame(2, 0, frameColor);
			tabFont(label).drawAt(rect.center().movedBy(0, unlocked ? 0 : -8), unlocked ? ColorF{ 0.08, 0.12, 0.18 } : ColorF{ 0.78, 0.82, 0.90 });

			if (not unlocked)
			{
				SimpleGUI::GetFont()(U"LOCK").drawAt(rect.center().movedBy(0, 12), ColorF{ 0.82, 0.86, 0.94, 0.88 });
			}

			return hovered && MouseL.down();
		}

		[[nodiscard]] bool DrawBattleCommandActionButton(const Rect& rect,
			const String& label,
			const bool enabled,
			const ColorF& accentColor = ColorF{ 0.34, 0.46, 0.72, 1.0 })
		{
			static const Font buttonFont{ 18, Typeface::Bold };
			const bool hovered = enabled && rect.mouseOver();
			const ColorF fillColor = enabled
				? (hovered ? accentColor.lerp(ColorF{ 1.0, 1.0, 1.0, 1.0 }, 0.18) : accentColor)
				: ColorF{ 0.42, 0.46, 0.50, 0.78 };
			rect.rounded(6).draw(fillColor).drawFrame(2, 0, enabled ? ColorF{ 0.12, 0.18, 0.28, 0.94 } : ColorF{ 0.24, 0.26, 0.30, 0.84 });
			buttonFont(label).drawAt(rect.center(), enabled ? ColorF{ 0.98 } : ColorF{ 0.86, 0.88, 0.90 });
			return hovered && MouseL.down();
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

      [[nodiscard]] String GetExplosionSkillLabel(const UnitEditorSettings& unitEditorSettings, const SpawnedSapper& selectedSapper, const bool explosionSkillReady)
		{
           const SapperUnitType selectedUnitType = selectedSapper.unitType;
          if (not CanUnitUseExplosionSkill(selectedUnitType))
			{
              return String{ GetExplosionSkillUnavailableLabel(selectedUnitType) };
			}

			const ExplosionSkillParameters& explosionSkill = GetExplosionSkillParameters(unitEditorSettings, selectedSapper.team, selectedUnitType);

            if (!explosionSkillReady)
			{
				return U"爆破スキル [準備中]";
			}

          return U"爆破スキル ({:.0f} 火薬)"_fmt(Clamp(explosionSkill.gunpowderCost, 0.0, 200.0));
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

	void DrawBattleCommandMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
		const MapData& mapData,
		const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
		ResourceStock& playerResources,
		int32& playerTier,
		size_t& selectedSlotIndex,
		int32& unlockedSlotCount,
		const UnitEditorSettings& unitEditorSettings,
		const ModelHeightSettings& modelHeightSettings,
		TimedMessage& battleCommandMessage)
	{
       (void)playerTier;
		unlockedSlotCount = Clamp(unlockedSlotCount, 1, BattleCommandSlotCount);
		selectedSlotIndex = Min(selectedSlotIndex, static_cast<size_t>(unlockedSlotCount - 1));

		const Rect& panelRect = panels.blacksmithMenu;
		const Rect portraitRect = SkyAppUiLayout::BattleCommandPortrait(panelRect);
		const Rect detailRect = SkyAppUiLayout::BattleCommandDetail(panelRect);
      const double tierUpgradeCost = GetTierUpgradeCost(unlockedSlotCount);

		UiInternal::DrawPanelFrame(panelRect, U"Battle Command", ColorF{ 0.97, 0.97, 0.98, 0.96 });
		SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panelRect.x + 16), (panelRect.y + 18), ColorF{ 0.12 });
      SimpleGUI::GetFont()(U"魔力: {:.0f} / 解放 {}/{}"_fmt(playerResources.mana, unlockedSlotCount, BattleCommandSlotCount)).draw((panelRect.x + 162), (panelRect.y + 18), ColorF{ 0.12 });

		for (int32 index = 0; index < BattleCommandSlotCount; ++index)
		{
			const bool unlocked = (index < unlockedSlotCount);
			if (DrawBattleCommandTabButton(SkyAppUiLayout::BattleCommandSlotButton(panelRect, index), U"T{}"_fmt(index + 1), unlocked, (selectedSlotIndex == static_cast<size_t>(index))))
			{
				selectedSlotIndex = static_cast<size_t>(index);
			}
		}

       const Optional<SapperUnitType> selectedUnitType = GetBattleCommandUnitType(selectedSlotIndex);
		const ColorF accentColor = GetBattleCommandAccentColor(selectedUnitType);
		portraitRect.rounded(8).draw(accentColor.lerp(ColorF{ 0.08, 0.10, 0.14, 1.0 }, 0.68)).drawFrame(2, 0, accentColor);
        SimpleGUI::GetFont()(GetBattleCommandPortraitLabel(selectedUnitType)).drawAt(portraitRect.center().movedBy(0, -10), Palette::White);
		SimpleGUI::GetFont()(selectedUnitType ? U"Unit image / icon fallback" : U"Slot reserved").drawAt(portraitRect.center().movedBy(0, 28), ColorF{ 0.90, 0.93, 0.98 });

		detailRect.rounded(8).draw(ColorF{ 0.95, 0.96, 0.98, 0.98 }).drawFrame(2, 0, accentColor);
		if (selectedUnitType)
		{
			const UnitParameters& unitParameters = GetUnitParameters(unitEditorSettings, UnitTeam::Player, *selectedUnitType);
			SimpleGUI::GetFont()(GetUnitDisplayName(*selectedUnitType)).draw((detailRect.x + 12), (detailRect.y + 10), ColorF{ 0.12 });
			SimpleGUI::GetFont()(U"消費魔力: {:.0f}"_fmt(unitParameters.manaCost)).draw((detailRect.x + 12), (detailRect.y + 34), ColorF{ 0.18 });
			SimpleGUI::GetFont()(U"HP {:.0f} / 攻撃 {:.0f}"_fmt(unitParameters.maxHitPoints, unitParameters.attackDamage)).draw((detailRect.x + 12), (detailRect.y + 56), ColorF{ 0.18 });
			SimpleGUI::GetFont()(U"射程 {:.1f} / 移動 {:.1f}"_fmt(unitParameters.attackRange, unitParameters.moveSpeed)).draw((detailRect.x + 12), (detailRect.y + 78), ColorF{ 0.18 });

			if (DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandPrimaryActionButton(panelRect), U"出撃", true, accentColor))
			{
				TrySpawnPlayerUnit(spawnedSappers, mapData, playerBasePosition, rallyPoint, playerResources, playerTier, unitEditorSettings, modelHeightSettings, *selectedUnitType, battleCommandMessage);
			}
		}
		else
		{
			SimpleGUI::GetFont()(U"T4 Reserve").draw((detailRect.x + 12), (detailRect.y + 10), ColorF{ 0.12 });
			SimpleGUI::GetFont()(U"将来の拡張スロットです").draw((detailRect.x + 12), (detailRect.y + 38), ColorF{ 0.20 });
			SimpleGUI::GetFont()(U"Tier up で順番にアンロック" ).draw((detailRect.x + 12), (detailRect.y + 62), ColorF{ 0.20 });
			DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandPrimaryActionButton(panelRect), U"準備中", false, accentColor);
		}

     if (DrawBattleCommandActionButton(SkyAppUiLayout::BattleCommandTierUpButton(panelRect), GetBattleCommandTierUpLabel(unlockedSlotCount), true, ColorF{ 0.34, 0.38, 0.46, 1.0 }))
		{
			const bool canUnlockSlot = (unlockedSlotCount < BattleCommandSlotCount);

         if (not canUnlockSlot)
			{
             battleCommandMessage.show(U"スロットは最大です");
			}
			else if (tierUpgradeCost <= playerResources.budget)
			{
				playerResources.budget -= tierUpgradeCost;
              ++unlockedSlotCount;
				selectedSlotIndex = static_cast<size_t>(unlockedSlotCount - 1);
				battleCommandMessage.show(U"T{} を解放"_fmt(unlockedSlotCount));
			}
			else
			{
				battleCommandMessage.show(U"予算不足");
			}
		}

		const Rect messageRect = SkyAppUiLayout::BattleCommandMessageRect(panelRect);
		messageRect.rounded(8).draw(ColorF{ 0.92, 0.94, 0.98, 0.70 }).drawFrame(1, 0, ColorF{ 0.68, 0.74, 0.82, 0.72 });
		SimpleGUI::GetFont()(U"押した Tier が上から順に解放されます").draw((messageRect.x + 10), (messageRect.y + 8), ColorF{ 0.18 });
		if (battleCommandMessage.isVisible())
		{
			SimpleGUI::GetFont()(battleCommandMessage.text).draw((messageRect.x + 10), (messageRect.y + 30), ColorF{ 0.12 });
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
      const UnitEditorSettings& unitEditorSettings,
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
          return SapperMenuAction::UseScoutingSkill;
		}

		if (DrawTextButton(fortifySkillButton, U"陣地化スキル"))
		{
			sapperMenuMessage.show(U"兵が陣地化スキルを準備");
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
			SimpleGUI::GetFont()(sapperMenuMessage.text).draw(SkyAppUiLayout::MenuMessagePosition(panelRect), ColorF{ 0.12 });
		}

			return SapperMenuAction::None;
	}
}
