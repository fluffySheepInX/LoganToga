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
		const double selectionPulse = (0.5 + (0.5 * Math::Sin(Scene::Time() * 14.0)));
		data.titleFont(U"Choose Reward").drawAt(Scene::CenterF().movedBy(0, -260), Palette::White);
		data.uiFont(s3d::Format(U"Battle ", runState.currentBattleIndex + 1, U" clear / choose 1 card")).drawAt(Scene::CenterF().movedBy(0, -210), ColorF{ 0.80, 0.88, 1.0 });
		data.smallFont(m_selectedCardIndex ? U"Reward acquired... preparing next battle" : U"Click a card or press 1-3").drawAt(Scene::CenterF().movedBy(0, -176), Palette::Yellow);

		for (int32 index = 0; index < static_cast<int32>(runState.pendingRewardCardIds.size()); ++index)
		{
			const auto* card = FindRewardCardDefinition(data.rewardCards, runState.pendingRewardCardIds[index]);
			if (!card)
			{
				continue;
			}

			const RectF cardRect = getCardRect(index);
			const ColorF rarityColor = GetRewardCardRarityColor(card->rarity);
			const bool isSelected = (m_selectedCardIndex && (*m_selectedCardIndex == index));
			const bool isDimmed = (m_selectedCardIndex && !isSelected);
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
			const double confirmT = m_selectedCardIndex ? Min(m_selectionEffectTime / SelectionEffectDuration, 1.0) : 0.0;
			const RectF drawRect = isSelected
				? visual.drawRect.stretched(6.0 + (4.0 * selectionPulse))
				: visual.drawRect;
			const double contentAlpha = isDimmed ? 0.28 : 1.0;
			if (isSelected)
			{
				RoundRect{ drawRect.stretched(10.0 + (6.0 * selectionPulse)), 24 }.draw(ColorF{ rarityColor.r, rarityColor.g, rarityColor.b, 0.12 + (0.10 * (1.0 - confirmT)) });
			}
			RoundRect{ drawRect, 18 }.draw(ColorF{ visual.fillColor.r, visual.fillColor.g, visual.fillColor.b, visual.fillColor.a * contentAlpha });
			RoundRect{ drawRect, 18 }.drawFrame(visual.frameThickness + (isSelected ? 2.0 : 0.0), 0, ColorF{ visual.frameColor.r, visual.frameColor.g, visual.frameColor.b, visual.frameColor.a * contentAlpha });
			RectF{ drawRect.x, drawRect.y, drawRect.w, 14 }.draw(rarityColor);
			data.uiFont(card->name).draw(drawRect.x + 18, drawRect.y + 28, ColorF{ 1.0, 1.0, 1.0, contentAlpha });
			data.smallFont(GetRewardCardRarityLabel(card->rarity)).draw(drawRect.x + 18, drawRect.y + 64, ColorF{ rarityColor.r, rarityColor.g, rarityColor.b, contentAlpha });
			data.smallFont(card->description).draw(drawRect.x + 18, drawRect.y + 98, ColorF{ 0.90, 0.93, 0.98, contentAlpha });
			if (!m_selectedCardIndex)
			{
				data.smallFont(s3d::Format(U"Press ", index + 1)).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
			}
		}

		if (m_selectedCardIndex)
		{
			const int32 selectedIndex = *m_selectedCardIndex;
			if ((selectedIndex >= 0) && (selectedIndex < static_cast<int32>(runState.pendingRewardCardIds.size())))
			{
				if (const auto* selectedCard = FindRewardCardDefinition(data.rewardCards, runState.pendingRewardCardIds[selectedIndex]))
				{
					const ColorF rarityColor = GetRewardCardRarityColor(selectedCard->rarity);
					Scene::Rect().draw(ColorF{ rarityColor.r, rarityColor.g, rarityColor.b, 0.05 + (0.06 * selectionPulse) });
					data.uiFont(U"Reward Acquired").drawAt(Scene::CenterF().movedBy(0, 360), rarityColor);
					data.smallFont(selectedCard->name).drawAt(Scene::CenterF().movedBy(0, 392), Palette::White);
				}
			}
		}

		DrawSceneTransitionOverlay(data);
	}

private:
	static constexpr double SelectionEffectDuration = 0.42;
	Optional<int32> m_selectedCardIndex;
	double m_selectionEffectTime = 0.0;

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
