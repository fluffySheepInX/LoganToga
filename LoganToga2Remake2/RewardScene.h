#pragma once

#include "AudioManager.h"
#include "GameData.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "RewardUiLayout.h"
#include "SceneTransition.h"

class RewardScene : public SceneBase
{
public:
	explicit RewardScene(const SceneBase::InitData& init)
		: SceneBase{ init }
	{
		PlayMenuBgm();
	}

	void update() override
	{
		if (UpdateSceneTransition(getData(), [this](const String& sceneName)
		{
			changeScene(sceneName);
		}))
		{
			return;
		}

		auto& runState = getData().runState;
		if (!runState.isActive)
		{
			RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		if (m_selectedCardIndex)
		{
			m_selectionEffectTime += Scene::DeltaTime();
			if (m_selectionEffectTime >= SelectionEffectDuration)
			{
				finishSelectedCard();
			}
			return;
		}

		if (runState.pendingRewardCardIds.isEmpty())
		{
			++runState.currentBattleIndex;
			SaveContinueRun(getData(), ContinueResumeScene::Battle);
			RequestSceneTransition(getData(), U"Battle", [this](const String& sceneName)
			{
				changeScene(sceneName);
			});
			return;
		}

		for (int32 index = 0; index < static_cast<int32>(runState.pendingRewardCardIds.size()); ++index)
		{
			const RectF cardRect = RewardUi::GetCardRect(RewardUi::GetRewardUiLayout(), index);
			if (IsMenuButtonClicked(cardRect))
			{
				chooseCard(index);
				return;
			}
		}

		if (Key1.down())
		{
			chooseCard(0);
			return;
		}
		if (Key2.down())
		{
			chooseCard(1);
			return;
		}
		if (Key3.down())
		{
			chooseCard(2);
			return;
		}
	}

	void draw() const override
	{
		RewardUi::DrawRewardSelectionScreen(getData(), getData().runState, m_selectedCardIndex, m_selectionEffectTime, RewardUi::GetRewardUiLayout());
		DrawSceneTransitionOverlay(getData());
	}

private:
	static constexpr double SelectionEffectDuration = 0.42;
	Optional<int32> m_selectedCardIndex;
	double m_selectionEffectTime = 0.0;

	void chooseCard(const int32 index)
	{
		auto& data = getData();
		auto& runState = data.runState;
		if (m_selectedCardIndex || (index < 0) || (index >= static_cast<int32>(runState.pendingRewardCardIds.size())))
		{
			return;
		}

		m_selectedCardIndex = index;
		m_selectionEffectTime = 0.0;
	}

	void finishSelectedCard()
	{
		auto& data = getData();
		auto& runState = data.runState;
		if (!m_selectedCardIndex)
		{
			return;
		}

		const int32 index = *m_selectedCardIndex;
		m_selectedCardIndex.reset();
		m_selectionEffectTime = 0.0;
		if ((index < 0) || (index >= static_cast<int32>(runState.pendingRewardCardIds.size())))
		{
			return;
		}

		ApplyRewardCardChoice(runState, data.rewardCards, runState.pendingRewardCardIds[index]);
		++runState.currentBattleIndex;
		SaveContinueRun(data, ContinueResumeScene::Battle);
		RequestSceneTransition(data, U"Battle", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}
};
