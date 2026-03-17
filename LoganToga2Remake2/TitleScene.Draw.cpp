#include "TitleScene.h"

#include "TitleUiText.h"

namespace
{
	[[nodiscard]] double ApplyIntroEaseOut(const double value)
	{
		const double t = Clamp(value, 0.0, 1.0);
		const double inverse = 1.0 - t;
		return 1.0 - (inverse * inverse * inverse);
	}
}

void TitleScene::draw() const
{
	//Scene::Rect().draw(ColorF{ 0.08, 0.10, 0.14 });
	getPanelRect().draw(ColorF{ 0.13, 0.16, 0.20 });
	getPanelRect().drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

	const auto& data = getData();
	const bool hasContinue = m_hasContinue;
	const auto& continuePreview = m_continuePreview;
	const TitleUiLayout layout = TitleUi::GetTitleUiLayout();
	const double introProgress = getIntroProgress();
	const double titleProgress = ApplyIntroEaseOut(introProgress);
	const double subtitleProgress = ApplyIntroEaseOut(Clamp((introProgress - 0.10) / 0.90, 0.0, 1.0));
	const double summaryProgress = ApplyIntroEaseOut(Clamp((introProgress - 0.18) / 0.82, 0.0, 1.0));
	data.titleFont(TitleUiText::Title)
		.drawAt(layout.titlePos.movedBy(0, -44.0 * (1.0 - titleProgress)), ColorF{ 1.0, 1.0, 1.0, titleProgress });
	data.uiFont(TitleUiText::Subtitle)
		.drawAt(layout.subtitlePos.movedBy(0, -24.0 * (1.0 - subtitleProgress)), ColorF{ 0.75, 0.86, 1.0, subtitleProgress });
	data.smallFont(TitleUiText::SummaryLines[0])
		.drawAt(layout.summaryLine1Pos.movedBy(0, -14.0 * (1.0 - summaryProgress)), ColorF{ 1.0, 1.0, 1.0, summaryProgress });
	data.smallFont(TitleUiText::SummaryLines[1])
		.drawAt(layout.summaryLine2Pos.movedBy(0, -14.0 * (1.0 - summaryProgress)), ColorF{ 1.0, 1.0, 1.0, summaryProgress });
	data.smallFont(TitleUiText::SummaryLines[2])
		.drawAt(layout.summaryLine3Pos.movedBy(0, -14.0 * (1.0 - summaryProgress)), ColorF{ 1.0, 1.0, 1.0, summaryProgress });
	data.smallFont(s3d::Format(TitleUiText::ViewedBonusRoomsPrefix.get(), data.bonusRoomProgress.viewedRoomIds.size(), U" / ", data.bonusRooms.size())).drawAt(layout.viewedBonusRoomsPos, Palette::White);
	data.smallFont(TitleUiText::GetEnterHintText(hasContinue)).drawAt(layout.enterHintPos, Palette::Yellow);
	if (hasContinue)
	{
		drawButton(getContinueButtonRect(), TitleUiText::ContinueButton, data.uiFont, true);
		if (continuePreview)
		{
			drawContinuePreview(*continuePreview, data);
		}
	}
	if (!data.bonusRoomProgress.viewedRoomIds.isEmpty())
	{
		data.smallFont(TitleUiText::BonusRoomsHint).drawAt(TitleUi::GetBonusRoomHintPos(layout, hasContinue), ColorF{ 1.0, 0.88, 0.55 });
		drawButton(getBonusButtonRect(hasContinue), TitleUiText::BonusRoomsButton, data.uiFont);
	}

	drawButton(getTutorialButtonRect(hasContinue), TitleUiText::TutorialButton, data.uiFont, true);
	drawButton(getQuickGuideButtonRect(hasContinue), TitleUiText::QuickGuideButton, data.uiFont, true);
	drawButton(getStartButtonRect(hasContinue), TitleUiText::GetStartButtonText(hasContinue), data.uiFont);
	data.smallFont(TitleUiText::QuickGuideHint).drawAt(TitleUi::GetQuickGuideHintPos(layout, hasContinue), ColorF{ 0.88, 0.92, 1.0 });

	const s3d::Size resolutionSize = GetWindowResolutionSize(data.displaySettings.resolutionPreset);
	data.smallFont(TitleUiText::ResolutionLabel).draw(getResolutionLabelPos(), Palette::White);
	data.smallFont(s3d::Format(TitleUiText::CurrentPrefix.get(), GetWindowResolutionLabel(data.displaySettings.resolutionPreset), U" (", resolutionSize.x, U"x", resolutionSize.y, U")"))
		.draw(layout.resolutionValuePos, ColorF{ 0.85, 0.92, 1.0 });

	const Array<WindowResolutionPreset> presets =
	{
		WindowResolutionPreset::Small,
		WindowResolutionPreset::Medium,
		WindowResolutionPreset::Large,
	};

	for (size_t i = 0; i < presets.size(); ++i)
	{
		const WindowResolutionPreset preset = presets[i];
		drawButton(getResolutionButtonRect(i), GetWindowResolutionLabel(preset), data.smallFont, data.displaySettings.resolutionPreset == preset);
	}

	const ContinueRunSaveLocation saveLocation = GetContinueRunSaveLocation();
	const RectF clearContinueButtonRect = getClearContinueRunButtonRect();
	const RectF clearSettingsButtonRect = getClearSettingsButtonRect();
	data.smallFont(TitleUiText::SaveLocationLabel).draw(getSaveLocationLabelPos(), Palette::White);
	data.smallFont(TitleUiText::CurrentPrefix.get() + GetContinueRunSaveLocationLabel(saveLocation)).draw(layout.saveLocationValuePos, ColorF{ 0.85, 0.92, 1.0 });
	drawButton(getSaveLocationButtonRect(), TitleUiText::SaveLocationButton, data.smallFont, true);
	data.smallFont(TitleUiText::DataManagementLabel).drawAt(layout.dataManagementLabelPos, ColorF{ 0.90, 0.94, 1.0 });
	drawButton(clearContinueButtonRect, TitleUiText::ClearContinueButton, data.smallFont, hasContinue);
	drawButton(clearSettingsButtonRect, TitleUiText::ClearSettingsButton, data.smallFont, true);
	drawButton(getExitButtonRect(), TitleUiText::ExitButton, data.smallFont);
	data.smallFont(TitleUiText::DataManagementHint).drawAt(layout.dataManagementHintPos, ColorF{ 0.82, 0.88, 0.96 });

#ifdef _DEBUG
	data.smallFont(TitleUiText::DebugUnlockHint).drawAt(hasContinue ? layout.debugHintPosWithContinue : layout.debugHintPosWithoutContinue, ColorF{ 1.0, 0.75, 0.45 });
	drawButton(getDebugButtonRect(hasContinue), TitleUiText::DebugFullUnlockButton, data.uiFont, true);
	drawButton(getMapEditButtonRect(), TitleUiText::MapEditButton, data.smallFont);
	drawButton(getBalanceEditButtonRect(), TitleUiText::BalanceEditButton, data.smallFont);
	drawButton(getTransitionPresetButtonRect(), TitleUiText::TransitionPresetPrefix.get() + GetSceneTransitionPresetLabel(data.sceneTransitionSettings.preset), data.smallFont, true);
	drawButton(getTitleUiEditorButtonRect(), TitleUiText::TitleUiEditorButton, data.smallFont, true);
	drawButton(getRewardEditorButtonRect(), TitleUiText::RewardEditorButton, data.smallFont, true);
#endif
	if (m_isQuickGuideOpen)
	{
		drawQuickGuide(data);
	}

	if (m_dataClearAction != DataClearAction::None)
	{
		drawDataClearDialog(m_dataClearAction, data);
	}

	if (m_isExitDialogOpen)
	{
		drawExitDialog(data);
	}

	DrawSceneTransitionOverlay(data);
}

RectF TitleScene::getContinuePreviewRect()
{
	return TitleUi::GetTitleUiLayout().continuePreviewRect;
}

String TitleScene::getContinuePreviewHeadline(const ContinueRunPreview& preview)
{
	switch (preview.resumeScene)
	{
	case ContinueResumeScene::Reward:
		return Localization::FormatText(U"title.continue_preview.reward_headline", U"戦闘 {0}/{1} 後の報酬", U"Reward after battle {0}/{1}", preview.currentBattleIndex + 1, preview.totalBattles);
	case ContinueResumeScene::BonusRoom:
		return Localization::GetText(U"title.continue_preview.bonus_room_headline", U"クリア後のボーナスルーム", U"Bonus Room after clear");
	case ContinueResumeScene::Battle:
	default:
		return Localization::FormatText(U"title.continue_preview.battle_headline", U"戦闘 {0}/{1}", U"Battle {0}/{1}", preview.currentBattleIndex + 1, preview.totalBattles);
	}
}

String TitleScene::getContinuePreviewDetail(const ContinueRunPreview& preview)
{
	switch (preview.resumeScene)
	{
	case ContinueResumeScene::Reward:
		return Localization::FormatText(U"title.continue_preview.reward_detail", U"報酬候補: {0}", U"Reward choices: {0}", preview.pendingRewardCardCount);
	case ContinueResumeScene::BonusRoom:
		return preview.isCleared
			? Localization::GetText(U"title.continue_preview.bonus_room_cleared", U"ランクリア済み", U"Run cleared")
			: Localization::GetText(U"title.continue_preview.bonus_room_available", U"クリア報酬を受け取れます", U"Clear reward available");
	case ContinueResumeScene::Battle:
	default:
		return Localization::GetText(U"title.continue_preview.battle_detail", U"戦闘開始チェックポイントから再開", U"Resume from battle start checkpoint");
	}
}

void TitleScene::drawContinuePreview(const ContinueRunPreview& preview, const GameData& data)
{
	const RectF rect = getContinuePreviewRect();
	rect.draw(ColorF{ 0.09, 0.12, 0.18, 0.96 });
	rect.drawFrame(2, ColorF{ 0.42, 0.60, 0.92 });
	data.smallFont(TitleUiText::ContinuePreviewTitle).draw(rect.x + 14, rect.y + 10, ColorF{ 0.82, 0.90, 1.0 });
	data.smallFont(getContinuePreviewHeadline(preview)).draw(rect.x + 14, rect.y + 32, Palette::White);
	data.smallFont(getContinuePreviewDetail(preview)).draw(rect.x + 14, rect.y + 52, ColorF{ 0.86, 0.90, 0.96 });
	data.smallFont(s3d::Format(TitleUiText::ContinuePreviewCardsPrefix.get(), preview.selectedCardCount)).draw(rect.x + 14, rect.y + 72, Palette::Gold);
}

void TitleScene::drawQuickGuide(const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF panelRect = getQuickGuidePanelRect();
	const TitleUiLayout layout = TitleUi::GetTitleUiLayout();
	panelRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	panelRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
	data.titleFont(TitleUiText::QuickGuideTitle).drawAt(panelRect.center().movedBy(0, -168), Palette::White);
	data.smallFont(TitleUiText::QuickGuideSubtitle)
		.drawAt(panelRect.center().movedBy(0, -128), ColorF{ 0.84, 0.90, 1.0 });

	const Vec2 left = layout.quickGuideBodyPos;
	const double lineStep = 34.0;
	for (size_t i = 0; i < TitleUiText::QuickGuideBodyLines.size(); ++i)
	{
		data.uiFont(TitleUiText::QuickGuideBodyLines[i]).draw(left.movedBy(0, lineStep * i), Palette::White);
	}

	data.smallFont(TitleUiText::QuickGuideFlow)
		.drawAt(layout.quickGuideFlowPos, ColorF{ 1.0, 0.90, 0.58 });
	drawButton(getQuickGuideTutorialButtonRect(), TitleUiText::QuickGuideTutorialButton, data.uiFont, true);
	drawButton(getQuickGuideCloseButtonRect(), TitleUiText::CloseButton, data.uiFont);
	data.smallFont(TitleUiText::QuickGuideEscHint).drawAt(layout.quickGuideEscHintPos, ColorF{ 0.80, 0.87, 0.95 });
}

void TitleScene::drawDataClearDialog(const DataClearAction action, const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF dialogRect = getDataClearDialogRect();
	dialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	dialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });

	const bool isContinueSave = (action == DataClearAction::ContinueRunSave);
	data.uiFont(isContinueSave ? TitleUiText::DataClearContinueQuestion : TitleUiText::DataClearQuestion)
		.drawAt(dialogRect.center().movedBy(0, -48), Palette::White);
	data.smallFont(isContinueSave ? TitleUiText::DataClearContinueBody : TitleUiText::DataClearBody)
		.drawAt(dialogRect.center().movedBy(0, -8), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(TitleUiText::DataClearImmediateHint).drawAt(dialogRect.center().movedBy(0, 24), ColorF{ 1.0, 0.86, 0.62 });
	drawButton(getDataClearDialogYesButtonRect(), TitleUiText::Yes, data.uiFont, true);
	drawButton(getDataClearDialogNoButtonRect(), TitleUiText::No, data.uiFont);
	data.smallFont(TitleUiText::DialogEnterYesNoHint).drawAt(dialogRect.center().movedBy(0, 94), ColorF{ 0.80, 0.87, 0.95 });
}

void TitleScene::drawExitDialog(const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF dialogRect = getExitDialogRect();
	dialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	dialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });

	data.uiFont(TitleUiText::ExitQuestion).drawAt(dialogRect.center().movedBy(0, -34), Palette::White);
	drawButton(getExitDialogYesButtonRect(), TitleUiText::Yes, data.uiFont, true);
	drawButton(getExitDialogNoButtonRect(), TitleUiText::No, data.uiFont);
	data.smallFont(TitleUiText::DialogEnterYesNoHint).drawAt(dialogRect.center().movedBy(0, 70), ColorF{ 0.80, 0.87, 0.95 });
}
