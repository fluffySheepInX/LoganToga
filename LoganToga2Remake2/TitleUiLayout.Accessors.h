#pragma once

#include "TitleUiLayout.Types.h"

namespace TitleUi
{
	[[nodiscard]] inline RectF GetTutorialButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.tutorialButtonRectWithContinue : layout.tutorialButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetQuickGuideButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.quickGuideButtonRectWithContinue : layout.quickGuideButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetStartButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.startButtonRectWithContinue : layout.startButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetBonusButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.bonusButtonRectWithContinue : layout.bonusButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetDebugButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.debugButtonRectWithContinue : layout.debugButtonRectWithoutContinue;
	}

	[[nodiscard]] inline Vec2 GetBonusRoomHintPos(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.bonusRoomHintPosWithContinue : layout.bonusRoomHintPosWithoutContinue;
	}

	[[nodiscard]] inline Vec2 GetQuickGuideHintPos(const TitleUiLayout& layout, const bool hasContinue)
	{
		const RectF buttonRect = GetQuickGuideButtonRect(layout, hasContinue);
		return Vec2{ buttonRect.center().x, buttonRect.center().y + 28 };
	}

	[[nodiscard]] inline RectF GetResolutionButtonRect(const TitleUiLayout& layout, const size_t index)
	{
		switch (index)
		{
		case 0:
			return layout.resolutionSmallButtonRect;
		case 2:
			return layout.resolutionLargeButtonRect;
		case 1:
		default:
			return layout.resolutionMediumButtonRect;
		}
	}
}
