#include "BalanceEditScene.h"

void BalanceEditScene::handleCoreInput()
{
	bool changed = false;
	changed = handleIntRowInput(0, m_editConfig.playerGold, 10, 50, 0) || changed;
	changed = handleIntRowInput(1, m_editConfig.enemyGold, 10, 50, 0) || changed;
	changed = handleDoubleRowInput(2, m_editConfig.income.interval, 0.1, 0.5, 0.1, 10.0) || changed;
	changed = handleIntRowInput(3, m_editConfig.income.playerAmount, 1, 5, 0) || changed;
	changed = handleIntRowInput(4, m_editConfig.income.enemyAmount, 1, 5, 0) || changed;
	changed = handleDoubleRowInput(5, m_editConfig.enemySpawn.interval, 0.1, 0.5, 0.1, 30.0) || changed;
	changed = handleDoubleRowInput(6, m_editConfig.enemySpawn.advancedProbability, 0.05, 0.2, 0.0, 1.0) || changed;
	changed = handleDoubleRowInput(7, m_editConfig.enemySpawn.randomYOffset, 5.0, 20.0, 0.0, 1000.0) || changed;
	changed = handleIntRowInput(8, m_editConfig.enemyAI.assaultUnitThreshold, 1, 2, 1) || changed;
	changed = handleIntRowInput(9, m_editConfig.enemyAI.stagingAssaultMinUnits, 1, 2, 1) || changed;
	applyEditedState(changed);
}

void BalanceEditScene::handleCardInput()
{
	auto* card = getSelectedCardDefinition();
	if (!card)
	{
		return;
	}

	bool changed = false;
	changed = handleDoubleRowInput(0, card->value, 0.1, 1.0, -9999.0, 9999.0) || changed;
	changed = handleCardRarityInput(1, card->rarity) || changed;
	changed = handleCardRepeatableInput(2, card->repeatable) || changed;
	applyEditedState(changed);
}

void BalanceEditScene::handleUnitInput()
{
	auto* unit = getSelectedUnitDefinition();
	if (!unit)
	{
		return;
	}

	bool changed = false;
	int32 row = 0;
	changed = handleIntRowInput(row++, unit->hp, 1, 10, 1) || changed;
	changed = handleIntRowInput(row++, unit->attackPower, 1, 2, 0) || changed;
	changed = handleIntRowInput(row++, unit->cost, 1, 10, 0) || changed;
	if (auto* slot = getSelectedProductionSlot())
	{
		changed = handleSelectedProductionCostInput(row++, 1, 10) || changed;
		changed = handleIntRowInput(row++, slot->batchCount, 1, 1, 1) || changed;
	}
	changed = handleDoubleRowInput(row++, unit->productionTime, 0.1, 0.5, 0.0, 60.0) || changed;
	changed = handleDoubleRowInput(row++, unit->moveSpeed, 1.0, 10.0, 0.0, 1000.0) || changed;
	changed = handleDoubleRowInput(row++, unit->attackRange, 1.0, 10.0, 0.0, 1000.0) || changed;
	changed = handleDoubleRowInput(row++, unit->attackCooldown, 0.05, 0.25, 0.05, 30.0) || changed;
	changed = handleDoubleRowInput(row++, unit->aggroRange, 1.0, 10.0, 0.0, 1000.0) || changed;
	applyEditedState(changed);
}

bool BalanceEditScene::handleUnitListInput()
{
	for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
	{
		if (!isButtonClicked(getUnitButtonRect(static_cast<int32>(index))))
		{
			continue;
		}

		m_selectedUnitIndex = static_cast<int32>(index);
		return true;
	}

	return false;
}

bool BalanceEditScene::handleCardListInput()
{
	for (size_t index = 0; index < m_editCards.size(); ++index)
	{
		if (!isButtonClicked(getCardButtonRect(static_cast<int32>(index))))
		{
			continue;
		}

		m_selectedCardIndex = static_cast<int32>(index);
		return true;
	}

	return false;
}

bool BalanceEditScene::handleSelectedProductionCostInput(const int32 rowIndex, const int32 smallStep, const int32 largeStep)
{
	auto* slot = getSelectedProductionSlot();
	if (!slot)
	{
		return false;
	}

	int32 value = getSelectedProductionCost();
	if (!handleIntRowInput(rowIndex, value, smallStep, largeStep, 0))
	{
		return false;
	}

	slot->cost = value;
	return true;
}

int32 BalanceEditScene::getSelectedProductionCost() const
{
	const auto* unit = getSelectedUnitDefinition();
	const auto* slot = getSelectedProductionSlot();
	if (!unit)
	{
		return 0;
	}
	if (!slot)
	{
		return unit->cost;
	}

	return (slot->cost > 0) ? slot->cost : unit->cost;
}

bool BalanceEditScene::handleIntRowInput(const int32 rowIndex, int32& value, const int32 smallStep, const int32 largeStep, const int32 minValue)
{
	const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
	const int32 originalValue = value;
	if (isButtonClicked(buttons.minusLarge))
	{
		value = Max(minValue, value - largeStep);
	}
	if (isButtonClicked(buttons.minusSmall))
	{
		value = Max(minValue, value - smallStep);
	}
	if (isButtonClicked(buttons.plusSmall))
	{
		value = Max(minValue, value + smallStep);
	}
	if (isButtonClicked(buttons.plusLarge))
	{
		value = Max(minValue, value + largeStep);
	}
	return value != originalValue;
}

bool BalanceEditScene::handleCardRarityInput(const int32 rowIndex, RewardCardRarity& rarity)
{
	const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
	const RewardCardRarity original = rarity;
	if (isButtonClicked(buttons.minusLarge) || isButtonClicked(buttons.minusSmall))
	{
		switch (rarity)
		{
		case RewardCardRarity::Common:
			rarity = RewardCardRarity::Epic;
			break;
		case RewardCardRarity::Rare:
			rarity = RewardCardRarity::Common;
			break;
		case RewardCardRarity::Epic:
		default:
			rarity = RewardCardRarity::Rare;
			break;
		}
	}
	if (isButtonClicked(buttons.plusSmall) || isButtonClicked(buttons.plusLarge))
	{
		switch (rarity)
		{
		case RewardCardRarity::Common:
			rarity = RewardCardRarity::Rare;
			break;
		case RewardCardRarity::Rare:
			rarity = RewardCardRarity::Epic;
			break;
		case RewardCardRarity::Epic:
		default:
			rarity = RewardCardRarity::Common;
			break;
		}
	}
	return rarity != original;
}

bool BalanceEditScene::handleCardRepeatableInput(const int32 rowIndex, bool& repeatable)
{
	const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
	if (!(isButtonClicked(buttons.minusLarge) || isButtonClicked(buttons.minusSmall) || isButtonClicked(buttons.plusSmall) || isButtonClicked(buttons.plusLarge)))
	{
		return false;
	}

	repeatable = !repeatable;
	return true;
}

bool BalanceEditScene::handleDoubleRowInput(const int32 rowIndex, double& value, const double smallStep, const double largeStep, const double minValue, const double maxValue)
{
	const auto buttons = getAdjustmentButtonRects(getEditorRowRect(rowIndex));
	const double originalValue = value;
	if (isButtonClicked(buttons.minusLarge))
	{
		value = Clamp(value - largeStep, minValue, maxValue);
	}
	if (isButtonClicked(buttons.minusSmall))
	{
		value = Clamp(value - smallStep, minValue, maxValue);
	}
	if (isButtonClicked(buttons.plusSmall))
	{
		value = Clamp(value + smallStep, minValue, maxValue);
	}
	if (isButtonClicked(buttons.plusLarge))
	{
		value = Clamp(value + largeStep, minValue, maxValue);
	}
	return value != originalValue;
}
