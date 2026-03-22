#include "BalanceEditScene.h"

#include "Localization.h"
#include "RunCardPresentation.h"

void BalanceEditScene::drawLeftPanel() const
{
	const auto& data = getData();
	const RectF panel = getLeftPanelRect();
  data.smallFont(Localization::GetText(U"balance_edit.left_panel.title")).draw(panel.x + 16, panel.y + 16, Palette::White);
	data.smallFont(m_tab == Tab::Core
		? Localization::GetText(U"balance_edit.left_panel.core_summary")
		: (m_tab == Tab::Units
			? Localization::GetText(U"balance_edit.left_panel.units_summary")
			: Localization::GetText(U"balance_edit.left_panel.cards_summary")))
		.draw(panel.x + 16, panel.y + 40, ColorF{ 0.84, 0.90, 0.98 });

	if (m_tab == Tab::Core)
	{
        data.smallFont(Localization::GetText(U"balance_edit.left_panel.core_detail")).draw(panel.x + 16, panel.y + 84, Palette::White);
		data.smallFont(Localization::GetText(U"balance_edit.left_panel.units_detail")).draw(panel.x + 16, panel.y + 108, Palette::White);
		data.smallFont(Localization::GetText(U"balance_edit.left_panel.cards_detail")).draw(panel.x + 16, panel.y + 132, Palette::White);
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
      drawButton(getCardButtonRect(static_cast<int32>(index)), GetRewardCardName(card), data.smallFont, static_cast<int32>(index) == m_selectedCardIndex);
	}
}

void BalanceEditScene::drawCorePanel() const
{
	const auto& data = getData();
	drawIntRow(0, U"Player Gold", m_editConfig.playerGold, data.smallFont);
	drawIntRow(1, U"Enemy Gold", m_editConfig.enemyGold, data.smallFont);
	drawDoubleRow(2, U"Income Interval", m_editConfig.income.interval, data.smallFont);
	drawIntRow(3, U"Player Income", m_editConfig.income.playerAmount, data.smallFont);
	drawIntRow(4, U"Enemy Income", m_editConfig.income.enemyAmount, data.smallFont);
	drawDoubleRow(5, U"Spawn Interval", m_editConfig.enemySpawn.interval, data.smallFont);
	drawDoubleRow(6, U"Advanced Spawn Rate", m_editConfig.enemySpawn.advancedProbability, data.smallFont);
	drawDoubleRow(7, U"Spawn Random Y", m_editConfig.enemySpawn.randomYOffset, data.smallFont);
	drawIntRow(8, U"Assault Threshold", m_editConfig.enemyAI.assaultUnitThreshold, data.smallFont);
	drawIntRow(9, U"Staging Min Units", m_editConfig.enemyAI.stagingAssaultMinUnits, data.smallFont);
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
	font(label).draw(rowRect.x + 12, rowRect.y + 8, Palette::White);
	font(valueText).draw(rowRect.x + 220, rowRect.y + 8, ColorF{ 0.90, 0.94, 1.0 });
	const auto buttons = getAdjustmentButtonRects(rowRect);
	const auto& data = getData();
	drawButton(buttons.minusLarge, U"-L", data.smallFont);
	drawButton(buttons.minusSmall, U"-S", data.smallFont);
	drawButton(buttons.plusSmall, U"+S", data.smallFont);
	drawButton(buttons.plusLarge, U"+L", data.smallFont);
}

BalanceEditScene::AdjustmentButtonRects BalanceEditScene::getAdjustmentButtonRects(const RectF& rowRect) const
{
	const double buttonWidth = 50;
	const double buttonHeight = 28;
	const double gap = 8;
	const double right = rowRect.x + rowRect.w - 12;
	return AdjustmentButtonRects{
		RectF{ right - ((buttonWidth + gap) * 4), rowRect.y + 6, buttonWidth, buttonHeight },
		RectF{ right - ((buttonWidth + gap) * 3), rowRect.y + 6, buttonWidth, buttonHeight },
		RectF{ right - ((buttonWidth + gap) * 2), rowRect.y + 6, buttonWidth, buttonHeight },
		RectF{ right - (buttonWidth + gap), rowRect.y + 6, buttonWidth, buttonHeight },
	};
}
