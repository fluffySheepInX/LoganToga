void BonusRoomScene::updateViewer()
{
	auto& progress = getData().bonusRoomProgress;
	const auto* room = FindBonusRoomDefinition(getData().bonusRooms, progress.activeRoomId);
	if (!room)
	{
		closeRoom();
		return;
	}

	if ((progress.activePageIndex > 0) && IsMenuButtonClicked(GetViewerPrevButtonRect()))
	{
		--progress.activePageIndex;
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			SaveContinueRun(getData(), ContinueResumeScene::BonusRoom);
		}
		return;
	}

	const bool hasNextPage = ((progress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
	if (IsMenuButtonClicked(GetViewerNextButtonRect()) || KeyEnter.down())
	{
		if (hasNextPage)
		{
			++progress.activePageIndex;
			if (progress.sceneMode == BonusRoomSceneMode::Selection)
			{
				SaveContinueRun(getData(), ContinueResumeScene::BonusRoom);
			}
		}
		else
		{
			closeRoom();
		}
		return;
	}

	if (IsMenuButtonClicked(GetViewerCloseButtonRect()) || KeyEscape.down())
	{
		closeRoom();
	}
}

void BonusRoomScene::drawViewer() const
{
	const auto& data = getData();
	const auto& progress = data.bonusRoomProgress;
	const auto* room = FindBonusRoomDefinition(data.bonusRooms, progress.activeRoomId);
	if (!room)
	{
		return;
	}

	const RectF pageRect{ 140, 120, 1000, 470 };
	pageRect.draw(ColorF{ 0.12, 0.14, 0.20, 0.98 });
	pageRect.drawFrame(3, ColorF{ 0.78, 0.68, 0.34 });
	data.titleFont(room->title).drawAt(Scene::CenterF().movedBy(0, -240), Palette::White);
 data.smallFont(Localization::FormatText(U"bonus_room.viewer.page", U"ページ {0} / {1}", U"Page {0} / {1}", progress.activePageIndex + 1, room->pages.size())).drawAt(Scene::CenterF().movedBy(0, -192), Palette::Yellow);
	data.uiFont(room->pages[progress.activePageIndex]).draw(pageRect.x + 36, pageRect.y + 36, ColorF{ 0.94, 0.96, 0.99 });

	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
	if (progress.activePageIndex > 0)
	{
     DrawMenuButton(GetViewerPrevButtonRect(), Localization::GetText(U"bonus_room.viewer.prev", U"前へ", U"Prev"), data.smallFont, false, buttonStyle);
	}
    DrawMenuButton(GetViewerCloseButtonRect(), progress.sceneMode == BonusRoomSceneMode::Selection
		? Localization::GetText(U"bonus_room.viewer.skip", U"スキップ", U"Skip")
		: Localization::GetText(U"bonus_room.viewer.back", U"戻る", U"Back"), data.smallFont, false, buttonStyle);
	const bool hasNextPage = ((progress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
    DrawMenuButton(GetViewerNextButtonRect(), hasNextPage
		? Localization::GetText(U"bonus_room.viewer.next", U"次へ", U"Next")
		: (progress.sceneMode == BonusRoomSceneMode::Selection
			? Localization::GetText(U"bonus_room.viewer.finish", U"完了", U"Finish")
			: Localization::GetText(U"bonus_room.viewer.close", U"閉じる", U"Close")), data.smallFont, false, buttonStyle);
}

void BonusRoomScene::openRoom(const String& roomId)
{
	auto& progress = getData().bonusRoomProgress;
	progress.activeRoomId = roomId;
	progress.activePageIndex = 0;
	MarkBonusRoomViewed(progress, roomId);
	if (progress.sceneMode == BonusRoomSceneMode::Selection)
	{
		SaveContinueRun(getData(), ContinueResumeScene::BonusRoom);
	}
}

void BonusRoomScene::closeRoom()
{
	auto& progress = getData().bonusRoomProgress;
	progress.activeRoomId.clear();
	progress.activePageIndex = 0;
	if (progress.sceneMode == BonusRoomSceneMode::Selection)
	{
		progress.pendingRoomIds.clear();
		ClearContinueRunSave();
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}
	else
	{
		ClearContinueRunSave();
	}
}
