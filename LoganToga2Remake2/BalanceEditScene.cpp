#include "BalanceEditScene.h"

#include "ContinueRunSave.h"
#include "Localization.h"
#include "RunCardLogic.h"

namespace
{
	[[nodiscard]] int32 GetMaxBattleNumberForTest(const BattleConfigData& config)
	{
		int32 maxBattleNumber = 1;
		for (const auto& progression : config.enemyProgression)
		{
			maxBattleNumber = Max(maxBattleNumber, progression.battle);
		}

		return maxBattleNumber;
	}
}

BalanceEditScene::BalanceEditScene(const SceneBase::InitData& init)
	: SceneBase{ init }
{
    reloadFromDisk(Localization::GetText(U"balance_edit.status.ready"));
}

void BalanceEditScene::update()
{
	auto& data = getData();
	if (UpdateSceneTransition(data, [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	if ((m_tab == Tab::Core) && isButtonClicked(getCoreTestStartButtonRect()))
	{
		startTestBattle();
		return;
	}

	if (KeyEscape.down() || isButtonClicked(getTopButtonRect(0)))
	{
		RequestSceneTransition(data, U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (isButtonClicked(getTopButtonRect(1)))
	{
      reloadFromDisk(Localization::GetText(U"balance_edit.status.applied_overrides"));
		return;
	}

	if (isButtonClicked(getTopButtonRect(2)))
	{
		if (saveEditorOverrides())
		{
          reloadFromDisk(Localization::GetText(U"balance_edit.status.saved_editor_overrides"));
		}
		else
		{
           m_statusMessage = Localization::GetText(U"balance_edit.status.failed_save_editor_overrides");
		}
		return;
	}

	if (isButtonClicked(getTopButtonRect(4)))
	{
		if (clearAllOverrides())
		{
            reloadFromDisk(Localization::GetText(U"balance_edit.status.cleared_all_overrides"));
		}
		else
		{
         m_statusMessage = Localization::GetText(U"balance_edit.status.failed_clear_all_overrides");
		}
		return;
	}

	if (isButtonClicked(getTopButtonRect(3)))
	{
		if (clearEditorOverrides())
		{
            reloadFromDisk(Localization::GetText(U"balance_edit.status.cleared_editor_overrides"));
		}
		else
		{
          m_statusMessage = Localization::GetText(U"balance_edit.status.failed_clear_editor_overrides");
		}
		return;
	}

	if (isButtonClicked(getTabButtonRect(Tab::Core)))
	{
		m_tab = Tab::Core;
		return;
	}

	if (isButtonClicked(getTabButtonRect(Tab::Progression)))
	{
		m_tab = Tab::Progression;
		return;
	}

	if (isButtonClicked(getTabButtonRect(Tab::Units)))
	{
		m_tab = Tab::Units;
		return;
	}

	if (isButtonClicked(getTabButtonRect(Tab::Cards)))
	{
		m_tab = Tab::Cards;
		return;
	}

 if ((m_tab == Tab::Progression) && handleProgressionListInput())
	{
		return;
	}

	if ((m_tab == Tab::Units) && handleUnitListInput())
	{
		return;
	}

	if ((m_tab == Tab::Cards) && handleCardListInput())
	{
		return;
	}

	if (m_tab == Tab::Core)
	{
		handleCoreInput();
	}
   else if (m_tab == Tab::Progression)
	{
		handleProgressionInput();
	}
	else if (m_tab == Tab::Units)
	{
		handleUnitInput();
	}
	else
	{
		handleCardInput();
	}
}

void BalanceEditScene::draw() const
{
	Scene::Rect().draw(ColorF{ 0.07, 0.09, 0.13 });
	getLeftPanelRect().draw(ColorF{ 0.12, 0.15, 0.20, 0.98 });
	getRightPanelRect().draw(ColorF{ 0.10, 0.13, 0.18, 0.98 });
	getLeftPanelRect().drawFrame(2, ColorF{ 0.34, 0.48, 0.70 });
	getRightPanelRect().drawFrame(2, ColorF{ 0.34, 0.48, 0.70 });

	const auto& data = getData();
    data.uiFont(Localization::GetText(U"balance_edit.title")).draw(getRightPanelRect().x + 16, getRightPanelRect().y + 16, Palette::White);
	data.smallFont(Localization::GetText(U"balance_edit.subtitle"))
		.draw(getRightPanelRect().x + 16, getRightPanelRect().y + 52, ColorF{ 0.82, 0.90, 1.0 });
   data.smallFont(m_hasUnsavedChanges
		? Localization::GetText(U"balance_edit.status.edited_unsaved")
		: Localization::GetText(U"balance_edit.status.synced"))
		.draw(getRightPanelRect().x + 16, getRightPanelRect().y + 74, m_hasUnsavedChanges ? ColorF{ 1.0, 0.92, 0.25 } : ColorF{ 0.72, 0.88, 0.78 });
	data.smallFont(m_statusMessage).draw(getRightPanelRect().x + 16, getRightPanelRect().y + 96, ColorF{ 0.94, 0.94, 0.98 });

   drawButton(getTopButtonRect(0), Localization::GetText(U"balance_edit.button.back"), data.smallFont);
	drawButton(getTopButtonRect(1), Localization::GetText(U"balance_edit.button.apply_overrides"), data.smallFont);
	drawButton(getTopButtonRect(2), Localization::GetText(U"balance_edit.button.save"), data.smallFont, m_hasUnsavedChanges);
	drawButton(getTopButtonRect(3), Localization::GetText(U"balance_edit.button.clear_editor"), data.smallFont);
	drawButton(getTopButtonRect(4), Localization::GetText(U"balance_edit.button.clear_all"), data.smallFont);

   drawButton(getTabButtonRect(Tab::Core), Localization::GetText(U"balance_edit.tab.core"), data.smallFont, m_tab == Tab::Core);
    drawButton(getTabButtonRect(Tab::Progression), Localization::GetText(U"balance_edit.tab.progression"), data.smallFont, m_tab == Tab::Progression);
	drawButton(getTabButtonRect(Tab::Units), Localization::GetText(U"balance_edit.tab.units"), data.smallFont, m_tab == Tab::Units);
	drawButton(getTabButtonRect(Tab::Cards), Localization::GetText(U"balance_edit.tab.cards"), data.smallFont, m_tab == Tab::Cards);

	drawLeftPanel();
	if (m_tab == Tab::Core)
	{
		drawCorePanel();
	}
   else if (m_tab == Tab::Progression)
	{
		drawProgressionPanel();
	}
	else if (m_tab == Tab::Units)
	{
		drawUnitPanel();
	}
	else
	{
		drawCardPanel();
	}

	drawHelpPanel();

	DrawSceneTransitionOverlay(data);
}

void BalanceEditScene::reloadFromDisk(const String& statusMessage)
{
	auto& data = getData();
	ReloadGameConfigData(data);
	m_editConfig = data.baseBattleConfig;
	m_editCards = data.rewardCards;
    m_testOwnedCardIds.remove_if([this](const String& id)
	{
		return !m_editCards.includes_if([&](const RewardCardDefinition& card)
		{
			return card.id == id;
		});
	});
 m_testBattleNumber = Clamp(m_testBattleNumber, 1, GetMaxBattleNumberForTest(m_editConfig));
   m_selectedProgressionIndex = Clamp(m_selectedProgressionIndex, 0, Max(0, static_cast<int32>(m_editConfig.enemyProgression.size()) - 1));
	m_selectedUnitIndex = Clamp(m_selectedUnitIndex, 0, Max(0, static_cast<int32>(m_editConfig.unitDefinitions.size()) - 1));
	m_selectedCardIndex = Clamp(m_selectedCardIndex, 0, Max(0, static_cast<int32>(m_editCards.size()) - 1));
	m_hasUnsavedChanges = false;
	m_statusMessage = statusMessage;
}

void BalanceEditScene::applyEditedState(const bool changed)
{
	if (!changed)
	{
		return;
	}

	m_hasUnsavedChanges = true;
 m_statusMessage = Localization::GetText(U"balance_edit.status.edited_local");
}

void BalanceEditScene::startTestBattle()
{
	auto& data = getData();
	data.baseBattleConfig = m_editConfig;
	data.rewardCards = m_editCards;
	data.battleLaunchMode = BattleLaunchMode::Run;
	BeginNewRun(data.runState, data.baseBattleConfig, data.runState.useDebugFullUnlocks);
	data.runState.currentBattleIndex = (m_testBattleNumber - 1);
	data.runState.totalBattles = Max(3, m_testBattleNumber);
	data.runState.mapProgressionBattles = BuildSequentialRunMapProgressionBattles(data.baseBattleConfig, data.runState.totalBattles);
   data.runState.selectedCardIds = m_testOwnedCardIds;
	data.runState.pendingRewardCardIds.clear();
	ResetBonusRoomSceneState(data.bonusRoomProgress);
	SaveContinueRun(data, ContinueResumeScene::Battle);
	RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}

UnitDefinition* BalanceEditScene::getSelectedUnitDefinition()
{
	if ((m_selectedUnitIndex < 0) || (static_cast<size_t>(m_selectedUnitIndex) >= m_editConfig.unitDefinitions.size()))
	{
		return nullptr;
	}

	return &m_editConfig.unitDefinitions[static_cast<size_t>(m_selectedUnitIndex)];
}

const UnitDefinition* BalanceEditScene::getSelectedUnitDefinition() const
{
	if ((m_selectedUnitIndex < 0) || (static_cast<size_t>(m_selectedUnitIndex) >= m_editConfig.unitDefinitions.size()))
	{
		return nullptr;
	}

	return &m_editConfig.unitDefinitions[static_cast<size_t>(m_selectedUnitIndex)];
}

ProductionSlot* BalanceEditScene::getSelectedProductionSlot()
{
	const auto* unit = getSelectedUnitDefinition();
	if (!unit)
	{
		return nullptr;
	}

	for (auto& slot : m_editConfig.playerProductionSlots)
	{
		if (slot.archetype == unit->archetype)
		{
			return &slot;
		}
	}

	return nullptr;
}

const ProductionSlot* BalanceEditScene::getSelectedProductionSlot() const
{
	const auto* unit = getSelectedUnitDefinition();
	if (!unit)
	{
		return nullptr;
	}

	for (const auto& slot : m_editConfig.playerProductionSlots)
	{
		if (slot.archetype == unit->archetype)
		{
			return &slot;
		}
	}

	return nullptr;
}

RewardCardDefinition* BalanceEditScene::getSelectedCardDefinition()
{
	if ((m_selectedCardIndex < 0) || (static_cast<size_t>(m_selectedCardIndex) >= m_editCards.size()))
	{
		return nullptr;
	}

	return &m_editCards[static_cast<size_t>(m_selectedCardIndex)];
}

const RewardCardDefinition* BalanceEditScene::getSelectedCardDefinition() const
{
	if ((m_selectedCardIndex < 0) || (static_cast<size_t>(m_selectedCardIndex) >= m_editCards.size()))
	{
		return nullptr;
	}

	return &m_editCards[static_cast<size_t>(m_selectedCardIndex)];
}

EnemyProgressionConfig* BalanceEditScene::getSelectedProgressionConfig()
{
	if ((m_selectedProgressionIndex < 0) || (static_cast<size_t>(m_selectedProgressionIndex) >= m_editConfig.enemyProgression.size()))
	{
		return nullptr;
	}

	return &m_editConfig.enemyProgression[static_cast<size_t>(m_selectedProgressionIndex)];
}

const EnemyProgressionConfig* BalanceEditScene::getSelectedProgressionConfig() const
{
	if ((m_selectedProgressionIndex < 0) || (static_cast<size_t>(m_selectedProgressionIndex) >= m_editConfig.enemyProgression.size()))
	{
		return nullptr;
	}

	return &m_editConfig.enemyProgression[static_cast<size_t>(m_selectedProgressionIndex)];
}

bool BalanceEditScene::isButtonClicked(const RectF& rect)
{
	return IsMenuButtonClicked(rect);
}

bool BalanceEditScene::hasTestOwnedCard(const String& id) const
{
	return m_testOwnedCardIds.includes(id);
}

void BalanceEditScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}

RectF BalanceEditScene::getLeftPanelRect()
{
	return RectF{ 12, 12, 220, Scene::Height() - 24 };
}

RectF BalanceEditScene::getRightPanelRect()
{
	return RectF{ 244, 12, Scene::Width() - 256, Scene::Height() - 24 };
}

RectF BalanceEditScene::getEditorPanelRect()
{
	const RectF right = getRightPanelRect();
	return RectF{ right.x + 16, right.y + 112, right.w - 32, right.h - 220 };
}

RectF BalanceEditScene::getHelpPanelRect()
{
	const RectF right = getRightPanelRect();
	return RectF{ right.x + 16, right.y + right.h - 96, right.w - 32, 80 };
}

RectF BalanceEditScene::getTopButtonRect(const int32 index)
{
	const RectF right = getRightPanelRect();
	return RectF{ right.x + right.w - 650 + (index * 130), right.y + 16, 118, 30 };
}

RectF BalanceEditScene::getTabButtonRect(const Tab tab)
{
	const RectF right = getRightPanelRect();
 const int32 index = (tab == Tab::Core)
		? 0
		: ((tab == Tab::Progression)
			? 1
			: ((tab == Tab::Units) ? 2 : 3));
	return RectF{ right.x + 16 + (index * 104), right.y + 112, 100, 30 };
}

RectF BalanceEditScene::getCoreTestStartButtonRect()
{
	const RectF editor = getEditorPanelRect();
	return RectF{ editor.x + editor.w - 150, editor.y + 8, 138, 28 };
}

RectF BalanceEditScene::getProgressionButtonRect(const int32 index)
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 84 + (index * 32), left.w - 32, 28 };
}

RectF BalanceEditScene::getUnitButtonRect(const int32 index)
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 84 + (index * 32), left.w - 32, 28 };
}

RectF BalanceEditScene::getCardButtonRect(const int32 index)
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 84 + (index * 32), left.w - 32, 28 };
}

RectF BalanceEditScene::getEditorRowRect(const int32 index)
{
	const RectF editor = getEditorPanelRect();
   return RectF{ editor.x, editor.y + 44 + (index * 33), editor.w, 30 };
}
