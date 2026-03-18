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

	[[nodiscard]] String GetLocalLayoutPath();
	[[nodiscard]] String GetAppDataLayoutPath();
	[[nodiscard]] String GetLayoutPathForLocation(ContinueRunSaveLocation location);
	[[nodiscard]] String GetLayoutPath();
	[[nodiscard]] RewardUiLayout MakeDefaultRewardUiLayout();
	void RepairRewardUiLayout(RewardUiLayout& layout);
	[[nodiscard]] RewardUiLayout LoadRewardUiLayoutFromDisk();
	[[nodiscard]] const RewardUiLayout& GetRewardUiLayout();
	[[nodiscard]] RewardUiLayout ReloadRewardUiLayout();
	[[nodiscard]] bool SaveRewardUiLayout(const RewardUiLayout& layout);
	[[nodiscard]] RectF& GetCardRect(RewardUiLayout& layout, int32 index);
	[[nodiscard]] const RectF& GetCardRect(const RewardUiLayout& layout, int32 index);
	[[nodiscard]] MenuButtonStyle MakeRewardCardStyle(const ColorF& rarityColor);
	void DrawRewardSelectionScreen(const GameData& data, const RunState& runState, const Optional<int32>& selectedCardIndex, double selectionEffectTime, const RewardUiLayout& layout);
}
