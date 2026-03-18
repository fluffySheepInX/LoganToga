#include "TitleScene.h"

namespace
{
	[[nodiscard]] MenuButtonStyle GetTitleMenuButtonStyle()
	{
		MenuButtonStyle style;
		style.hoverExpand = 4.0;
		style.hoverBorderThickness = 3.5;
		style.hoverFillColor = ColorF{ 0.30, 0.36, 0.47, 0.99 };
		style.selectedHoverFillColor = ColorF{ 0.40, 0.54, 0.82, 0.99 };
		style.hoverFrameColor = ColorF{ 0.88, 0.94, 1.0, 0.99 };
		return style;
	}
}

bool TitleScene::isButtonClicked(const RectF& rect)
{
	return IsMenuButtonClicked(rect);
}

void TitleScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected, GetTitleMenuButtonStyle());
}

RectF TitleScene::getPanelRect()
{
	return TitleUi::GetTitleUiLayout().panelRect;
}

RectF TitleScene::getMenuButtonRect(const double yOffset)
{
	return RectF{ Scene::CenterF().movedBy(-110, yOffset), 220, 36 };
}

RectF TitleScene::getContinueButtonRect()
{
	return TitleUi::GetTitleUiLayout().continueButtonRect;
}

RectF TitleScene::getTutorialButtonRect(const bool hasContinue)
{
	return TitleUi::GetTutorialButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getQuickGuideButtonRect(const bool hasContinue)
{
	return TitleUi::GetQuickGuideButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getStartButtonRect(const bool hasContinue)
{
	return TitleUi::GetStartButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getBonusButtonRect(const bool hasContinue)
{
	return TitleUi::GetBonusButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getDebugButtonRect(const bool hasContinue)
{
	return TitleUi::GetDebugButtonRect(TitleUi::GetTitleUiLayout(), hasContinue);
}

RectF TitleScene::getTitleUiEditorButtonRect()
{
	return TitleUi::GetTitleUiLayout().titleUiEditorButtonRect;
}

RectF TitleScene::getQuickGuidePanelRect()
{
	return TitleUi::GetTitleUiLayout().quickGuidePanelRect;
}

RectF TitleScene::getQuickGuideTutorialButtonRect()
{
	return TitleUi::GetTitleUiLayout().quickGuideTutorialButtonRect;
}

RectF TitleScene::getQuickGuideCloseButtonRect()
{
	return TitleUi::GetTitleUiLayout().quickGuideCloseButtonRect;
}

RectF TitleScene::getDataClearDialogRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogRect;
}

RectF TitleScene::getDataClearDialogYesButtonRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogYesButtonRect;
}

RectF TitleScene::getDataClearDialogNoButtonRect()
{
	return TitleUi::GetTitleUiLayout().dataClearDialogNoButtonRect;
}

RectF TitleScene::getExitDialogRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogRect;
}

RectF TitleScene::getExitDialogYesButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogYesButtonRect;
}

RectF TitleScene::getExitDialogNoButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitDialogNoButtonRect;
}

Vec2 TitleScene::getResolutionLabelPos()
{
	return TitleUi::GetTitleUiLayout().resolutionLabelPos;
}

RectF TitleScene::getResolutionButtonRect(const size_t index)
{
	return TitleUi::GetResolutionButtonRect(TitleUi::GetTitleUiLayout(), index);
}

Vec2 TitleScene::getSaveLocationLabelPos()
{
	return TitleUi::GetTitleUiLayout().saveLocationLabelPos;
}

RectF TitleScene::getSaveLocationButtonRect()
{
	return TitleUi::GetTitleUiLayout().saveLocationButtonRect;
}

RectF TitleScene::getClearContinueRunButtonRect()
{
	return TitleUi::GetTitleUiLayout().clearContinueRunButtonRect;
}

RectF TitleScene::getClearSettingsButtonRect()
{
	return TitleUi::GetTitleUiLayout().clearSettingsButtonRect;
}

RectF TitleScene::getExitButtonRect()
{
	return TitleUi::GetTitleUiLayout().exitButtonRect;
}

RectF TitleScene::getMapEditButtonRect()
{
	return TitleUi::GetTitleUiLayout().mapEditButtonRect;
}

RectF TitleScene::getTransitionPresetButtonRect()
{
	return TitleUi::GetTitleUiLayout().transitionPresetButtonRect;
}

RectF TitleScene::getBalanceEditButtonRect()
{
	return TitleUi::GetTitleUiLayout().balanceEditButtonRect;
}

RectF TitleScene::getRewardEditorButtonRect()
{
	return TitleUi::GetTitleUiLayout().rewardEditorButtonRect;
}

RectF TitleScene::getBonusRoomEditorButtonRect()
{
	return TitleUi::GetTitleUiLayout().bonusRoomEditorButtonRect;
}
