#include "TitleUiEditorScene.h"

namespace TitleUiEditorSceneDrawDetail
{
	constexpr ColorF EditorPanelColor{ 0.06, 0.08, 0.12, 0.94 };
	constexpr ColorF EditorPanelFrameColor{ 0.34, 0.50, 0.76, 0.98 };
	constexpr int32 EditorGridCellSize = 8;
	constexpr int32 EditorGridMajorLineSpan = 4;

	void DrawEditorPanel(const RectF& rect)
	{
		rect.draw(EditorPanelColor);
		rect.drawFrame(2, EditorPanelFrameColor);
	}
}

void TitleUiEditorScene::drawPreview() const
{
	Scene::Rect().draw(ColorF{ 0.07, 0.10, 0.14 });
	const RectF leftPanel = getLeftPanelRect();
	const RectF rightPanel = getRightPanelRect();
	const RectF previewGridRect{
		leftPanel.x + leftPanel.w + 12,
		12,
		rightPanel.x - (leftPanel.x + leftPanel.w) - 24,
		Scene::Height() - 24
	};
	for (int32 x = 0; x <= static_cast<int32>(previewGridRect.w); x += TitleUiEditorSceneDrawDetail::EditorGridCellSize)
	{
		const bool isMajorLine = (((x / TitleUiEditorSceneDrawDetail::EditorGridCellSize) % TitleUiEditorSceneDrawDetail::EditorGridMajorLineSpan) == 0);
		const double drawX = previewGridRect.x + x;
		Line{ drawX, previewGridRect.y, drawX, previewGridRect.y + previewGridRect.h }
			.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}
	for (int32 y = 0; y <= static_cast<int32>(previewGridRect.h); y += TitleUiEditorSceneDrawDetail::EditorGridCellSize)
	{
		const bool isMajorLine = (((y / TitleUiEditorSceneDrawDetail::EditorGridCellSize) % TitleUiEditorSceneDrawDetail::EditorGridMajorLineSpan) == 0);
		const double drawY = previewGridRect.y + y;
		Line{ previewGridRect.x, drawY, previewGridRect.x + previewGridRect.w, drawY }
			.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}
	m_layout.panelRect.draw(ColorF{ 0.13, 0.16, 0.20 });
	m_layout.panelRect.drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

	const auto& data = getData();
	const bool hasContinue = m_previewHasContinue;
	const bool hasViewedBonusRooms = m_previewHasViewedBonusRooms;

	data.titleFont(U"LoganToga2Remake2").drawAt(m_layout.titlePos, Palette::White);
	data.uiFont(U"RTS run prototype").drawAt(m_layout.subtitlePos, ColorF{ 0.75, 0.86, 1.0 });
	data.smallFont(U"・3-5 battles per run").drawAt(m_layout.summaryLine1Pos, Palette::White);
	data.smallFont(U"・Choose 1 of 3 reward cards after each victory").drawAt(m_layout.summaryLine2Pos, Palette::White);
	data.smallFont(U"・Lose once and the run ends").drawAt(m_layout.summaryLine3Pos, Palette::White);
	data.smallFont(U"・Viewed bonus rooms: 2 / 6").drawAt(m_layout.viewedBonusRoomsPos, Palette::White);
	data.smallFont(hasContinue ? U"Press Enter to continue the saved run" : U"Press Enter to start a new run")
		.drawAt(m_layout.enterHintPos, Palette::Yellow);

	if (hasContinue)
	{
		drawButton(m_layout.continueButtonRect, U"Continue", data.uiFont, true);
		m_layout.continuePreviewRect.draw(ColorF{ 0.09, 0.12, 0.18, 0.96 });
		m_layout.continuePreviewRect.drawFrame(2, ColorF{ 0.42, 0.60, 0.92 });
		data.smallFont(U"CONTINUE").draw(m_layout.continuePreviewRect.x + 14, m_layout.continuePreviewRect.y + 10, ColorF{ 0.82, 0.90, 1.0 });
		data.smallFont(U"Battle 2/5").draw(m_layout.continuePreviewRect.x + 14, m_layout.continuePreviewRect.y + 32, Palette::White);
		data.smallFont(U"Resume from battle start checkpoint").draw(m_layout.continuePreviewRect.x + 14, m_layout.continuePreviewRect.y + 52, ColorF{ 0.86, 0.90, 0.96 });
		data.smallFont(U"Cards selected: 3").draw(m_layout.continuePreviewRect.x + 14, m_layout.continuePreviewRect.y + 72, Palette::Gold);
	}

	if (hasViewedBonusRooms)
	{
		data.smallFont(U"Bonus Rooms can be revisited from this menu")
			.drawAt(TitleUi::GetBonusRoomHintPos(m_layout, hasContinue), ColorF{ 1.0, 0.88, 0.55 });
		drawButton(TitleUi::GetBonusButtonRect(m_layout, hasContinue), U"Bonus Rooms", data.uiFont);
	}

	drawButton(TitleUi::GetTutorialButtonRect(m_layout, hasContinue), U"Tutorial", data.uiFont, true);
	drawButton(TitleUi::GetQuickGuideButtonRect(m_layout, hasContinue), U"クイック操作説明", data.uiFont, true);
	drawButton(TitleUi::GetStartButtonRect(m_layout, hasContinue), hasContinue ? U"New Run" : U"Start Run", data.uiFont);
	data.smallFont(U"チュートリアル前に基本操作を確認")
		.drawAt(TitleUi::GetQuickGuideHintPos(m_layout, hasContinue), ColorF{ 0.88, 0.92, 1.0 });

	const s3d::Size resolutionSize = GetWindowResolutionSize(data.displaySettings.resolutionPreset);
	data.smallFont(U"解像度").draw(m_layout.resolutionLabelPos, Palette::White);
	data.smallFont(s3d::Format(U"現在: ", GetWindowResolutionLabel(data.displaySettings.resolutionPreset), U" (", resolutionSize.x, U"x", resolutionSize.y, U")"))
		.draw(m_layout.resolutionLabelPos.movedBy(0, 24), ColorF{ 0.85, 0.92, 1.0 });
	drawButton(m_layout.resolutionSmallButtonRect, GetWindowResolutionLabel(WindowResolutionPreset::Small), data.smallFont, false);
	drawButton(m_layout.resolutionMediumButtonRect, GetWindowResolutionLabel(WindowResolutionPreset::Medium), data.smallFont, data.displaySettings.resolutionPreset == WindowResolutionPreset::Medium);
	drawButton(m_layout.resolutionLargeButtonRect, GetWindowResolutionLabel(WindowResolutionPreset::Large), data.smallFont, false);

	data.smallFont(U"セーブ保存先").draw(m_layout.saveLocationLabelPos, Palette::White);
	data.smallFont(U"現在: " + GetContinueRunSaveLocationLabel(GetContinueRunSaveLocation()))
		.draw(m_layout.saveLocationLabelPos.movedBy(0, 24), ColorF{ 0.85, 0.92, 1.0 });
	drawButton(m_layout.saveLocationButtonRect, U"Local ⇔ AppData", data.smallFont, true);
	data.smallFont(U"データ管理")
		.drawAt(Vec2{ Scene::CenterF().x, m_layout.clearContinueRunButtonRect.y - 18 }, ColorF{ 0.90, 0.94, 1.0 });
	drawButton(m_layout.clearContinueRunButtonRect, U"セーブ削除", data.smallFont, hasContinue);
	drawButton(m_layout.clearSettingsButtonRect, U"設定初期化", data.smallFont, true);
	data.smallFont(U"現在の保存先のみ削除 / 設定は既定値へ戻ります")
		.drawAt(Vec2{ Scene::CenterF().x, m_layout.clearContinueRunButtonRect.bottomY() + 18 }, ColorF{ 0.82, 0.88, 0.96 });

#ifdef _DEBUG
	if (m_previewDebugButtons)
	{
		data.smallFont(U"DEBUG: Start with all unlockable units/buildings")
			.drawAt(Vec2{ Scene::CenterF().x, TitleUi::GetDebugButtonRect(m_layout, hasContinue).y - 24 }, ColorF{ 1.0, 0.75, 0.45 });
		drawButton(TitleUi::GetDebugButtonRect(m_layout, hasContinue), U"Debug Full Unlock", data.uiFont, true);
		drawButton(m_layout.mapEditButtonRect, U"Map Edit", data.smallFont);
		drawButton(m_layout.balanceEditButtonRect, U"Balance Edit", data.smallFont);
		drawButton(m_layout.transitionPresetButtonRect, U"Fade: Default", data.smallFont, true);
		drawButton(m_layout.titleUiEditorButtonRect, U"Title UI Editor", data.smallFont, true);
	}
#endif

	if (m_previewQuickGuideOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		m_layout.quickGuidePanelRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		m_layout.quickGuidePanelRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.titleFont(U"クイック操作説明").drawAt(m_layout.quickGuidePanelRect.center().movedBy(0, -168), Palette::White);
		data.smallFont(U"まずはここだけ覚えればOK。詳しくは Tutorial で確認できます。")
			.drawAt(m_layout.quickGuidePanelRect.center().movedBy(0, -128), ColorF{ 0.84, 0.90, 1.0 });
		drawButton(m_layout.quickGuideTutorialButtonRect, U"Tutorial へ", data.uiFont, true);
		drawButton(m_layout.quickGuideCloseButtonRect, U"閉じる", data.uiFont);
	}

	if (m_previewDataClearDialogOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		m_layout.dataClearDialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		m_layout.dataClearDialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.uiFont(U"設定ファイルを初期化しますか？").drawAt(m_layout.dataClearDialogRect.center().movedBy(0, -48), Palette::White);
		data.smallFont(U"解像度・フルスクリーン・音量を既定値へ戻します")
			.drawAt(m_layout.dataClearDialogRect.center().movedBy(0, -8), ColorF{ 0.84, 0.90, 1.0 });
		drawButton(m_layout.dataClearDialogYesButtonRect, U"はい", data.uiFont, true);
		drawButton(m_layout.dataClearDialogNoButtonRect, U"いいえ", data.uiFont);
	}

	if (m_previewExitDialogOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		m_layout.exitDialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		m_layout.exitDialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.uiFont(U"ゲームを終了しますか？").drawAt(m_layout.exitDialogRect.center().movedBy(0, -34), Palette::White);
		drawButton(m_layout.exitDialogYesButtonRect, U"はい", data.uiFont, true);
		drawButton(m_layout.exitDialogNoButtonRect, U"いいえ", data.uiFont);
	}
}

void TitleUiEditorScene::drawPanels() const
{
	const auto& data = getData();
	const RectF leftPanel = getLeftPanelRect();
	const RectF rightPanel = getRightPanelRect();
	TitleUiEditorSceneDrawDetail::DrawEditorPanel(leftPanel);
	TitleUiEditorSceneDrawDetail::DrawEditorPanel(rightPanel);

	data.uiFont(U"Title UI Editor").draw(leftPanel.x + 16, leftPanel.y + 16, Palette::White);
	data.smallFont(m_hasUnsavedChanges ? U"Status: edited / unsaved" : U"Status: synced")
		.draw(leftPanel.x + 16, leftPanel.y + 48, m_hasUnsavedChanges ? ColorF{ 1.0, 0.92, 0.28 } : ColorF{ 0.72, 0.88, 0.78 });

	drawButton(getTopButtonRect(0), U"Back", data.smallFont);
	drawButton(getTopButtonRect(1), U"Save", data.smallFont, m_hasUnsavedChanges);
	drawButton(getTopButtonRect(2), U"Reload", data.smallFont);
	drawButton(getTopButtonRect(3), U"Reset Selected", data.smallFont);
	drawButton(getTopButtonRect(4), U"Reset All", data.smallFont);

	data.smallFont(U"Elements (W/S also works)").draw(leftPanel.x + 16, leftPanel.y + 220, ColorF{ 0.82, 0.90, 1.0 });
	const RectF selectionListRect = getSelectionListRect();
	selectionListRect.draw(ColorF{ 0.10, 0.13, 0.18, 0.96 });
	selectionListRect.drawFrame(1, ColorF{ 0.34, 0.50, 0.76, 0.98 });
	const auto& elements = getEditableElements();
	const int32 visibleRowCount = getSelectionVisibleRowCount();
	for (int32 visibleIndex = 0; visibleIndex < visibleRowCount; ++visibleIndex)
	{
		const int32 actualIndex = (m_selectionScrollRow + visibleIndex);
		if (static_cast<int32>(elements.size()) <= actualIndex)
		{
			break;
		}

		drawButton(getSelectionRowRect(visibleIndex), elements[static_cast<size_t>(actualIndex)].name, data.smallFont, actualIndex == m_selectedElementIndex);
	}

	if (getMaxSelectionScrollRow() > 0)
	{
		const double scrollRatio = (static_cast<double>(m_selectionScrollRow) / getMaxSelectionScrollRow());
		const double thumbHeight = Max(32.0, (selectionListRect.h * visibleRowCount) / static_cast<double>(elements.size()));
		const double thumbRange = Max(0.0, selectionListRect.h - thumbHeight);
		RectF{ selectionListRect.rightX() - 8, selectionListRect.y + (thumbRange * scrollRatio), 4, thumbHeight }
			.draw(ColorF{ 0.70, 0.82, 1.0, 0.85 });
	}

	data.smallFont(U"Preview States").draw(rightPanel.x + 16, rightPanel.y + 16, Palette::White);
	drawButton(getToggleButtonRect(0), U"Continue", data.smallFont, m_previewHasContinue);
	drawButton(getToggleButtonRect(1), U"Bonus", data.smallFont, m_previewHasViewedBonusRooms);
	drawButton(getToggleButtonRect(2), U"QuickGuide", data.smallFont, m_previewQuickGuideOpen);
	drawButton(getToggleButtonRect(3), U"DataClear", data.smallFont, m_previewDataClearDialogOpen);
	drawButton(getToggleButtonRect(4), U"ExitDialog", data.smallFont, m_previewExitDialogOpen);
#ifdef _DEBUG
	drawButton(getToggleButtonRect(5), U"DebugButtons", data.smallFont, m_previewDebugButtons);
#endif

	const RectF infoRect{ rightPanel.x + 16, rightPanel.y + 214, rightPanel.w - 32, rightPanel.h - 230 };
	infoRect.draw(ColorF{ 0.10, 0.13, 0.18, 0.96 });
	infoRect.drawFrame(1, ColorF{ 0.34, 0.50, 0.76, 0.98 });
	data.uiFont(getSelectedElement().name).draw(infoRect.x + 12, infoRect.y + 10, Palette::White);

	if (const RectF* rect = getSelectedRect())
	{
		data.smallFont(s3d::Format(U"x: ", rect->x)).draw(infoRect.x + 12, infoRect.y + 52, Palette::White);
		data.smallFont(s3d::Format(U"y: ", rect->y)).draw(infoRect.x + 12, infoRect.y + 78, Palette::White);
		data.smallFont(s3d::Format(U"w: ", rect->w)).draw(infoRect.x + 12, infoRect.y + 104, Palette::White);
		data.smallFont(s3d::Format(U"h: ", rect->h)).draw(infoRect.x + 12, infoRect.y + 130, Palette::White);
	}
	else if (const Vec2* point = getSelectedPoint())
	{
		data.smallFont(s3d::Format(U"x: ", point->x)).draw(infoRect.x + 12, infoRect.y + 52, Palette::White);
		data.smallFont(s3d::Format(U"y: ", point->y)).draw(infoRect.x + 12, infoRect.y + 78, Palette::White);
	}

	data.smallFont(U"Wheel over Elements list to scroll hidden items").draw(infoRect.x + 12, infoRect.y + 170, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Preview uses an 8px snap grid with faint guides").draw(infoRect.x + 12, infoRect.y + 196, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Arrow: move 8px / Shift+Arrow: move 32px").draw(infoRect.x + 12, infoRect.y + 222, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Ctrl+Arrow: resize rect / Shift adds 32px").draw(infoRect.x + 12, infoRect.y + 248, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Ctrl+S: save / Ctrl+R: reload / R: reset selected").draw(infoRect.x + 12, infoRect.y + 274, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Shift+R resets all edited elements").draw(infoRect.x + 12, infoRect.y + 300, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Esc returns to title").draw(infoRect.x + 12, infoRect.y + 326, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(m_statusMessage).draw(infoRect.x + 12, infoRect.bottomY() - 28, ColorF{ 1.0, 0.94, 0.70 });
}

void TitleUiEditorScene::drawSelectionHighlight() const
{
	if (const RectF* rect = getSelectedRect())
	{
		rect->drawFrame(4, 0, ColorF{ 1.0, 0.92, 0.22, 0.95 });
		const Vec2 topLeft{ rect->x, rect->y };
		const Vec2 topRight{ rect->x + rect->w, rect->y };
		const Vec2 bottomLeft{ rect->x, rect->y + rect->h };
		const Vec2 bottomRight{ rect->x + rect->w, rect->y + rect->h };
		Line{ topLeft, bottomRight }.draw(1.5, ColorF{ 1.0, 0.92, 0.22, 0.40 });
		Line{ topRight, bottomLeft }.draw(1.5, ColorF{ 1.0, 0.92, 0.22, 0.40 });
		return;
	}

	if (const Vec2* point = getSelectedPoint())
	{
		Circle{ *point, 8 }.draw(ColorF{ 1.0, 0.92, 0.22, 0.95 });
		Line{ point->movedBy(-12, 0), point->movedBy(12, 0) }.draw(2, Palette::Black);
		Line{ point->movedBy(0, -12), point->movedBy(0, 12) }.draw(2, Palette::Black);
	}
}

void TitleUiEditorScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}
