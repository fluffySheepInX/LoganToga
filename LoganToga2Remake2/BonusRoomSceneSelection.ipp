void BonusRoomScene::updateSelection()
{
	auto& progress = getData().bonusRoomProgress;
	if (progress.pendingRoomIds.isEmpty())
	{
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
		return;
	}

	for (int32 index = 0; index < static_cast<int32>(progress.pendingRoomIds.size()); ++index)
	{
		const RectF cardRect = getSelectionCardRect(index);
		if (IsMenuButtonClicked(cardRect))
		{
			openRoom(progress.pendingRoomIds[index]);
			return;
		}
	}

	if (Key1.down() && (progress.pendingRoomIds.size() >= 1))
	{
		openRoom(progress.pendingRoomIds[0]);
		return;
	}
	if (Key2.down() && (progress.pendingRoomIds.size() >= 2))
	{
		openRoom(progress.pendingRoomIds[1]);
		return;
	}
	if (Key3.down() && (progress.pendingRoomIds.size() >= 3))
	{
		openRoom(progress.pendingRoomIds[2]);
		return;
	}

	if (IsMenuButtonClicked(GetBackButtonRect()) || KeyEscape.down())
	{
		ResetBonusRoomSceneState(progress);
		ClearContinueRunSave();
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}
}

void BonusRoomScene::drawSelection() const
{
	const auto& data = getData();
	const auto& progress = data.bonusRoomProgress;
	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
   data.titleFont(Localization::GetText(U"bonus_room.selection.title")).drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
	data.uiFont(Localization::GetText(U"bonus_room.selection.subtitle")).drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(Localization::GetText(U"bonus_room.selection.hint")).drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);
	DrawMenuButton(GetBackButtonRect(), Localization::GetText(U"bonus_room.common.back_to_title"), data.smallFont, false, buttonStyle);

	for (int32 index = 0; index < static_cast<int32>(progress.pendingRoomIds.size()); ++index)
	{
		const auto* room = FindBonusRoomDefinition(data.bonusRooms, progress.pendingRoomIds[index]);
		if (!room)
		{
			continue;
		}

		const RectF cardRect = getSelectionCardRect(index);
		MenuButtonStyle cardStyle = buttonStyle;
		cardStyle.cornerRadius = 18.0;
		cardStyle.hoverExpand = 3.0;
		cardStyle.pressOffsetY = 3.0;
		cardStyle.pressInsetY = 5.0;
		cardStyle.baseBorderThickness = 3.0;
		cardStyle.hoverBorderThickness = 5.0;
		cardStyle.drawAccent = false;
		const auto visual = GetMenuButtonVisualState(cardRect, false, cardStyle);
		const RectF drawRect = visual.drawRect;
		RoundRect{ drawRect, 18 }.draw(visual.fillColor);
		RoundRect{ drawRect, 18 }.drawFrame(visual.frameThickness, 0, visual.frameColor);
		RectF{ drawRect.x, drawRect.y, drawRect.w, 14 }.draw(ColorF{ 0.82, 0.72, 0.38 });
		data.uiFont(room->title).draw(drawRect.x + 18, drawRect.y + 28, Palette::White);
		data.smallFont(room->teaser).draw(drawRect.x + 18, drawRect.y + 84, ColorF{ 0.90, 0.93, 0.98 });
     data.smallFont(Localization::FormatText(U"bonus_room.selection.press_slot", index + 1)).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
	}
}
