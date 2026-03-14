#include "BonusRoomScene.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

namespace
{
	[[nodiscard]] MenuButtonStyle GetBonusButtonStyle()
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

	[[nodiscard]] RectF GetBackButtonRect()
	{
		return RectF{ 40, 40, 180, 36 };
	}

	[[nodiscard]] RectF GetGalleryRoomRect(const int32 index)
	{
		return RectF{ 160, 160 + (index * 84), 820, 68 };
	}

	[[nodiscard]] RectF GetViewerPrevButtonRect()
	{
		return RectF{ 320, 640, 160, 40 };
	}

	[[nodiscard]] RectF GetViewerCloseButtonRect()
	{
		return RectF{ 560, 640, 160, 40 };
	}

	[[nodiscard]] RectF GetViewerNextButtonRect()
	{
		return RectF{ 800, 640, 160, 40 };
	}
}

BonusRoomScene::BonusRoomScene(const SceneBase::InitData& init)
	: SceneBase{ init } {}

void BonusRoomScene::update()
{
	if (UpdateSceneTransition(getData(), [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	auto& data = getData();
	auto& progress = data.bonusRoomProgress;
	if (progress.activeRoomId.isEmpty())
	{
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			updateSelection();
		}
		else
		{
			updateGallery();
		}
		return;
	}

	updateViewer();
}

void BonusRoomScene::draw() const
{
	Scene::Rect().draw(ColorF{ 0.06, 0.07, 0.10 });
	const auto& data = getData();
	const auto& progress = data.bonusRoomProgress;
	if (progress.activeRoomId.isEmpty())
	{
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			drawSelection();
		}
		else
		{
			drawGallery();
		}
	}
	else
	{
		drawViewer();
	}

	DrawSceneTransitionOverlay(data);
}

RectF BonusRoomScene::getSelectionCardRect(const int32 index)
{
	const double cardWidth = 300.0;
	const double cardHeight = 260.0;
	const double gap = 28.0;
	const double totalWidth = (cardWidth * 3.0) + (gap * 2.0);
	const double startX = (Scene::Width() - totalWidth) * 0.5;
	return RectF{ startX + (index * (cardWidth + gap)), 230, cardWidth, cardHeight };
}

Array<const BonusRoomDefinition*> BonusRoomScene::viewedRooms() const
{
	const auto& data = getData();
	return CollectViewedBonusRooms(data.bonusRooms, data.bonusRoomProgress);
}

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
	const String nextLabel = hasNextPage
		? U"Next"
		: (progress.sceneMode == BonusRoomSceneMode::Selection ? U"Finish" : U"Close");
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

void BonusRoomScene::drawSelection() const
{
	const auto& data = getData();
	const auto& progress = data.bonusRoomProgress;
	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
	data.titleFont(U"Bonus Room").drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
	data.uiFont(U"Choose 1 room after clearing the run").drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(U"Viewed rooms are removed from future clear rewards").drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);
	DrawMenuButton(GetBackButtonRect(), U"Back to Title", data.smallFont, false, buttonStyle);

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
		data.smallFont(s3d::Format(U"Press ", index + 1)).draw(drawRect.x + 18, drawRect.bottomY() - 34, Palette::Yellow);
	}
}

void BonusRoomScene::drawGallery() const
{
	const auto& data = getData();
	const Array<const BonusRoomDefinition*> rooms = viewedRooms();
	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
	data.titleFont(U"Bonus Room Gallery").drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
	data.uiFont(U"Revisit viewed rooms from the title menu").drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
	data.smallFont(s3d::Format(U"Viewed ", rooms.size(), U" / ", data.bonusRooms.size(), U" rooms")).drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);
	DrawMenuButton(GetBackButtonRect(), U"Back to Title", data.smallFont, false, buttonStyle);

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
	data.smallFont(s3d::Format(U"Page ", progress.activePageIndex + 1, U" / ", room->pages.size())).drawAt(Scene::CenterF().movedBy(0, -192), Palette::Yellow);
	data.uiFont(room->pages[progress.activePageIndex]).draw(pageRect.x + 36, pageRect.y + 36, ColorF{ 0.94, 0.96, 0.99 });

	const MenuButtonStyle buttonStyle = GetBonusButtonStyle();
	if (progress.activePageIndex > 0)
	{
		DrawMenuButton(GetViewerPrevButtonRect(), U"Prev", data.smallFont, false, buttonStyle);
	}
	DrawMenuButton(GetViewerCloseButtonRect(), progress.sceneMode == BonusRoomSceneMode::Selection ? U"Skip" : U"Back", data.smallFont, false, buttonStyle);
	const bool hasNextPage = ((progress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
	DrawMenuButton(GetViewerNextButtonRect(), hasNextPage ? U"Next" : (progress.sceneMode == BonusRoomSceneMode::Selection ? U"Finish" : U"Close"), data.smallFont, false, buttonStyle);
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
