#include "BonusRoomEditorScene.h"

void BonusRoomEditorScene::draw() const
{
	Scene::Rect().draw(ColorF{ 0.05, 0.07, 0.10 });

	const auto& data = getData();
	const RectF leftPanel = getLeftPanelRect();
	const RectF rightPanel = getRightPanelRect();
	leftPanel.draw(ColorF{ 0.05, 0.07, 0.09, 0.96 });
	leftPanel.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });
	rightPanel.draw(ColorF{ 0.08, 0.10, 0.14, 0.96 });
	rightPanel.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });

	data.uiFont(Localization::GetText(U"bonus_room_editor.title", U"Bonus Room Editor", U"Bonus Room Editor")).draw(leftPanel.x + 16, leftPanel.y + 14, Palette::White);
	data.smallFont(Localization::GetText(U"bonus_room_editor.subtitle", U"Selection / Gallery / Viewer を切り替えて確認できます", U"Preview Selection / Gallery / Viewer states"))
		.draw(leftPanel.x + 16, leftPanel.y + 46, ColorF{ 0.82, 0.88, 0.96 });

	const BonusRoomDefinition* room = selectedRoom();
	const bool viewed = room && isRoomViewed(room->id);
	DrawMenuButton(getTopButtonRect(0), viewed
		? Localization::GetText(U"bonus_room_editor.unmark_viewed_button", U"閲覧済み解除", U"Unmark Viewed")
		: Localization::GetText(U"bonus_room_editor.mark_viewed_button", U"閲覧済みにする", U"Mark Viewed"), data.smallFont, viewed);
	DrawMenuButton(getTopButtonRect(1), Localization::GetText(U"bonus_room_editor.mark_all_viewed_button", U"すべて閲覧済み", U"Mark All Viewed"), data.smallFont);
	DrawMenuButton(getTopButtonRect(2), Localization::GetText(U"bonus_room_editor.reset_viewed_button", U"閲覧状態を初期化", U"Reset Viewed"), data.smallFont);
	DrawMenuButton(getTopButtonRect(3), Localization::GetText(U"bonus_room_editor.back_button", U"タイトルへ戻る", U"Back to Title"), data.smallFont);

	data.smallFont(Localization::FormatText(U"bonus_room_editor.viewed_count", U"閲覧済み {0} / {1}", U"Viewed {0} / {1}", m_previewProgress.viewedRoomIds.size(), data.bonusRooms.size()))
		.draw(leftPanel.x + 16, leftPanel.y + 206, ColorF{ 0.88, 0.93, 1.0 });
	data.smallFont(Localization::GetText(U"bonus_room_editor.rooms_title", U"部屋一覧", U"Rooms")).draw(leftPanel.x + 16, leftPanel.y + 232, Palette::White);

	for (int32 i = 0; i < static_cast<int32>(data.bonusRooms.size()); ++i)
	{
		const auto& candidate = data.bonusRooms[i];
		const bool rowViewed = isRoomViewed(candidate.id);
		const bool selected = (m_previewProgress.activeRoomId == candidate.id);
		const String rowLabel = (rowViewed ? U"● " : U"○ ") + candidate.title.get();
		DrawMenuButton(getRoomRowRect(i), rowLabel, data.smallFont, selected);
	}

	if (!room)
	{
		data.uiFont(Localization::GetText(U"bonus_room_editor.no_rooms", U"Bonus Room がありません", U"No bonus rooms available")).drawAt(rightPanel.center(), Palette::White);
		DrawSceneTransitionOverlay(data);
		return;
	}

	DrawMenuButton(getModeButtonRect(0), Localization::GetText(U"bonus_room_editor.mode_selection", U"Selection", U"Selection"), data.smallFont, m_previewMode == PreviewMode::Selection);
	DrawMenuButton(getModeButtonRect(1), Localization::GetText(U"bonus_room_editor.mode_gallery", U"Gallery", U"Gallery"), data.smallFont, m_previewMode == PreviewMode::Gallery);
	DrawMenuButton(getModeButtonRect(2), Localization::GetText(U"bonus_room_editor.mode_viewer", U"Viewer", U"Viewer"), data.smallFont, m_previewMode == PreviewMode::Viewer);

	switch (m_previewMode)
	{
	case PreviewMode::Selection:
		drawSelectionPreview(data);
		break;
	case PreviewMode::Gallery:
		drawGalleryPreview(data);
		break;
	case PreviewMode::Viewer:
	default:
		drawViewerPreview(data);
		break;
	}

	DrawSceneTransitionOverlay(data);
}

void BonusRoomEditorScene::drawSelectionPreview(const GameData& data) const
{
	const RectF contentRect = getPreviewContentRect();
	const MenuButtonStyle buttonStyle = getPreviewButtonStyle();
	data.titleFont(Localization::GetText(U"bonus_room.selection.title", U"Bonus Room", U"Bonus Room"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 18 }, Palette::White);
	data.uiFont(Localization::GetText(U"bonus_room.selection.subtitle", U"ラン制覇後に 1 つ選択", U"Choose 1 room after clearing the run"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(Localization::GetText(U"bonus_room.selection.hint", U"閲覧した部屋は次回以降のクリア報酬候補から外れます", U"Viewed rooms are removed from future clear rewards"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 92 }, Palette::Yellow);
	DrawMenuButton(getPreviewBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title", U"タイトルへ戻る", U"Back to Title"), data.smallFont, false, buttonStyle);

	const auto rooms = collectSelectionPreviewRooms();
	if (rooms.isEmpty())
	{
		data.uiFont(Localization::GetText(U"bonus_room_editor.selection_empty", U"未閲覧の候補がありません", U"No unviewed selection candidates"))
			.drawAt(contentRect.center().movedBy(0, 80), ColorF{ 0.92, 0.94, 1.0 });
		data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_selection", U"左の一覧や閲覧状態を変えて候補を確認できます", U"Change viewed state or room selection to preview candidates"))
			.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
		return;
	}

	for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
	{
		const auto* room = rooms[index];
		const RectF cardRect = getSelectionCardRect(index);
		MenuButtonStyle cardStyle = buttonStyle;
		cardStyle.cornerRadius = 18.0;
		cardStyle.hoverExpand = 3.0;
		cardStyle.pressOffsetY = 3.0;
		cardStyle.pressInsetY = 5.0;
		cardStyle.baseBorderThickness = 3.0;
		cardStyle.hoverBorderThickness = 5.0;
		cardStyle.drawAccent = false;
		const bool selected = (selectedRoom() && (selectedRoom()->id == room->id));
		const auto visual = GetMenuButtonVisualState(cardRect, selected, cardStyle);
		const RectF drawRect = visual.drawRect;
		RoundRect{ drawRect, 18 }.draw(visual.fillColor);
		RoundRect{ drawRect, 18 }.drawFrame(visual.frameThickness, 0, visual.frameColor);
		RectF{ drawRect.x, drawRect.y, drawRect.w, 14 }.draw(ColorF{ 0.82, 0.72, 0.38 });
		data.uiFont(room->title).draw(drawRect.x + 18, drawRect.y + 28, Palette::White);
		data.smallFont(room->teaser).draw(drawRect.x + 18, drawRect.y + 84, ColorF{ 0.90, 0.93, 0.98 });
		data.smallFont(Localization::FormatText(U"bonus_room.selection.press_slot", U"{0}で選択", U"Press {0}", index + 1))
			.draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
	}

	data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_selection", U"カードクリック / 1-3 で Viewer プレビューへ", U"Click a card or press 1-3 to open the Viewer preview"))
		.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
}

void BonusRoomEditorScene::drawGalleryPreview(const GameData& data) const
{
	const RectF contentRect = getPreviewContentRect();
	const MenuButtonStyle buttonStyle = getPreviewButtonStyle();
	const auto rooms = collectViewedPreviewRooms();
	data.titleFont(Localization::GetText(U"bonus_room.gallery.title", U"Bonus Room Gallery", U"Bonus Room Gallery"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 18 }, Palette::White);
	data.uiFont(Localization::GetText(U"bonus_room.gallery.subtitle", U"タイトルメニューから閲覧済みの部屋を再訪できます", U"Revisit viewed rooms from the title menu"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(Localization::FormatText(U"bonus_room.gallery.viewed_count", U"閲覧済み {0} / {1} 部屋", U"Viewed {0} / {1} rooms", rooms.size(), getData().bonusRooms.size()))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 92 }, Palette::Yellow);
	DrawMenuButton(getPreviewBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title", U"タイトルへ戻る", U"Back to Title"), data.smallFont, false, buttonStyle);

	if (rooms.isEmpty())
	{
		data.uiFont(Localization::GetText(U"bonus_room_editor.gallery_empty", U"閲覧済みの部屋がありません", U"No viewed rooms available"))
			.drawAt(contentRect.center().movedBy(0, 80), ColorF{ 0.92, 0.94, 1.0 });
		data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_gallery", U"左の一覧から閲覧済みにすると Gallery を確認できます", U"Mark a room as viewed to preview the Gallery"))
			.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
		return;
	}

	for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
	{
		const RectF rowRect = getGalleryRowRect(index);
		const bool selected = (selectedRoom() && (selectedRoom()->id == rooms[index]->id));
		const auto visual = GetMenuButtonVisualState(rowRect, selected, buttonStyle);
		RoundRect{ visual.drawRect, buttonStyle.cornerRadius }.draw(visual.fillColor);
		RoundRect{ visual.drawRect, buttonStyle.cornerRadius }.drawFrame(visual.frameThickness, 0, visual.frameColor);
		if (visual.hovered || selected)
		{
			RectF{ visual.drawRect.x + 18, visual.drawRect.bottomY() - 12, visual.drawRect.w - 36, 4 }.draw(buttonStyle.accentColor);
		}
		data.uiFont(rooms[index]->title).draw(visual.drawRect.x + 20, visual.drawRect.y + 14, Palette::White);
		data.smallFont(rooms[index]->teaser).draw(visual.drawRect.x + 20, visual.drawRect.y + 42, ColorF{ 0.88, 0.91, 0.96 });
	}

	data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_gallery", U"行をクリックで Viewer プレビューへ", U"Click a row to open the Viewer preview"))
		.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
}

void BonusRoomEditorScene::drawViewerPreview(const GameData& data) const
{
	const auto* room = selectedRoom();
	if (!room)
	{
		return;
	}

	const RectF contentRect = getPreviewContentRect();
	const RectF pageRect = getViewerPageRect();
	const MenuButtonStyle buttonStyle = getPreviewButtonStyle();
	data.titleFont(room->title).drawAt(Vec2{ contentRect.center().x, contentRect.y + 18 }, Palette::White);
	data.smallFont(Localization::FormatText(U"bonus_room.viewer.page", U"ページ {0} / {1}", U"Page {0} / {1}", m_previewProgress.activePageIndex + 1, room->pages.size()))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, Palette::Yellow);
	pageRect.draw(ColorF{ 0.12, 0.14, 0.20, 0.98 });
	pageRect.drawFrame(3, ColorF{ 0.78, 0.68, 0.34 });
	data.uiFont(room->pages[m_previewProgress.activePageIndex]).draw(pageRect.x + 36, pageRect.y + 36, ColorF{ 0.94, 0.96, 0.99 });

	if (m_previewProgress.activePageIndex > 0)
	{
		DrawMenuButton(getViewerPrevButtonRect(), Localization::GetText(U"bonus_room.viewer.prev", U"前へ", U"Prev"), data.smallFont, false, buttonStyle);
	}

	DrawMenuButton(getViewerCloseButtonRect(), m_previewViewerSourceMode == BonusRoomSceneMode::Selection
		? Localization::GetText(U"bonus_room.viewer.skip", U"スキップ", U"Skip")
		: Localization::GetText(U"bonus_room.viewer.back", U"戻る", U"Back"), data.smallFont, false, buttonStyle);

	const bool hasNextPage = ((m_previewProgress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
	DrawMenuButton(getViewerNextButtonRect(), hasNextPage
		? Localization::GetText(U"bonus_room.viewer.next", U"次へ", U"Next")
		: (m_previewViewerSourceMode == BonusRoomSceneMode::Selection
			? Localization::GetText(U"bonus_room.viewer.finish", U"完了", U"Finish")
			: Localization::GetText(U"bonus_room.viewer.close", U"閉じる", U"Close")), data.smallFont, false, buttonStyle);

	data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_viewer", U"← → / Enter でページ確認 / Esc で元のプレビューへ戻る", U"Use Left/Right or Enter to change pages / Esc returns to the previous preview"))
		.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
}

MenuButtonStyle BonusRoomEditorScene::getPreviewButtonStyle()
{
	MenuButtonStyle style;
	style.cornerRadius = 12.0;
	style.fillColor = ColorF{ 0.11, 0.13, 0.19, 0.97 };
	style.hoverFillColor = ColorF{ 0.16, 0.18, 0.24, 0.98 };
	style.pressedFillColor = ColorF{ 0.10, 0.12, 0.18, 0.98 };
	style.selectedFillColor = ColorF{ 0.28, 0.26, 0.16, 0.98 };
	style.selectedHoverFillColor = ColorF{ 0.36, 0.32, 0.18, 0.98 };
	style.frameColor = ColorF{ 0.62, 0.54, 0.30, 0.90 };
	style.hoverFrameColor = ColorF{ 0.92, 0.82, 0.44, 0.98 };
	style.selectedFrameColor = ColorF{ 1.0, 0.90, 0.56, 0.98 };
	style.accentColor = ColorF{ 0.82, 0.72, 0.38, 0.95 };
	style.selectedAccentColor = ColorF{ 0.98, 0.90, 0.60, 0.98 };
	return style;
}
