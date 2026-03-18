#include "RewardEditorScene.h"

#include "AudioManager.h"

RewardEditorScene::RewardEditorScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_layout{ RewardUi::GetRewardUiLayout() }
{
	PlayMenuBgm();
	resetPreviewState();
}

void RewardEditorScene::update()
{
	auto& data = getData();
	if (UpdateSceneTransition(data, [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	handleTopButtonInput();
	handleSelectionInput();
	handleDragInput();

	if (m_selectedCardIndex)
	{
		m_selectionEffectTime += Scene::DeltaTime();
		if (m_selectionEffectTime >= SelectionEffectDuration)
		{
			finishSelectedCardPreview();
		}
		return;
	}

	if (!isCursorOnControlPanel())
	{
		if (Key1.down())
		{
			choosePreviewCard(0);
			return;
		}
		if (Key2.down())
		{
			choosePreviewCard(1);
			return;
		}
		if (Key3.down())
		{
			choosePreviewCard(2);
			return;
		}
	}
}

void RewardEditorScene::initializePreviewRun()
{
	m_previewRunState = {};
	m_previewRunState.isActive = true;
	m_previewRunState.currentBattleIndex = 0;
	m_previewRunState.totalBattles = 4;
	m_previewRunState.mapProgressionBattles = { 4, 2, 3 };
	m_selectedCardIndex.reset();
	m_selectionEffectTime = 0.0;
}

void RewardEditorScene::rebuildPreviewChoices(const String& statusMessage)
{
	auto& data = getData();
	if (data.rewardCards.isEmpty())
	{
		m_previewRunState.pendingRewardCardIds.clear();
		m_statusMessage = Localization::GetText(U"reward_editor.no_cards", U"表示できる報酬カードがありません", U"No reward cards available for preview");
		return;
	}

	m_previewRunState.pendingRewardCardIds = BuildRewardCardChoices(m_previewRunState, data.rewardCards);
	if (m_previewRunState.pendingRewardCardIds.isEmpty() && !m_previewRunState.selectedCardIds.isEmpty())
	{
		m_previewRunState.selectedCardIds.clear();
		m_previewRunState.pendingRewardCardIds = BuildRewardCardChoices(m_previewRunState, data.rewardCards);
	}

	m_statusMessage = statusMessage;
}

void RewardEditorScene::resetPreviewState()
{
	initializePreviewRun();
	rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_reset_preview", U"プレビュー状態を初期化しました", U"Reset preview state"));
}

void RewardEditorScene::choosePreviewCard(const int32 index)
{
	if (m_selectedCardIndex || (index < 0) || (index >= static_cast<int32>(m_previewRunState.pendingRewardCardIds.size())))
	{
		return;
	}

	m_selectedCardIndex = index;
	m_selectionEffectTime = 0.0;
}

void RewardEditorScene::finishSelectedCardPreview()
{
	auto& data = getData();
	if (!m_selectedCardIndex)
	{
		return;
	}

	const int32 index = *m_selectedCardIndex;
	m_selectedCardIndex.reset();
	m_selectionEffectTime = 0.0;
	if ((index < 0) || (index >= static_cast<int32>(m_previewRunState.pendingRewardCardIds.size())))
	{
		return;
	}

	ApplyRewardCardChoice(m_previewRunState, data.rewardCards, m_previewRunState.pendingRewardCardIds[index]);
	m_previewRunState.currentBattleIndex = ((m_previewRunState.currentBattleIndex + 1) % Max(m_previewRunState.totalBattles, 1));
	rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_applied", U"報酬選択をプレビューへ反映しました", U"Applied reward choice to preview"));
}

void RewardEditorScene::requestReturnToTitle()
{
	RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}
