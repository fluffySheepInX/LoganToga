#pragma once

#include "Remake2Common.h"

struct TitleUiLayout
{
	RectF panelRect{ 280, 190, 1040, 520 };
	Vec2 titlePos{ 800, 280 };
	Vec2 subtitlePos{ 800, 350 };
	Vec2 summaryLine1Pos{ 800, 430 };
	Vec2 summaryLine2Pos{ 800, 462 };
	Vec2 summaryLine3Pos{ 800, 494 };
	Vec2 viewedBonusRoomsPos{ 800, 526 };
	Vec2 enterHintPos{ 800, 562 };
	RectF continueButtonRect{ 690, 570, 220, 36 };
	RectF tutorialButtonRectWithContinue{ 690, 622, 220, 36 };
	RectF tutorialButtonRectWithoutContinue{ 690, 570, 220, 36 };
	RectF quickGuideButtonRectWithContinue{ 690, 674, 220, 36 };
	RectF quickGuideButtonRectWithoutContinue{ 690, 622, 220, 36 };
	RectF startButtonRectWithContinue{ 690, 726, 220, 36 };
	RectF startButtonRectWithoutContinue{ 690, 674, 220, 36 };
	RectF bonusButtonRectWithContinue{ 690, 778, 220, 36 };
	RectF bonusButtonRectWithoutContinue{ 690, 726, 220, 36 };
	Vec2 bonusRoomHintPosWithContinue{ 800, 606 };
	Vec2 bonusRoomHintPosWithoutContinue{ 800, 594 };
	RectF debugButtonRectWithContinue{ 690, 830, 220, 36 };
	RectF debugButtonRectWithoutContinue{ 690, 778, 220, 36 };
	RectF continuePreviewRect{ 956, 568, 308, 92 };
	RectF quickGuidePanelRect{ 370, 190, 860, 520 };
	Vec2 quickGuideBodyPos{ 408, 274 };
	Vec2 quickGuideFlowPos{ 800, 568 };
	RectF quickGuideTutorialButtonRect{ 580, 598, 200, 40 };
	RectF quickGuideCloseButtonRect{ 820, 598, 200, 40 };
	Vec2 quickGuideEscHintPos{ 800, 650 };
	RectF dataClearDialogRect{ 540, 345, 520, 210 };
	RectF dataClearDialogYesButtonRect{ 610, 492, 160, 40 };
	RectF dataClearDialogNoButtonRect{ 830, 492, 160, 40 };
	RectF exitDialogRect{ 590, 360, 420, 180 };
	RectF exitDialogYesButtonRect{ 640, 474, 140, 40 };
	RectF exitDialogNoButtonRect{ 820, 474, 140, 40 };
	Vec2 resolutionLabelPos{ 650, 700 };
	Vec2 resolutionValuePos{ 650, 724 };
	RectF resolutionSmallButtonRect{ 750, 696, 96, 32 };
	RectF resolutionMediumButtonRect{ 858, 696, 96, 32 };
	RectF resolutionLargeButtonRect{ 966, 696, 96, 32 };
	Vec2 saveLocationLabelPos{ 1050, 700 };
	Vec2 saveLocationValuePos{ 1050, 724 };
	RectF saveLocationButtonRect{ 1162, 696, 180, 32 };
	Vec2 dataManagementLabelPos{ 800, 628 };
	RectF clearContinueRunButtonRect{ 623, 646, 170, 32 };
	RectF clearSettingsButtonRect{ 807, 646, 170, 32 };
	RectF exitButtonRect{ 991, 646, 170, 32 };
	Vec2 dataManagementHintPos{ 800, 696 };
	Vec2 debugHintPosWithContinue{ 800, 806 };
	Vec2 debugHintPosWithoutContinue{ 800, 754 };
	RectF mapEditButtonRect{ 1170, 208, 128, 30 };
	RectF balanceEditButtonRect{ 1170, 246, 128, 30 };
	RectF transitionPresetButtonRect{ 1106, 284, 192, 30 };
	RectF titleUiEditorButtonRect{ 1106, 322, 192, 30 };
	RectF rewardEditorButtonRect{ 1106, 360, 192, 30 };
	RectF bonusRoomEditorButtonRect{ 1106, 398, 192, 30 };
};
