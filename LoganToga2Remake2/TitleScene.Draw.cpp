#include "TitleScene.h"

void TitleScene::draw() const
{
	//Scene::Rect().draw(ColorF{ 0.08, 0.10, 0.14 });
	getPanelRect().draw(ColorF{ 0.13, 0.16, 0.20 });
	getPanelRect().drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

	const auto& data = getData();
	const bool hasContinue = m_hasContinue;
	const auto& continuePreview = m_continuePreview;
	const TitleUiLayout layout = TitleUi::GetTitleUiLayout();
	data.titleFont(U"LoganToga2Remake2").drawAt(layout.titlePos, Palette::White);
	data.uiFont(U"RTS run prototype").drawAt(layout.subtitlePos, ColorF{ 0.75, 0.86, 1.0 });
	data.smallFont(U"・3-5 battles per run").drawAt(layout.summaryLine1Pos, Palette::White);
	data.smallFont(U"・Choose 1 of 3 reward cards after each victory").drawAt(layout.summaryLine2Pos, Palette::White);
	data.smallFont(U"・Lose once and the run ends").drawAt(layout.summaryLine3Pos, Palette::White);
	data.smallFont(s3d::Format(U"・Viewed bonus rooms: ", data.bonusRoomProgress.viewedRoomIds.size(), U" / ", data.bonusRooms.size())).drawAt(layout.viewedBonusRoomsPos, Palette::White);
	data.smallFont(hasContinue ? U"Press Enter to continue the saved run" : U"Press Enter to start a new run").drawAt(layout.enterHintPos, Palette::Yellow);
	if (hasContinue)
	{
		drawButton(getContinueButtonRect(), U"Continue", data.uiFont, true);
		if (continuePreview)
		{
			drawContinuePreview(*continuePreview, data);
		}
	}
	if (!data.bonusRoomProgress.viewedRoomIds.isEmpty())
	{
		data.smallFont(U"Bonus Rooms can be revisited from this menu").drawAt(TitleUi::GetBonusRoomHintPos(layout, hasContinue), ColorF{ 1.0, 0.88, 0.55 });
		drawButton(getBonusButtonRect(hasContinue), U"Bonus Rooms", data.uiFont);
	}

	drawButton(getTutorialButtonRect(hasContinue), U"Tutorial", data.uiFont, true);
	drawButton(getQuickGuideButtonRect(hasContinue), U"クイック操作説明", data.uiFont, true);
	drawButton(getStartButtonRect(hasContinue), hasContinue ? U"New Run" : U"Start Run", data.uiFont);
	data.smallFont(U"チュートリアル前に基本操作を確認").drawAt(TitleUi::GetQuickGuideHintPos(layout, hasContinue), ColorF{ 0.88, 0.92, 1.0 });

	const s3d::Size resolutionSize = GetWindowResolutionSize(data.displaySettings.resolutionPreset);
	data.smallFont(U"解像度").draw(getResolutionLabelPos(), Palette::White);
	data.smallFont(s3d::Format(U"現在: ", GetWindowResolutionLabel(data.displaySettings.resolutionPreset), U" (", resolutionSize.x, U"x", resolutionSize.y, U")"))
		.draw(getResolutionLabelPos().movedBy(0, 24), ColorF{ 0.85, 0.92, 1.0 });

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
	data.smallFont(U"セーブ保存先").draw(getSaveLocationLabelPos(), Palette::White);
	data.smallFont(U"現在: " + GetContinueRunSaveLocationLabel(saveLocation)).draw(getSaveLocationLabelPos().movedBy(0, 24), ColorF{ 0.85, 0.92, 1.0 });
	drawButton(getSaveLocationButtonRect(), U"Local ⇔ AppData", data.smallFont, true);
	data.smallFont(U"データ管理").drawAt(Vec2{ Scene::CenterF().x, clearContinueButtonRect.y - 18 }, ColorF{ 0.90, 0.94, 1.0 });
	drawButton(clearContinueButtonRect, U"セーブ削除", data.smallFont, hasContinue);
	drawButton(clearSettingsButtonRect, U"設定初期化", data.smallFont, true);
	data.smallFont(U"現在の保存先のみ削除 / 設定は既定値へ戻ります").drawAt(Vec2{ Scene::CenterF().x, clearContinueButtonRect.bottomY() + 18 }, ColorF{ 0.82, 0.88, 0.96 });

#ifdef _DEBUG
	data.smallFont(U"DEBUG: Start with all unlockable units/buildings").drawAt(Vec2{ Scene::CenterF().x, getDebugButtonRect(hasContinue).y - 24 }, ColorF{ 1.0, 0.75, 0.45 });
	drawButton(getDebugButtonRect(hasContinue), U"Debug Full Unlock", data.uiFont, true);
	drawButton(getMapEditButtonRect(), U"Map Edit", data.smallFont);
	drawButton(getBalanceEditButtonRect(), U"Balance Edit", data.smallFont);
	drawButton(getTransitionPresetButtonRect(), U"Fade: " + GetSceneTransitionPresetLabel(data.sceneTransitionSettings.preset), data.smallFont, true);
	drawButton(getTitleUiEditorButtonRect(), U"Title UI Editor", data.smallFont, true);
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
		return s3d::Format(U"Reward after battle ", preview.currentBattleIndex + 1, U"/", preview.totalBattles);
	case ContinueResumeScene::BonusRoom:
		return U"Bonus Room after clear";
	case ContinueResumeScene::Battle:
	default:
		return s3d::Format(U"Battle ", preview.currentBattleIndex + 1, U"/", preview.totalBattles);
	}
}

String TitleScene::getContinuePreviewDetail(const ContinueRunPreview& preview)
{
	switch (preview.resumeScene)
	{
	case ContinueResumeScene::Reward:
		return s3d::Format(U"Reward choices: ", preview.pendingRewardCardCount);
	case ContinueResumeScene::BonusRoom:
		return preview.isCleared ? U"Run cleared" : U"Clear reward available";
	case ContinueResumeScene::Battle:
	default:
		return U"Resume from battle start checkpoint";
	}
}

void TitleScene::drawContinuePreview(const ContinueRunPreview& preview, const GameData& data)
{
	const RectF rect = getContinuePreviewRect();
	rect.draw(ColorF{ 0.09, 0.12, 0.18, 0.96 });
	rect.drawFrame(2, ColorF{ 0.42, 0.60, 0.92 });
	data.smallFont(U"CONTINUE").draw(rect.x + 14, rect.y + 10, ColorF{ 0.82, 0.90, 1.0 });
	data.smallFont(getContinuePreviewHeadline(preview)).draw(rect.x + 14, rect.y + 32, Palette::White);
	data.smallFont(getContinuePreviewDetail(preview)).draw(rect.x + 14, rect.y + 52, ColorF{ 0.86, 0.90, 0.96 });
	data.smallFont(s3d::Format(U"Cards selected: ", preview.selectedCardCount)).draw(rect.x + 14, rect.y + 72, Palette::Gold);
}

void TitleScene::drawQuickGuide(const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF panelRect = getQuickGuidePanelRect();
	panelRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	panelRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
	data.titleFont(U"クイック操作説明").drawAt(panelRect.center().movedBy(0, -168), Palette::White);
	data.smallFont(U"まずはここだけ覚えればOK。詳しくは Tutorial で確認できます。")
		.drawAt(panelRect.center().movedBy(0, -128), ColorF{ 0.84, 0.90, 1.0 });

	const Vec2 left = panelRect.pos.movedBy(38, 84);
	const double lineStep = 34.0;
	data.uiFont(U"1. 左クリックでユニットや建物を選択").draw(left, Palette::White);
	data.uiFont(U"2. Shift + 左クリックで選択追加").draw(left.movedBy(0, lineStep), Palette::White);
	data.uiFont(U"3. 右クリックで移動 / 攻撃指示").draw(left.movedBy(0, lineStep * 2), Palette::White);
	data.uiFont(U"4. 下のコマンドパネルで生産 / 建築 / 強化").draw(left.movedBy(0, lineStep * 3), Palette::White);
	data.uiFont(U"5. Esc でポーズ。困ったら一度止める").draw(left.movedBy(0, lineStep * 4), Palette::White);
	data.uiFont(U"6. 勝利後は報酬カードを1枚選んで次の戦闘へ").draw(left.movedBy(0, lineStep * 5), Palette::White);
	data.uiFont(U"7. 敗北するとラン終了。再挑戦はタイトルから").draw(left.movedBy(0, lineStep * 6), Palette::White);

	data.smallFont(U"流れ: 選択 → 右クリックで行動 → 建築と生産で戦力を増やす → 勝利後に強化")
		.drawAt(panelRect.center().movedBy(0, 118), ColorF{ 1.0, 0.90, 0.58 });
	drawButton(getQuickGuideTutorialButtonRect(), U"Tutorial へ", data.uiFont, true);
	drawButton(getQuickGuideCloseButtonRect(), U"閉じる", data.uiFont);
	data.smallFont(U"Esc でも閉じられます").drawAt(panelRect.center().movedBy(0, 200), ColorF{ 0.80, 0.87, 0.95 });
}

void TitleScene::drawDataClearDialog(const DataClearAction action, const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF dialogRect = getDataClearDialogRect();
	dialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	dialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });

	const bool isContinueSave = (action == DataClearAction::ContinueRunSave);
	data.uiFont(isContinueSave ? U"セーブデータを削除しますか？" : U"設定ファイルを初期化しますか？")
		.drawAt(dialogRect.center().movedBy(0, -48), Palette::White);
	data.smallFont(isContinueSave ? U"現在の保存先にある continue データを削除します" : U"解像度・フルスクリーン・音量を既定値へ戻します")
		.drawAt(dialogRect.center().movedBy(0, -8), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"この操作はタイトルメニューからすぐ反映されます").drawAt(dialogRect.center().movedBy(0, 24), ColorF{ 1.0, 0.86, 0.62 });
	drawButton(getDataClearDialogYesButtonRect(), U"はい", data.uiFont, true);
	drawButton(getDataClearDialogNoButtonRect(), U"いいえ", data.uiFont);
	data.smallFont(U"Enter: はい / Esc: いいえ").drawAt(dialogRect.center().movedBy(0, 94), ColorF{ 0.80, 0.87, 0.95 });
}

void TitleScene::drawExitDialog(const GameData& data)
{
	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });

	const RectF dialogRect = getExitDialogRect();
	dialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
	dialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });

	data.uiFont(U"ゲームを終了しますか？").drawAt(dialogRect.center().movedBy(0, -34), Palette::White);
	drawButton(getExitDialogYesButtonRect(), U"はい", data.uiFont, true);
	drawButton(getExitDialogNoButtonRect(), U"いいえ", data.uiFont);
	data.smallFont(U"Enter: はい / Esc: いいえ").drawAt(dialogRect.center().movedBy(0, 70), ColorF{ 0.80, 0.87, 0.95 });
}
