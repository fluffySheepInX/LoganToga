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

    data.uiFont(Localization::GetText(U"bonus_room_editor.title")).draw(leftPanel.x + 16, leftPanel.y + 14, Palette::White);
	data.smallFont(Localization::GetText(U"bonus_room_editor.subtitle"))
		.draw(leftPanel.x + 16, leftPanel.y + 46, ColorF{ 0.82, 0.88, 0.96 });

	const BonusRoomDefinition* room = selectedRoom();
	const bool viewed = room && isRoomViewed(room->id);
	DrawMenuButton(getTopButtonRect(0), viewed
     ? Localization::GetText(U"bonus_room_editor.unmark_viewed_button")
		: Localization::GetText(U"bonus_room_editor.mark_viewed_button"), data.smallFont, viewed);
	DrawMenuButton(getTopButtonRect(1), Localization::GetText(U"bonus_room_editor.mark_all_viewed_button"), data.smallFont);
	DrawMenuButton(getTopButtonRect(2), Localization::GetText(U"bonus_room_editor.reset_viewed_button"), data.smallFont);
	DrawMenuButton(getTopButtonRect(3), Localization::GetText(U"bonus_room_editor.back_button"), data.smallFont);

 data.smallFont(Localization::FormatText(U"bonus_room_editor.viewed_count", m_previewProgress.viewedRoomIds.size(), data.bonusRooms.size()))
		.draw(leftPanel.x + 16, leftPanel.y + 206, ColorF{ 0.88, 0.93, 1.0 });
   data.smallFont(Localization::GetText(U"bonus_room_editor.rooms_title")).draw(leftPanel.x + 16, leftPanel.y + 232, Palette::White);

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
       data.uiFont(Localization::GetText(U"bonus_room_editor.no_rooms")).drawAt(rightPanel.center(), Palette::White);
		DrawSceneTransitionOverlay(data);
		return;
	}

  DrawMenuButton(getModeButtonRect(0), Localization::GetText(U"bonus_room_editor.mode_selection"), data.smallFont, m_previewMode == PreviewMode::Selection);
	DrawMenuButton(getModeButtonRect(1), Localization::GetText(U"bonus_room_editor.mode_gallery"), data.smallFont, m_previewMode == PreviewMode::Gallery);
	DrawMenuButton(getModeButtonRect(2), Localization::GetText(U"bonus_room_editor.mode_viewer"), data.smallFont, m_previewMode == PreviewMode::Viewer);

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
  data.titleFont(Localization::GetText(U"bonus_room.selection.title"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 18 }, Palette::White);
  data.uiFont(Localization::GetText(U"bonus_room.selection.subtitle"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, ColorF{ 0.84, 0.90, 1.0 });
    data.smallFont(Localization::GetText(U"bonus_room.selection.hint"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 92 }, Palette::Yellow);
    DrawMenuButton(getPreviewBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title"), data.smallFont, false, buttonStyle);

	const auto rooms = collectSelectionPreviewRooms();
	if (rooms.isEmpty())
	{
      data.uiFont(Localization::GetText(U"bonus_room_editor.selection_empty"))
			.drawAt(contentRect.center().movedBy(0, 80), ColorF{ 0.92, 0.94, 1.0 });
      data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_selection"))
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
        data.smallFont(Localization::FormatText(U"bonus_room.selection.press_slot", index + 1))
			.draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
	}

  data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_selection"))
		.draw(contentRect.x + 24, contentRect.bottomY() - 18, ColorF{ 0.82, 0.88, 0.96 });
}

void BonusRoomEditorScene::drawGalleryPreview(const GameData& data) const
{
	const RectF contentRect = getPreviewContentRect();
	const MenuButtonStyle buttonStyle = getPreviewButtonStyle();
	const auto rooms = collectViewedPreviewRooms();
    data.titleFont(Localization::GetText(U"bonus_room.gallery.title"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 18 }, Palette::White);
    data.uiFont(Localization::GetText(U"bonus_room.gallery.subtitle"))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, ColorF{ 0.84, 0.90, 1.0 });
    data.smallFont(Localization::FormatText(U"bonus_room.gallery.viewed_count", rooms.size(), getData().bonusRooms.size()))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 92 }, Palette::Yellow);
    DrawMenuButton(getPreviewBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title"), data.smallFont, false, buttonStyle);

	if (rooms.isEmpty())
	{
      data.uiFont(Localization::GetText(U"bonus_room_editor.gallery_empty"))
			.drawAt(contentRect.center().movedBy(0, 80), ColorF{ 0.92, 0.94, 1.0 });
       data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_gallery"))
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

  data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_gallery"))
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
 data.smallFont(Localization::FormatText(U"bonus_room.viewer.page", m_previewProgress.activePageIndex + 1, room->pages.size()))
		.drawAt(Vec2{ contentRect.center().x, contentRect.y + 62 }, Palette::Yellow);
	pageRect.draw(ColorF{ 0.12, 0.14, 0.20, 0.98 });
	pageRect.drawFrame(3, ColorF{ 0.78, 0.68, 0.34 });
	data.uiFont(room->pages[m_previewProgress.activePageIndex]).draw(pageRect.x + 36, pageRect.y + 36, ColorF{ 0.94, 0.96, 0.99 });

	if (m_previewProgress.activePageIndex > 0)
	{
        DrawMenuButton(getViewerPrevButtonRect(), Localization::GetText(U"bonus_room.viewer.prev"), data.smallFont, false, buttonStyle);
	}

	DrawMenuButton(getViewerCloseButtonRect(), m_previewViewerSourceMode == BonusRoomSceneMode::Selection
        ? Localization::GetText(U"bonus_room.viewer.skip")
		: Localization::GetText(U"bonus_room.viewer.back"), data.smallFont, false, buttonStyle);

	const bool hasNextPage = ((m_previewProgress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
	DrawMenuButton(getViewerNextButtonRect(), hasNextPage
      ? Localization::GetText(U"bonus_room.viewer.next")
		: (m_previewViewerSourceMode == BonusRoomSceneMode::Selection
          ? Localization::GetText(U"bonus_room.viewer.finish")
			: Localization::GetText(U"bonus_room.viewer.close")), data.smallFont, false, buttonStyle);

 data.smallFont(Localization::GetText(U"bonus_room_editor.controls_hint_viewer"))
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
