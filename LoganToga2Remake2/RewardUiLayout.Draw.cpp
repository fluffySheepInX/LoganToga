#include "RewardUiLayout.h"

namespace
{
	inline constexpr double RewardSelectionEffectDuration = 0.42;
}

namespace RewardUi
{
	RectF& GetCardRect(RewardUiLayout& layout, const int32 index)
	{
		switch (index)
		{
		case 0:
			return layout.card1Rect;
		case 1:
			return layout.card2Rect;
		case 2:
		default:
			return layout.card3Rect;
		}
	}

	const RectF& GetCardRect(const RewardUiLayout& layout, const int32 index)
	{
		switch (index)
		{
		case 0:
			return layout.card1Rect;
		case 1:
			return layout.card2Rect;
		case 2:
		default:
			return layout.card3Rect;
		}
	}

	MenuButtonStyle MakeRewardCardStyle(const ColorF& rarityColor)
	{
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
		return cardStyle;
	}

	void DrawRewardSelectionScreen(const GameData& data, const RunState& runState, const Optional<int32>& selectedCardIndex, const double selectionEffectTime, const RewardUiLayout& layout)
	{
		Scene::Rect().draw(ColorF{ 0.06, 0.08, 0.12 });
		const double selectionPulse = (0.5 + (0.5 * Math::Sin(Scene::Time() * 14.0)));
		const String titleText = Localization::GetText(U"reward_screen.title", U"報酬を選択", U"Choose Reward");
		const String subtitleText = Localization::FormatText(U"reward_screen.subtitle", U"戦闘 {0} クリア / 1枚選択", U"Battle {0} clear / choose 1 card", runState.currentBattleIndex + 1);
		String hintText = Localization::GetText(U"reward_screen.controls_hint", U"カードをクリック / 1-3 で選択", U"Click a card or press 1-3");
		if (selectedCardIndex)
		{
			hintText = Localization::GetText(U"reward_screen.acquired_hint", U"報酬反映中... 次の戦闘を準備しています", U"Reward acquired... preparing next battle");
		}

		data.titleFont(titleText).drawAt(layout.titlePos, Palette::White);
		data.uiFont(subtitleText).drawAt(layout.subtitlePos, ColorF{ 0.80, 0.88, 1.0 });
		data.smallFont(hintText).drawAt(layout.hintPos, Palette::Yellow);

		for (int32 index = 0; index < static_cast<int32>(runState.pendingRewardCardIds.size()); ++index)
		{
			const RewardCardDefinition* card = FindRewardCardDefinition(data.rewardCards, runState.pendingRewardCardIds[index]);
			if (!card)
			{
				continue;
			}

			const RectF& cardRect = GetCardRect(layout, index);
			const ColorF rarityColor = GetRewardCardRarityColor(card->rarity);
			const bool isSelected = (selectedCardIndex && (*selectedCardIndex == index));
			const bool isDimmed = (selectedCardIndex && !isSelected);
			const MenuButtonVisualState visual = GetMenuButtonVisualState(cardRect, false, MakeRewardCardStyle(rarityColor));
			const double confirmT = selectedCardIndex ? Min(selectionEffectTime / RewardSelectionEffectDuration, 1.0) : 0.0;
			const RectF drawRect = isSelected ? visual.drawRect.stretched(6.0 + (4.0 * selectionPulse)) : visual.drawRect;
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

			if (!selectedCardIndex)
			{
				const String pressText = Localization::FormatText(U"reward_screen.press_slot", U"{0}で選択", U"Press {0}", index + 1);
				data.smallFont(pressText).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
			}
		}

		if (selectedCardIndex)
		{
			const int32 selectedIndex = *selectedCardIndex;
			if ((0 <= selectedIndex) && (selectedIndex < static_cast<int32>(runState.pendingRewardCardIds.size())))
			{
				const RewardCardDefinition* selectedCard = FindRewardCardDefinition(data.rewardCards, runState.pendingRewardCardIds[selectedIndex]);
				if (selectedCard)
				{
					const ColorF rarityColor = GetRewardCardRarityColor(selectedCard->rarity);
					Scene::Rect().draw(ColorF{ rarityColor.r, rarityColor.g, rarityColor.b, 0.05 + (0.06 * selectionPulse) });
					data.uiFont(Localization::GetText(U"reward_screen.acquired_title", U"報酬獲得", U"Reward Acquired")).drawAt(layout.acquiredLabelPos, rarityColor);
					data.smallFont(selectedCard->name).drawAt(layout.acquiredCardNamePos, Palette::White);
				}
			}
		}
	}
}
