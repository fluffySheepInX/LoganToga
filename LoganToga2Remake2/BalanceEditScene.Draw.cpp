#include "BalanceEditScene.h"

void BalanceEditScene::drawLeftPanel() const
{
	const auto& data = getData();
	const RectF panel = getLeftPanelRect();
	data.smallFont(U"Panel").draw(panel.x + 16, panel.y + 16, Palette::White);
	data.smallFont(m_tab == Tab::Core
		? U"Global economy / AI values"
		: (m_tab == Tab::Units ? U"Select a unit to edit stats and queue values" : U"Select a reward card to edit balance values"))
		.draw(panel.x + 16, panel.y + 40, ColorF{ 0.84, 0.90, 0.98 });

	if (m_tab == Tab::Core)
	{
		data.smallFont(U"Core tab edits values from battle_core.toml").draw(panel.x + 16, panel.y + 84, Palette::White);
		data.smallFont(U"Units tab edits battle_units.toml and player_production slots").draw(panel.x + 16, panel.y + 108, Palette::White);
		data.smallFont(U"Cards tab edits reward card values from cards.toml").draw(panel.x + 16, panel.y + 132, Palette::White);
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
		drawButton(getCardButtonRect(static_cast<int32>(index)), card.name, data.smallFont, static_cast<int32>(index) == m_selectedCardIndex);
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
		data.smallFont(U"No unit selected").draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
		return;
	}

	data.uiFont(toUnitArchetypeDisplayString(unit->archetype)).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
	const auto* slot = getSelectedProductionSlot();
	data.smallFont(slot ? U"Queue values are backed by player_production." : U"This unit has no player production slot.")
		.draw(getEditorPanelRect().x + 220, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

	int32 row = 0;
	drawIntRow(row++, U"HP", unit->hp, data.smallFont);
	drawIntRow(row++, U"Attack", unit->attackPower, data.smallFont);
	drawIntRow(row++, U"Unit Cost", unit->cost, data.smallFont);
	if (slot)
	{
		drawIntRow(row++, U"Queue Cost", getSelectedProductionCost(), data.smallFont);
		drawIntRow(row++, U"Batch Count", slot->batchCount, data.smallFont);
	}
	drawDoubleRow(row++, U"Production Time", unit->productionTime, data.smallFont);
	drawDoubleRow(row++, U"Move Speed", unit->moveSpeed, data.smallFont);
	drawDoubleRow(row++, U"Attack Range", unit->attackRange, data.smallFont);
	drawDoubleRow(row++, U"Attack Cooldown", unit->attackCooldown, data.smallFont);
	drawDoubleRow(row++, U"Aggro Range", unit->aggroRange, data.smallFont);
}

void BalanceEditScene::drawCardPanel() const
{
	const auto& data = getData();
	const auto* card = getSelectedCardDefinition();
	if (!card)
	{
		data.smallFont(U"No card selected").draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 138, Palette::White);
		return;
	}

	data.uiFont(card->name).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 118, Palette::White);
	data.smallFont(card->id).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 146, ColorF{ 0.74, 0.84, 0.98 });
	data.smallFont(toRewardCardEffectTypeDisplayString(card->effectType)).draw(getEditorPanelRect().x + 240, getEditorPanelRect().y + 126, ColorF{ 0.82, 0.90, 1.0 });

	String targetText;
	if (card->effectType == RewardCardEffectType::TurretUpgradeUnlock)
	{
		targetText = card->targetTurretUpgradeType ? toTurretUpgradeTypeDisplayString(*card->targetTurretUpgradeType) : U"No target";
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
	data.smallFont(card->description).draw(getEditorPanelRect().x + 16, getEditorPanelRect().y + 174, ColorF{ 0.86, 0.90, 0.96 });

	drawDoubleRow(0, U"Value", card->value, data.smallFont);
	drawTextRow(1, U"Rarity", toRewardCardRarityDisplayString(card->rarity), data.smallFont);
	drawTextRow(2, U"Repeatable", card->repeatable ? U"Yes" : U"No", data.smallFont);
}

void BalanceEditScene::drawHelpPanel() const
{
	const auto help = getHoveredHelpText();
	const auto& data = getData();
	const RectF panel = getHelpPanelRect();
	panel.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	panel.drawFrame(1, ColorF{ 0.32, 0.44, 0.64 });
	data.smallFont(U"Hover Help").draw(panel.x + 12, panel.y + 8, ColorF{ 0.82, 0.90, 1.0 });
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
