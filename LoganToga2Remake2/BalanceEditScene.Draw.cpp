#include "BalanceEditScene.h"

#include "Localization.h"
#include "RunCardPresentation.h"

void BalanceEditScene::drawLeftPanel() const
{
	const auto& data = getData();
	const RectF panel = getLeftPanelRect();
 data.smallFont(Localization::GetText(U"balance_edit.left_panel.title")).draw(panel.x + 16, panel.y + 16, Palette::White);
	const String summaryText = (m_tab == Tab::Core)
		? Localization::GetText(U"balance_edit.left_panel.core_summary")
		: ((m_tab == Tab::Progression)
			? Localization::GetText(U"balance_edit.left_panel.progression_summary")
			: ((m_tab == Tab::Units)
				? Localization::GetText(U"balance_edit.left_panel.units_summary")
				: Localization::GetText(U"balance_edit.left_panel.cards_summary")));
	data.smallFont(summaryText).draw(panel.x + 16, panel.y + 40, ColorF{ 0.84, 0.90, 0.98 });

	if (m_tab == Tab::Core)
	{
        const String detailText = Localization::GetText(U"balance_edit.left_panel.core_detail")
			+ U"\n\n" + Localization::GetText(U"balance_edit.left_panel.progression_detail")
			+ U"\n\n" + Localization::GetText(U"balance_edit.left_panel.units_detail")
			+ U"\n\n" + Localization::GetText(U"balance_edit.left_panel.cards_detail");
		const RectF detailRect{ panel.x + 16, panel.y + 84, panel.w - 32, panel.h - 100 };
		data.smallFont(detailText).draw(detailRect, Palette::White);
		return;
	}

	if (m_tab == Tab::Progression)
	{
		for (size_t index = 0; index < m_editConfig.enemyProgression.size(); ++index)
		{
			const auto& progression = m_editConfig.enemyProgression[index];
			drawButton(
				getProgressionButtonRect(static_cast<int32>(index)),
				Localization::FormatText(U"balance_edit.progression.entry", progression.battle),
				data.smallFont,
				static_cast<int32>(index) == m_selectedProgressionIndex);
		}
		return;
	}

	if (m_tab == Tab::Units)
	{
		for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
		{
			const auto& unit = m_editConfig.unitDefinitions[index];
			drawButton(getUnitButtonRect(static_cast<int32>(index)), toUnitArchetypeDisplayString(unit.archetype), data.smallFont, static_cast<int32>(index) == m_selectedUnitIndex);
		}
		return;
	}

	for (size_t index = 0; index < m_editCards.size(); ++index)
	{
		const auto& card = m_editCards[index];
      const String label = hasTestOwnedCard(card.id)
			? (U"★ " + GetRewardCardName(card))
			: GetRewardCardName(card);
		drawButton(getCardButtonRect(static_cast<int32>(index)), label, data.smallFont, static_cast<int32>(index) == m_selectedCardIndex);
	}
}

void BalanceEditScene::drawCorePanel() const
{
	const auto& data = getData();
    drawButton(getCoreTestStartButtonRect(), Localization::GetText(U"balance_edit.button.test_start"), data.smallFont);
 drawIntRow(0, Localization::GetText(U"balance_edit.row.player_gold"), m_editConfig.playerGold, data.smallFont);
	drawIntRow(1, Localization::GetText(U"balance_edit.row.enemy_gold"), m_editConfig.enemyGold, data.smallFont);
	drawDoubleRow(2, Localization::GetText(U"balance_edit.row.income_interval"), m_editConfig.income.interval, data.smallFont);
	drawIntRow(3, Localization::GetText(U"balance_edit.row.player_income"), m_editConfig.income.playerAmount, data.smallFont);
	drawIntRow(4, Localization::GetText(U"balance_edit.row.enemy_income"), m_editConfig.income.enemyAmount, data.smallFont);
	drawDoubleRow(5, Localization::GetText(U"balance_edit.row.spawn_interval"), m_editConfig.enemySpawn.interval, data.smallFont);
	drawDoubleRow(6, Localization::GetText(U"balance_edit.row.advanced_spawn_rate"), m_editConfig.enemySpawn.advancedProbability, data.smallFont);
	drawDoubleRow(7, Localization::GetText(U"balance_edit.row.spawn_random_y"), m_editConfig.enemySpawn.randomYOffset, data.smallFont);
	drawIntRow(8, Localization::GetText(U"balance_edit.row.assault_threshold"), m_editConfig.enemyAI.assaultUnitThreshold, data.smallFont);
	drawIntRow(9, Localization::GetText(U"balance_edit.row.staging_min_units"), m_editConfig.enemyAI.stagingAssaultMinUnits, data.smallFont);
	drawDoubleRow(10, Localization::GetText(U"balance_edit.row.decision_interval"), m_editConfig.enemyAI.decisionInterval, data.smallFont);
	drawDoubleRow(11, Localization::GetText(U"balance_edit.row.defense_radius"), m_editConfig.enemyAI.defenseRadius, data.smallFont);
	drawDoubleRow(12, Localization::GetText(U"balance_edit.row.rally_distance"), m_editConfig.enemyAI.rallyDistance, data.smallFont);
	drawDoubleRow(13, Localization::GetText(U"balance_edit.row.base_assault_lock_radius"), m_editConfig.enemyAI.baseAssaultLockRadius, data.smallFont);
	drawDoubleRow(14, Localization::GetText(U"balance_edit.row.staging_gather_radius"), m_editConfig.enemyAI.stagingAssaultGatherRadius, data.smallFont);
	drawDoubleRow(15, Localization::GetText(U"balance_edit.row.staging_max_wait"), m_editConfig.enemyAI.stagingAssaultMaxWait, data.smallFont);
	drawDoubleRow(16, Localization::GetText(U"balance_edit.row.staging_commit_time"), m_editConfig.enemyAI.stagingAssaultCommitTime, data.smallFont);
	drawTextRow(17, Localization::GetText(U"balance_edit.row.attack_target_pathfinding"), m_editConfig.enemyAI.usePathfindingForAttackTarget ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"), data.smallFont);
   drawIntRow(18, Localization::GetText(U"balance_edit.row.test_battle_number"), m_testBattleNumber, data.smallFont);
   drawTextRow(19, Localization::GetText(U"balance_edit.row.test_owned_cards"), Format(m_testOwnedCardIds.size()), data.smallFont);
}

void BalanceEditScene::drawProgressionPanel() const
{
	const auto& data = getData();
	const auto* progression = getSelectedProgressionConfig();
	if (!progression)
	{
		data.smallFont(Localization::GetText(U"balance_edit.no_progression_selected")).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
		return;
	}

	data.uiFont(Localization::FormatText(U"balance_edit.progression.title", progression->battle)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
	data.smallFont(Localization::FormatText(U"balance_edit.progression.map_source", FileSystem::FileName(progression->mapSourcePath))).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 146, ColorF{ 0.74, 0.84, 0.98 });
	data.smallFont(Localization::FormatText(U"balance_edit.progression.enemy_initial_units", progression->enemyInitialUnits.size())).draw(getEditorPanelRect().x + 240, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

	drawIntRow(0, Localization::GetText(U"balance_edit.row.gold_bonus"), progression->goldBonus, data.smallFont);
	drawIntRow(1, Localization::GetText(U"balance_edit.row.income_bonus"), progression->incomeBonus, data.smallFont);
	drawDoubleRow(2, Localization::GetText(U"balance_edit.row.spawn_interval"), progression->spawnInterval, data.smallFont);
	drawIntRow(3, Localization::GetText(U"balance_edit.row.assault_threshold"), progression->assaultUnitThreshold, data.smallFont);
	drawTextRow(4, Localization::GetText(U"balance_edit.row.ai_mode_override"), progression->overrideEnemyAiMode ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"), data.smallFont);
	drawTextRow(5, Localization::GetText(U"balance_edit.row.ai_mode"), progression->overrideEnemyAiMode ? toEnemyAiModeDisplayString(progression->enemyAiMode) : Localization::GetText(U"balance_edit.progression.ai_mode_disabled"), data.smallFont);
	drawIntRow(6, Localization::GetText(U"balance_edit.row.staging_min_units"), progression->stagingAssaultMinUnits, data.smallFont);
	drawDoubleRow(7, Localization::GetText(U"balance_edit.row.staging_gather_radius"), progression->stagingAssaultGatherRadius, data.smallFont);
	drawDoubleRow(8, Localization::GetText(U"balance_edit.row.staging_max_wait"), progression->stagingAssaultMaxWait, data.smallFont);
	drawDoubleRow(9, Localization::GetText(U"balance_edit.row.staging_commit_time"), progression->stagingAssaultCommitTime, data.smallFont);
	drawIntRow(10, Localization::GetText(U"balance_edit.row.extra_basic_units"), progression->extraBasicUnits, data.smallFont);
	drawIntRow(11, Localization::GetText(U"balance_edit.row.extra_advanced_units"), progression->extraAdvancedUnits, data.smallFont);
	drawTextRow(12, Localization::GetText(U"balance_edit.row.replace_enemy_initial_units"), progression->replaceEnemyInitialUnits ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"), data.smallFont);
}

void BalanceEditScene::drawUnitPanel() const
{
	const auto& data = getData();
	const auto* unit = getSelectedUnitDefinition();
	if (!unit)
	{
        data.smallFont(Localization::GetText(U"balance_edit.no_unit_selected")).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
		return;
	}

	data.uiFont(toUnitArchetypeDisplayString(unit->archetype)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
	const auto* slot = getSelectedProductionSlot();
   data.smallFont(slot
		? Localization::GetText(U"balance_edit.unit_panel.queue_backed")
		: Localization::GetText(U"balance_edit.unit_panel.no_production_slot"))
		.draw(getEditorPanelRect().x + 220, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

	int32 row = 0;
	drawIntRow(row++, U"HP", unit->hp, data.smallFont);
    drawIntRow(row++, Localization::GetText(U"balance_edit.row.attack"), unit->attackPower, data.smallFont);
	drawIntRow(row++, Localization::GetText(U"balance_edit.row.unit_cost"), unit->cost, data.smallFont);
	if (slot)
	{
      drawIntRow(row++, Localization::GetText(U"balance_edit.row.queue_cost"), getSelectedProductionCost(), data.smallFont);
		drawIntRow(row++, Localization::GetText(U"balance_edit.row.batch_count"), slot->batchCount, data.smallFont);
	}
 drawDoubleRow(row++, Localization::GetText(U"balance_edit.row.production_time"), unit->productionTime, data.smallFont);
	drawDoubleRow(row++, Localization::GetText(U"balance_edit.row.move_speed"), unit->moveSpeed, data.smallFont);
	drawDoubleRow(row++, Localization::GetText(U"balance_edit.row.attack_range"), unit->attackRange, data.smallFont);
	drawDoubleRow(row++, Localization::GetText(U"balance_edit.row.attack_cooldown"), unit->attackCooldown, data.smallFont);
	drawDoubleRow(row++, Localization::GetText(U"balance_edit.row.aggro_range"), unit->aggroRange, data.smallFont);
}

void BalanceEditScene::drawCardPanel() const
{
	const auto& data = getData();
	const auto* card = getSelectedCardDefinition();
	if (!card)
	{
        data.smallFont(Localization::GetText(U"balance_edit.no_card_selected")).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
		return;
	}

    data.uiFont(GetRewardCardName(*card)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
	data.smallFont(card->id).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 146, ColorF{ 0.74, 0.84, 0.98 });
	data.smallFont(toRewardCardEffectTypeDisplayString(card->effectType)).draw(getEditorPanelRect().x + 240, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

	String targetText;
	if (card->effectType == RewardCardEffectType::TurretUpgradeUnlock)
	{
       targetText = card->targetTurretUpgradeType ? toTurretUpgradeTypeDisplayString(*card->targetTurretUpgradeType) : Localization::GetText(U"balance_edit.no_target");
	}
	else
	{
		targetText = toUnitArchetypeDisplayString(card->targetArchetype);
		if (card->effectType == RewardCardEffectType::UnitStatBonus)
		{
			targetText += U" / " + toRewardCardStatTypeDisplayString(card->statType);
		}
	}
	data.smallFont(targetText).draw(getEditorPanelRect().x + 240, getEditorPanelRect().y + 150, ColorF{ 0.82, 0.90, 1.0 });
  data.smallFont(GetRewardCardDescription(*card)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 174, ColorF{ 0.86, 0.90, 0.96 });

    drawDoubleRow(0, Localization::GetText(U"balance_edit.row.value"), card->value, data.smallFont);
	drawTextRow(1, Localization::GetText(U"balance_edit.row.rarity"), toRewardCardRarityDisplayString(card->rarity), data.smallFont);
	drawTextRow(2, Localization::GetText(U"balance_edit.row.repeatable"), card->repeatable ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"), data.smallFont);
   drawTextRow(3, Localization::GetText(U"balance_edit.row.test_owned"), hasTestOwnedCard(card->id) ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"), data.smallFont);
}

void BalanceEditScene::drawHelpPanel() const
{
	const auto help = getHoveredHelpText();
	const auto& data = getData();
	const RectF panel = getHelpPanelRect();
	panel.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	panel.drawFrame(1, ColorF{ 0.32, 0.44, 0.64 });
   data.smallFont(Localization::GetText(U"balance_edit.help_panel.title")).draw(panel.x + 12, panel.y + 8, ColorF{ 0.82, 0.90, 1.0 });
	data.smallFont(help.title).draw(panel.x + 12, panel.y + 30, Palette::White);
	data.smallFont(help.body).draw(panel.x + 12, panel.y + 54, ColorF{ 0.88, 0.92, 0.98 });
}

void BalanceEditScene::drawIntRow(const int32 rowIndex, const String& label, const int32 value, const Font& font) const
{
	drawAdjustmentRow(getEditorRowRect(rowIndex), label, Format(value), font);
}

void BalanceEditScene::drawDoubleRow(const int32 rowIndex, const String& label, const double value, const Font& font) const
{
	drawAdjustmentRow(getEditorRowRect(rowIndex), label, Format(value), font);
}

void BalanceEditScene::drawTextRow(const int32 rowIndex, const String& label, const String& valueText, const Font& font) const
{
	drawAdjustmentRow(getEditorRowRect(rowIndex), label, valueText, font);
}

void BalanceEditScene::drawAdjustmentRow(const RectF& rowRect, const String& label, const String& valueText, const Font& font) const
{
	rowRect.draw(ColorF{ 0.14, 0.18, 0.24, 0.96 });
	rowRect.drawFrame(1, ColorF{ 0.32, 0.44, 0.64 });
    font(label).draw(rowRect.x + 12, rowRect.y + 5, Palette::White);
	font(valueText).draw(rowRect.x + 220, rowRect.y + 5, ColorF{ 0.90, 0.94, 1.0 });
	const auto buttons = getAdjustmentButtonRects(rowRect);
	const auto& data = getData();
	drawButton(buttons.minusLarge, U"-L", data.smallFont);
	drawButton(buttons.minusSmall, U"-S", data.smallFont);
	drawButton(buttons.plusSmall, U"+S", data.smallFont);
	drawButton(buttons.plusLarge, U"+L", data.smallFont);
}

BalanceEditScene::AdjustmentButtonRects BalanceEditScene::getAdjustmentButtonRects(const RectF& rowRect) const
{
  const double buttonWidth = 44;
	const double buttonHeight = 24;
	const double gap = 6;
	const double right = rowRect.x + rowRect.w - 12;
	return AdjustmentButtonRects{
       RectF{ right - ((buttonWidth + gap) * 4), rowRect.y + 3, buttonWidth, buttonHeight },
		RectF{ right - ((buttonWidth + gap) * 3), rowRect.y + 3, buttonWidth, buttonHeight },
		RectF{ right - ((buttonWidth + gap) * 2), rowRect.y + 3, buttonWidth, buttonHeight },
		RectF{ right - (buttonWidth + gap), rowRect.y + 3, buttonWidth, buttonHeight },
	};
}
