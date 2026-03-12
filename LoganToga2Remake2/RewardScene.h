#pragma once

#include "GameData.h"
#include "ContinueRunSave.h"

class RewardScene : public SceneBase
{
public:
	explicit RewardScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

	void update() override
	{
		auto& runState = getData().runState;
		if (!runState.isActive)
		{
			changeScene(U"Title");
			return;
		}

		if (runState.pendingRewardCardIds.isEmpty())
		{
			++runState.currentBattleIndex;
			SaveContinueRun(getData(), ContinueResumeScene::Battle);
			changeScene(U"Battle");
			return;
		}

		for (int32 index = 0; index < static_cast<int32>(runState.pendingRewardCardIds.size()); ++index)
		{
			const RectF cardRect = getCardRect(index);
			if (cardRect.intersects(Cursor::PosF()) && MouseL.down())
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
			const bool isHovered = cardRect.intersects(Cursor::PosF());
			const ColorF rarityColor = GetRewardCardRarityColor(card->rarity);
			const RectF drawRect = isHovered
				? RectF{ cardRect.x, cardRect.y - 6, cardRect.w, cardRect.h + 6 }
				: cardRect;
			RoundRect{ drawRect, 18 }.draw(ColorF{ 0.10, 0.12, 0.18, 0.96 });
			RoundRect{ drawRect, 18 }.drawFrame(isHovered ? 5 : 3, 0, rarityColor);
			RectF{ drawRect.x, drawRect.y, drawRect.w, 14 }.draw(rarityColor);
			data.uiFont(card->name).draw(drawRect.x + 18, drawRect.y + 28, Palette::White);
			data.smallFont(GetRewardCardRarityLabel(card->rarity)).draw(drawRect.x + 18, drawRect.y + 64, rarityColor);
			data.smallFont(card->description).draw(drawRect.x + 18, drawRect.y + 98, ColorF{ 0.90, 0.93, 0.98 });
			data.smallFont(s3d::Format(U"Press ", index + 1)).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
		}
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
		changeScene(U"Battle");
	}
};
