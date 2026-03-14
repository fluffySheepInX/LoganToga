#pragma once

#include "GameData.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class RewardScene : public SceneBase
{
public:
	explicit RewardScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

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
			const RectF cardRect = getCardRect(index);
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
		Scene::Rect().draw(ColorF{ 0.06, 0.08, 0.12 });
		const auto& data = getData();
		const auto& runState = data.runState;
		data.titleFont(U"Choose Reward").drawAt(Scene::CenterF().movedBy(0, -260), Palette::White);
		data.uiFont(s3d::Format(U"Battle ", runState.currentBattleIndex + 1, U" clear / choose 1 card")).drawAt(Scene::CenterF().movedBy(0, -210), ColorF{ 0.80, 0.88, 1.0 });
		data.smallFont(U"Click a card or press 1-3").drawAt(Scene::CenterF().movedBy(0, -176), Palette::Yellow);

		for (int32 index = 0; index < static_cast<int32>(runState.pendingRewardCardIds.size()); ++index)
		{
			const auto* card = FindRewardCardDefinition(data.rewardCards, runState.pendingRewardCardIds[index]);
			if (!card)
			{
				continue;
			}

			const RectF cardRect = getCardRect(index);
			const ColorF rarityColor = GetRewardCardRarityColor(card->rarity);
			MenuButtonStyle cardStyle;
			cardStyle.cornerRadius = 18.0;
			cardStyle.hoverExpand = 3.0;
			cardStyle.pressOffsetX = 1.0;
			cardStyle.pressOffsetY = 3.0;
			cardStyle.pressInsetX = 2.0;
			cardStyle.pressInsetY = 5.0;
			cardStyle.baseBorderThickness = 3.0;
			cardStyle.hoverBorderThickness = 5.0;
			cardStyle.fillColor = ColorF{ 0.10, 0.12, 0.18, 0.96 };
			cardStyle.hoverFillColor = ColorF{ 0.14, 0.16, 0.23, 0.98 };
			cardStyle.pressedFillColor = ColorF{ 0.09, 0.11, 0.17, 0.98 };
			cardStyle.frameColor = ColorF{ rarityColor.r * 0.65, rarityColor.g * 0.65, rarityColor.b * 0.65, 0.90 };
			cardStyle.hoverFrameColor = rarityColor;
			cardStyle.drawAccent = false;
			const auto visual = GetMenuButtonVisualState(cardRect, false, cardStyle);
			const RectF drawRect = visual.drawRect;
			RoundRect{ drawRect, 18 }.draw(visual.fillColor);
			RoundRect{ drawRect, 18 }.drawFrame(visual.frameThickness, 0, visual.frameColor);
			RectF{ drawRect.x, drawRect.y, drawRect.w, 14 }.draw(rarityColor);
			data.uiFont(card->name).draw(drawRect.x + 18, drawRect.y + 28, Palette::White);
			data.smallFont(GetRewardCardRarityLabel(card->rarity)).draw(drawRect.x + 18, drawRect.y + 64, rarityColor);
			data.smallFont(card->description).draw(drawRect.x + 18, drawRect.y + 98, ColorF{ 0.90, 0.93, 0.98 });
			data.smallFont(s3d::Format(U"Press ", index + 1)).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
		}

		DrawSceneTransitionOverlay(data);
	}

private:
	[[nodiscard]] static RectF getCardRect(const int32 index)
	{
		const double cardWidth = 300.0;
		const double cardHeight = 260.0;
		const double gap = 28.0;
		const double totalWidth = (cardWidth * 3.0) + (gap * 2.0);
		const double startX = (Scene::Width() - totalWidth) * 0.5;
		return RectF{ startX + (index * (cardWidth + gap)), 240, cardWidth, cardHeight };
	}

	void chooseCard(const int32 index)
	{
		auto& data = getData();
		auto& runState = data.runState;
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
