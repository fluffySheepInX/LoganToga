#pragma once

#include "ContinueRunSave.h"
#include "GameData.h"
#include "MenuButtonUi.h"

struct RewardUiLayout
{
	Vec2 titlePos{ 800, 190 };
	Vec2 subtitlePos{ 800, 240 };
	Vec2 hintPos{ 800, 274 };
	RectF card1Rect{ 322, 240, 300, 260 };
	RectF card2Rect{ 650, 240, 300, 260 };
	RectF card3Rect{ 978, 240, 300, 260 };
	Vec2 acquiredLabelPos{ 800, 810 };
	Vec2 acquiredCardNamePos{ 800, 842 };
};

namespace RewardUi
{
	inline constexpr int32 SchemaVersion = 1;

	[[nodiscard]] inline String GetLocalLayoutPath()
	{
		return U"save/reward_ui_layout.toml";
	}

	[[nodiscard]] inline String GetAppDataLayoutPath()
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(SpecialFolder::LocalAppData), U"LoganToga2Remake2/save/reward_ui_layout.toml");
	}

	[[nodiscard]] inline String GetLayoutPathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataLayoutPath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalLayoutPath();
		}
	}

	[[nodiscard]] inline String GetLayoutPath()
	{
		return GetLayoutPathForLocation(GetContinueRunSaveLocation());
	}

	[[nodiscard]] inline RewardUiLayout MakeDefaultRewardUiLayout()
	{
		return {};
	}

	inline void AppendTomlLine(String& content, const String& key, const String& value)
	{
		content += key + U" = " + value + U"\n";
	}

	inline void AppendVec2Section(String& content, const String& section, const Vec2& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
	}

	inline void AppendRectSection(String& content, const String& section, const RectF& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
		AppendTomlLine(content, U"w", Format(value.w));
		AppendTomlLine(content, U"h", Format(value.h));
	}

	inline void TryLoadVec2Section(const TOMLReader& toml, const String& section, Vec2& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
		}
		catch (const std::exception&)
		{
		}
	}

	inline void TryLoadRectSection(const TOMLReader& toml, const String& section, RectF& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
			value.w = Max(8.0, toml[section][U"w"].get<double>());
			value.h = Max(8.0, toml[section][U"h"].get<double>());
		}
		catch (const std::exception&)
		{
		}
	}

	inline void RepairRectSizeFromDefault(RectF& value, const RectF& defaults, const double minWidth, const double minHeight)
	{
		if (value.w < minWidth)
		{
			value.w = defaults.w;
		}

		if (value.h < minHeight)
		{
			value.h = defaults.h;
		}
	}

	inline void RepairRewardUiLayout(RewardUiLayout& layout)
	{
		const RewardUiLayout defaults = MakeDefaultRewardUiLayout();
		RepairRectSizeFromDefault(layout.card1Rect, defaults.card1Rect, 180.0, 180.0);
		RepairRectSizeFromDefault(layout.card2Rect, defaults.card2Rect, 180.0, 180.0);
		RepairRectSizeFromDefault(layout.card3Rect, defaults.card3Rect, 180.0, 180.0);
	}

	[[nodiscard]] inline String BuildRewardUiLayoutTomlContent(const RewardUiLayout& layout)
	{
		String content;
		AppendTomlLine(content, U"schemaVersion", Format(SchemaVersion));
		AppendVec2Section(content, U"title", layout.titlePos);
		AppendVec2Section(content, U"subtitle", layout.subtitlePos);
		AppendVec2Section(content, U"hint", layout.hintPos);
		AppendRectSection(content, U"card1", layout.card1Rect);
		AppendRectSection(content, U"card2", layout.card2Rect);
		AppendRectSection(content, U"card3", layout.card3Rect);
		AppendVec2Section(content, U"acquiredLabel", layout.acquiredLabelPos);
		AppendVec2Section(content, U"acquiredCardName", layout.acquiredCardNamePos);
		return content;
	}

	[[nodiscard]] inline RewardUiLayout LoadRewardUiLayoutFromDisk()
	{
		RewardUiLayout layout = MakeDefaultRewardUiLayout();
		const TOMLReader toml{ GetLayoutPath() };
		if (!toml)
		{
			return layout;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != SchemaVersion)
			{
				return MakeDefaultRewardUiLayout();
			}
		}
		catch (const std::exception&)
		{
			return MakeDefaultRewardUiLayout();
		}

		TryLoadVec2Section(toml, U"title", layout.titlePos);
		TryLoadVec2Section(toml, U"subtitle", layout.subtitlePos);
		TryLoadVec2Section(toml, U"hint", layout.hintPos);
		TryLoadRectSection(toml, U"card1", layout.card1Rect);
		TryLoadRectSection(toml, U"card2", layout.card2Rect);
		TryLoadRectSection(toml, U"card3", layout.card3Rect);
		TryLoadVec2Section(toml, U"acquiredLabel", layout.acquiredLabelPos);
		TryLoadVec2Section(toml, U"acquiredCardName", layout.acquiredCardNamePos);
		RepairRewardUiLayout(layout);
		return layout;
	}

	[[nodiscard]] inline RewardUiLayout& GetRewardUiLayoutStorage()
	{
		static RewardUiLayout layout = LoadRewardUiLayoutFromDisk();
		return layout;
	}

	[[nodiscard]] inline const RewardUiLayout& GetRewardUiLayout()
	{
		return GetRewardUiLayoutStorage();
	}

	[[nodiscard]] inline RewardUiLayout ReloadRewardUiLayout()
	{
		GetRewardUiLayoutStorage() = LoadRewardUiLayoutFromDisk();
		return GetRewardUiLayoutStorage();
	}

	[[nodiscard]] inline bool SaveRewardUiLayout(const RewardUiLayout& layout)
	{
		const String path = GetLayoutPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildRewardUiLayoutTomlContent(layout));
		GetRewardUiLayoutStorage() = layout;
		return true;
	}

	[[nodiscard]] inline RectF& GetCardRect(RewardUiLayout& layout, const int32 index)
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

	[[nodiscard]] inline const RectF& GetCardRect(const RewardUiLayout& layout, const int32 index)
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

	[[nodiscard]] inline MenuButtonStyle MakeRewardCardStyle(const ColorF& rarityColor)
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

	inline void DrawRewardSelectionScreen(const GameData& data, const RunState& runState, const Optional<int32>& selectedCardIndex, const double selectionEffectTime, const RewardUiLayout& layout)
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
			const double confirmT = selectedCardIndex ? Min(selectionEffectTime / 0.42, 1.0) : 0.0;
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
