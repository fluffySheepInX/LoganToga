void BonusRoomScene::updateGallery()
{
	auto& progress = getData().bonusRoomProgress;
	const Array<const BonusRoomDefinition*> rooms = viewedRooms();
	if (rooms.isEmpty())
	{
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	if (IsMenuButtonClicked(GetBackButtonRect()) || KeyEscape.down())
	{
		ResetBonusRoomSceneState(progress);
		progress.sceneMode = BonusRoomSceneMode::Gallery;
		ClearContinueRunSave();
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
	{
		if (IsMenuButtonClicked(GetGalleryRoomRect(index)))
		{
			openRoom(rooms[index]->id);
			return;
		}
	}
}

void BonusRoomScene::drawGallery() const
{
	const auto& data = getData();
	const Array<const BonusRoomDefinition*> rooms = viewedRooms();
	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
    data.titleFont(Localization::GetText(U"bonus_room.gallery.title", U"Bonus Room Gallery", U"Bonus Room Gallery")).drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
	data.uiFont(Localization::GetText(U"bonus_room.gallery.subtitle", U"タイトルメニューから閲覧済みの部屋を再訪できます", U"Revisit viewed rooms from the title menu")).drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(Localization::FormatText(U"bonus_room.gallery.viewed_count", U"閲覧済み {0} / {1} 部屋", U"Viewed {0} / {1} rooms", rooms.size(), data.bonusRooms.size())).drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);
	DrawMenuButton(GetBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title", U"タイトルへ戻る", U"Back to Title"), data.smallFont, false, buttonStyle);

	for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
	{
		MenuButtonStyle panelStyle = buttonStyle;
		panelStyle.cornerRadius = 10.0;
		panelStyle.accentMargin = 18.0;
		const auto visual = GetMenuButtonVisualState(GetGalleryRoomRect(index), false, panelStyle);
		RoundRect{ visual.drawRect, panelStyle.cornerRadius }.draw(visual.fillColor);
		RoundRect{ visual.drawRect, panelStyle.cornerRadius }.drawFrame(visual.frameThickness, 0, visual.frameColor);
		if (visual.hovered)
		{
			RectF{ visual.drawRect.x + 18, visual.drawRect.bottomY() - 12, visual.drawRect.w - 36, 4 }.draw(panelStyle.accentColor);
		}
		data.uiFont(rooms[index]->title).draw(visual.drawRect.x + 20, visual.drawRect.y + 14, Palette::White);
		data.smallFont(rooms[index]->teaser).draw(visual.drawRect.x + 20, visual.drawRect.y + 42, ColorF{ 0.88, 0.91, 0.96 });
	}
}
