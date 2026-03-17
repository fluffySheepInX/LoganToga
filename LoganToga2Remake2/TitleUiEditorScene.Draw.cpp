#include "TitleUiEditorScene.h"

#include "TitleUiText.h"

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
	const RectF previewGridRect = getPreviewViewportRect();
	auto screenPos = [this](const Vec2& pos)
	{
		return toPreviewScreenPos(pos);
	};
	auto screenRect = [this](const RectF& rect)
	{
		return toPreviewScreenRect(rect);
	};

	const double cellSize = TitleUiEditorSceneDrawDetail::EditorGridCellSize;
	const double startWorldX = (Math::Floor((previewGridRect.x - m_previewCameraOffset.x) / cellSize) * cellSize);
	const double endWorldX = (Math::Ceil((previewGridRect.rightX() - m_previewCameraOffset.x) / cellSize) * cellSize);
	for (double worldX = startWorldX; worldX <= endWorldX; worldX += cellSize)
	{
		const int32 cellIndex = static_cast<int32>(Math::Round(worldX / cellSize));
		const bool isMajorLine = ((Abs(cellIndex) % TitleUiEditorSceneDrawDetail::EditorGridMajorLineSpan) == 0);
		const double drawX = (worldX + m_previewCameraOffset.x);
		Line{ drawX, previewGridRect.y, drawX, previewGridRect.y + previewGridRect.h }
			.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}
	const double startWorldY = (Math::Floor((previewGridRect.y - m_previewCameraOffset.y) / cellSize) * cellSize);
	const double endWorldY = (Math::Ceil((previewGridRect.bottomY() - m_previewCameraOffset.y) / cellSize) * cellSize);
	for (double worldY = startWorldY; worldY <= endWorldY; worldY += cellSize)
	{
		const int32 cellIndex = static_cast<int32>(Math::Round(worldY / cellSize));
		const bool isMajorLine = ((Abs(cellIndex) % TitleUiEditorSceneDrawDetail::EditorGridMajorLineSpan) == 0);
		const double drawY = (worldY + m_previewCameraOffset.y);
		Line{ previewGridRect.x, drawY, previewGridRect.x + previewGridRect.w, drawY }
			.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}
	screenRect(m_layout.panelRect).draw(ColorF{ 0.13, 0.16, 0.20 });
	screenRect(m_layout.panelRect).drawFrame(2, ColorF{ 0.3, 0.45, 0.7 });

	const auto& data = getData();
	const bool hasContinue = m_previewHasContinue;
	const bool hasViewedBonusRooms = m_previewHasViewedBonusRooms;

	data.titleFont(TitleUiText::Title).drawAt(screenPos(m_layout.titlePos), Palette::White);
	data.uiFont(TitleUiText::Subtitle).drawAt(screenPos(m_layout.subtitlePos), ColorF{ 0.75, 0.86, 1.0 });
	data.smallFont(TitleUiText::SummaryLines[0]).drawAt(screenPos(m_layout.summaryLine1Pos), Palette::White);
	data.smallFont(TitleUiText::SummaryLines[1]).drawAt(screenPos(m_layout.summaryLine2Pos), Palette::White);
	data.smallFont(TitleUiText::SummaryLines[2]).drawAt(screenPos(m_layout.summaryLine3Pos), Palette::White);
	data.smallFont(TitleUiText::ViewedBonusRoomsPreview).drawAt(screenPos(m_layout.viewedBonusRoomsPos), Palette::White);
	data.smallFont(TitleUiText::GetEnterHintText(hasContinue))
		.drawAt(screenPos(m_layout.enterHintPos), Palette::Yellow);

	if (hasContinue)
	{
		const RectF continuePreviewRect = screenRect(m_layout.continuePreviewRect);
		drawButton(screenRect(m_layout.continueButtonRect), TitleUiText::ContinueButton, data.uiFont, true);
		continuePreviewRect.draw(ColorF{ 0.09, 0.12, 0.18, 0.96 });
		continuePreviewRect.drawFrame(2, ColorF{ 0.42, 0.60, 0.92 });
		data.smallFont(TitleUiText::ContinuePreviewTitle).draw(continuePreviewRect.x + 14, continuePreviewRect.y + 10, ColorF{ 0.82, 0.90, 1.0 });
		data.smallFont(TitleUiText::ContinuePreviewHeadline).draw(continuePreviewRect.x + 14, continuePreviewRect.y + 32, Palette::White);
		data.smallFont(TitleUiText::ContinuePreviewDetail).draw(continuePreviewRect.x + 14, continuePreviewRect.y + 52, ColorF{ 0.86, 0.90, 0.96 });
		data.smallFont(s3d::Format(TitleUiText::ContinuePreviewCardsPrefix.get(), 3)).draw(continuePreviewRect.x + 14, continuePreviewRect.y + 72, Palette::Gold);
	}

	if (hasViewedBonusRooms)
	{
		data.smallFont(TitleUiText::BonusRoomsHint)
			.drawAt(screenPos(TitleUi::GetBonusRoomHintPos(m_layout, hasContinue)), ColorF{ 1.0, 0.88, 0.55 });
		drawButton(screenRect(TitleUi::GetBonusButtonRect(m_layout, hasContinue)), TitleUiText::BonusRoomsButton, data.uiFont);
	}

	drawButton(screenRect(TitleUi::GetTutorialButtonRect(m_layout, hasContinue)), TitleUiText::TutorialButton, data.uiFont, true);
	drawButton(screenRect(TitleUi::GetQuickGuideButtonRect(m_layout, hasContinue)), TitleUiText::QuickGuideButton, data.uiFont, true);
	drawButton(screenRect(TitleUi::GetStartButtonRect(m_layout, hasContinue)), TitleUiText::GetStartButtonText(hasContinue), data.uiFont);
	data.smallFont(TitleUiText::QuickGuideHint)
		.drawAt(screenPos(TitleUi::GetQuickGuideHintPos(m_layout, hasContinue)), ColorF{ 0.88, 0.92, 1.0 });

	const s3d::Size resolutionSize = GetWindowResolutionSize(data.displaySettings.resolutionPreset);
	data.smallFont(TitleUiText::ResolutionLabel).draw(screenPos(m_layout.resolutionLabelPos), Palette::White);
	data.smallFont(s3d::Format(TitleUiText::CurrentPrefix.get(), GetWindowResolutionLabel(data.displaySettings.resolutionPreset), U" (", resolutionSize.x, U"x", resolutionSize.y, U")"))
		.draw(screenPos(m_layout.resolutionValuePos), ColorF{ 0.85, 0.92, 1.0 });
	drawButton(screenRect(m_layout.resolutionSmallButtonRect), GetWindowResolutionLabel(WindowResolutionPreset::Small), data.smallFont, false);
	drawButton(screenRect(m_layout.resolutionMediumButtonRect), GetWindowResolutionLabel(WindowResolutionPreset::Medium), data.smallFont, data.displaySettings.resolutionPreset == WindowResolutionPreset::Medium);
	drawButton(screenRect(m_layout.resolutionLargeButtonRect), GetWindowResolutionLabel(WindowResolutionPreset::Large), data.smallFont, false);

	data.smallFont(TitleUiText::SaveLocationLabel).draw(screenPos(m_layout.saveLocationLabelPos), Palette::White);
	data.smallFont(TitleUiText::CurrentPrefix.get() + GetContinueRunSaveLocationLabel(GetContinueRunSaveLocation()))
		.draw(screenPos(m_layout.saveLocationValuePos), ColorF{ 0.85, 0.92, 1.0 });
	drawButton(screenRect(m_layout.saveLocationButtonRect), TitleUiText::SaveLocationButton, data.smallFont, true);
	data.smallFont(TitleUiText::DataManagementLabel)
		.drawAt(screenPos(m_layout.dataManagementLabelPos), ColorF{ 0.90, 0.94, 1.0 });
	drawButton(screenRect(m_layout.clearContinueRunButtonRect), TitleUiText::ClearContinueButton, data.smallFont, hasContinue);
	drawButton(screenRect(m_layout.clearSettingsButtonRect), TitleUiText::ClearSettingsButton, data.smallFont, true);
	drawButton(screenRect(m_layout.exitButtonRect), TitleUiText::ExitButton, data.smallFont);
	data.smallFont(TitleUiText::DataManagementHint)
		.drawAt(screenPos(m_layout.dataManagementHintPos), ColorF{ 0.82, 0.88, 0.96 });

#ifdef _DEBUG
	if (m_previewDebugButtons)
	{
		data.smallFont(TitleUiText::DebugUnlockHint)
			.drawAt(screenPos(hasContinue ? m_layout.debugHintPosWithContinue : m_layout.debugHintPosWithoutContinue), ColorF{ 1.0, 0.75, 0.45 });
		drawButton(screenRect(TitleUi::GetDebugButtonRect(m_layout, hasContinue)), TitleUiText::DebugFullUnlockButton, data.uiFont, true);
		drawButton(screenRect(m_layout.mapEditButtonRect), TitleUiText::MapEditButton, data.smallFont);
		drawButton(screenRect(m_layout.balanceEditButtonRect), TitleUiText::BalanceEditButton, data.smallFont);
		drawButton(screenRect(m_layout.transitionPresetButtonRect), TitleUiText::TransitionPresetPrefix.get() + U"Default", data.smallFont, true);
		drawButton(screenRect(m_layout.titleUiEditorButtonRect), TitleUiText::TitleUiEditorButton, data.smallFont, true);
	}
#endif

	if (m_previewQuickGuideOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		const RectF quickGuidePanelRect = screenRect(m_layout.quickGuidePanelRect);
		const Vec2 quickGuideBodyPos = screenPos(m_layout.quickGuideBodyPos);
		const double lineStep = 34.0;
		quickGuidePanelRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		quickGuidePanelRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.titleFont(TitleUiText::QuickGuideTitle).drawAt(quickGuidePanelRect.center().movedBy(0, -168), Palette::White);
		data.smallFont(TitleUiText::QuickGuideSubtitle)
			.drawAt(quickGuidePanelRect.center().movedBy(0, -128), ColorF{ 0.84, 0.90, 1.0 });
		for (size_t i = 0; i < TitleUiText::QuickGuideBodyLines.size(); ++i)
		{
			data.uiFont(TitleUiText::QuickGuideBodyLines[i]).draw(quickGuideBodyPos.movedBy(0, lineStep * i), Palette::White);
		}
		data.smallFont(TitleUiText::QuickGuideFlow)
			.drawAt(screenPos(m_layout.quickGuideFlowPos), ColorF{ 1.0, 0.90, 0.58 });
		drawButton(screenRect(m_layout.quickGuideTutorialButtonRect), TitleUiText::QuickGuideTutorialButton, data.uiFont, true);
		drawButton(screenRect(m_layout.quickGuideCloseButtonRect), TitleUiText::CloseButton, data.uiFont);
		data.smallFont(TitleUiText::QuickGuideEscHint).drawAt(screenPos(m_layout.quickGuideEscHintPos), ColorF{ 0.80, 0.87, 0.95 });
	}

	if (m_previewDataClearDialogOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		const RectF dataClearDialogRect = screenRect(m_layout.dataClearDialogRect);
		dataClearDialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		dataClearDialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.uiFont(TitleUiText::DataClearQuestion).drawAt(dataClearDialogRect.center().movedBy(0, -48), Palette::White);
		data.smallFont(TitleUiText::DataClearBody)
			.drawAt(dataClearDialogRect.center().movedBy(0, -8), ColorF{ 0.84, 0.90, 1.0 });
		drawButton(screenRect(m_layout.dataClearDialogYesButtonRect), TitleUiText::Yes, data.uiFont, true);
		drawButton(screenRect(m_layout.dataClearDialogNoButtonRect), TitleUiText::No, data.uiFont);
	}

	if (m_previewExitDialogOpen)
	{
		Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
		const RectF exitDialogRect = screenRect(m_layout.exitDialogRect);
		exitDialogRect.draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
		exitDialogRect.drawFrame(2, ColorF{ 0.40, 0.58, 0.90, 0.98 });
		data.uiFont(TitleUiText::ExitQuestion).drawAt(exitDialogRect.center().movedBy(0, -34), Palette::White);
		drawButton(screenRect(m_layout.exitDialogYesButtonRect), TitleUiText::Yes, data.uiFont, true);
		drawButton(screenRect(m_layout.exitDialogNoButtonRect), TitleUiText::No, data.uiFont);
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

		if (const auto defaultRect = getSelectedDefaultRect())
		{
			data.smallFont(s3d::Format(U"default w/h: ", defaultRect->w, U" x ", defaultRect->h)).draw(infoRect.x + 12, infoRect.y + 156, ColorF{ 0.82, 0.90, 1.0 });
			const bool tooSmall = isRectLikelyTooSmall(*rect, *defaultRect);
			data.smallFont(tooSmall ? U"Warning: current size is too small for this button" : U"Current size is within the normal button range")
				.draw(infoRect.x + 12, infoRect.y + 186, tooSmall ? ColorF{ 1.0, 0.68, 0.52 } : ColorF{ 0.72, 0.88, 0.78 });
			drawButton(getInfoPresetButtonRect(0), U"220x36", data.smallFont, rect->size == SizeF{ 220, 36 });
			drawButton(getInfoPresetButtonRect(1), U"170x32", data.smallFont, rect->size == SizeF{ 170, 32 });
			drawButton(getInfoPresetButtonRect(2), U"140x40", data.smallFont, rect->size == SizeF{ 140, 40 });
			drawButton(getInfoPresetButtonRect(3), U"128x30", data.smallFont, rect->size == SizeF{ 128, 30 });
			drawButton(getInfoPresetButtonRect(4), U"Default Size", data.smallFont, rect->size == defaultRect->size);
		}
	}
	else if (const Vec2* point = getSelectedPoint())
	{
		data.smallFont(s3d::Format(U"x: ", point->x)).draw(infoRect.x + 12, infoRect.y + 52, Palette::White);
		data.smallFont(s3d::Format(U"y: ", point->y)).draw(infoRect.x + 12, infoRect.y + 78, Palette::White);
		if (const auto defaultPoint = getSelectedDefaultPoint())
		{
			data.smallFont(s3d::Format(U"default x/y: ", defaultPoint->x, U" / ", defaultPoint->y)).draw(infoRect.x + 12, infoRect.y + 104, ColorF{ 0.82, 0.90, 1.0 });
		}
		data.smallFont(U"Size presets are available when a rectangle element is selected").draw(infoRect.x + 12, infoRect.y + 156, ColorF{ 0.84, 0.90, 1.0 });
	}

	data.smallFont(U"Left-drag empty preview to pan the camera").draw(infoRect.x + 12, infoRect.y + 274, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Preview uses an 8px snap grid with faint guides").draw(infoRect.x + 12, infoRect.y + 300, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Arrow: move 8px / Shift+Arrow: move 32px").draw(infoRect.x + 12, infoRect.y + 326, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Ctrl+Arrow: resize rect / Shift adds 32px").draw(infoRect.x + 12, infoRect.y + 352, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Use size presets to recover tiny buttons before saving").draw(infoRect.x + 12, infoRect.y + 378, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Ctrl+S saves / invalid tiny buttons are blocked").draw(infoRect.x + 12, infoRect.y + 404, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"R resets selected / Shift+R resets all / Esc returns").draw(infoRect.x + 12, infoRect.y + 430, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(m_statusMessage).draw(infoRect.x + 12, infoRect.bottomY() - 28, ColorF{ 1.0, 0.94, 0.70 });
}

void TitleUiEditorScene::drawSelectionHighlight() const
{
	if (const RectF* rect = getSelectedRect())
	{
		const RectF screenRect = toPreviewScreenRect(*rect);
		screenRect.drawFrame(4, 0, ColorF{ 1.0, 0.92, 0.22, 0.95 });
		const Vec2 topLeft{ screenRect.x, screenRect.y };
		const Vec2 topRight{ screenRect.x + screenRect.w, screenRect.y };
		const Vec2 bottomLeft{ screenRect.x, screenRect.y + screenRect.h };
		const Vec2 bottomRight{ screenRect.x + screenRect.w, screenRect.y + screenRect.h };
		Line{ topLeft, bottomRight }.draw(1.5, ColorF{ 1.0, 0.92, 0.22, 0.40 });
		Line{ topRight, bottomLeft }.draw(1.5, ColorF{ 1.0, 0.92, 0.22, 0.40 });
		return;
	}

	if (const Vec2* point = getSelectedPoint())
	{
		const Vec2 screenPoint = toPreviewScreenPos(*point);
		Circle{ screenPoint, 8 }.draw(ColorF{ 1.0, 0.92, 0.22, 0.95 });
		Line{ screenPoint.movedBy(-12, 0), screenPoint.movedBy(12, 0) }.draw(2, Palette::Black);
		Line{ screenPoint.movedBy(0, -12), screenPoint.movedBy(0, 12) }.draw(2, Palette::Black);
	}
}

void TitleUiEditorScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}
