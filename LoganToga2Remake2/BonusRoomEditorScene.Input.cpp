#include "BonusRoomEditorScene.h"

void BonusRoomEditorScene::handleModeInput()
{
	if (IsMenuButtonClicked(getModeButtonRect(0)))
	{
		m_previewMode = PreviewMode::Selection;
		m_previewViewerSourceMode = BonusRoomSceneMode::Selection;
		return;
	}

	if (IsMenuButtonClicked(getModeButtonRect(1)))
	{
		m_previewMode = PreviewMode::Gallery;
		m_previewViewerSourceMode = BonusRoomSceneMode::Gallery;
		return;
	}

	if (IsMenuButtonClicked(getModeButtonRect(2)))
	{
		m_previewMode = PreviewMode::Viewer;
	}
}

void BonusRoomEditorScene::handleTopButtonInput()
{
	if (IsMenuButtonClicked(getTopButtonRect(0)))
	{
		toggleSelectedRoomViewed();
		return;
	}

	if (IsMenuButtonClicked(getTopButtonRect(1)))
	{
		markAllRoomsViewed();
		return;
	}

	if (IsMenuButtonClicked(getTopButtonRect(2)))
	{
		resetViewedRooms();
		return;
	}

	if (IsMenuButtonClicked(getTopButtonRect(3)))
	{
		requestReturnToTitle();
	}
}

void BonusRoomEditorScene::handleRoomSelectionInput()
{
	const auto& rooms = getData().bonusRooms;
	for (int32 i = 0; i < static_cast<int32>(rooms.size()); ++i)
	{
		if (IsMenuButtonClicked(getRoomRowRect(i)))
		{
			setSelectedRoom(rooms[i].id);
			return;
		}
	}
}

void BonusRoomEditorScene::handlePreviewInput()
{
	switch (m_previewMode)
	{
	case PreviewMode::Selection:
	{
		const auto rooms = collectSelectionPreviewRooms();
		for (int32 i = 0; i < static_cast<int32>(rooms.size()); ++i)
		{
			if (IsMenuButtonClicked(getSelectionCardRect(i)))
			{
				setSelectedRoom(rooms[i]->id);
				m_previewViewerSourceMode = BonusRoomSceneMode::Selection;
				m_previewMode = PreviewMode::Viewer;
				return;
			}
		}

		if (Key1.down() && (rooms.size() >= 1))
		{
			setSelectedRoom(rooms[0]->id);
			m_previewViewerSourceMode = BonusRoomSceneMode::Selection;
			m_previewMode = PreviewMode::Viewer;
			return;
		}
		if (Key2.down() && (rooms.size() >= 2))
		{
			setSelectedRoom(rooms[1]->id);
			m_previewViewerSourceMode = BonusRoomSceneMode::Selection;
			m_previewMode = PreviewMode::Viewer;
			return;
		}
		if (Key3.down() && (rooms.size() >= 3))
		{
			setSelectedRoom(rooms[2]->id);
			m_previewViewerSourceMode = BonusRoomSceneMode::Selection;
			m_previewMode = PreviewMode::Viewer;
			return;
		}

		if (KeyEscape.down())
		{
			requestReturnToTitle();
		}
		break;
	}
	case PreviewMode::Gallery:
	{
		const auto rooms = collectViewedPreviewRooms();
		for (int32 i = 0; i < static_cast<int32>(rooms.size()); ++i)
		{
			if (IsMenuButtonClicked(getGalleryRowRect(i)))
			{
				setSelectedRoom(rooms[i]->id);
				m_previewViewerSourceMode = BonusRoomSceneMode::Gallery;
				m_previewMode = PreviewMode::Viewer;
				return;
			}
		}

		if (KeyEscape.down())
		{
			requestReturnToTitle();
		}
		break;
	}
	case PreviewMode::Viewer:
	default:
	{
		const auto* room = selectedRoom();
		if (!room)
		{
			return;
		}

		if ((m_previewProgress.activePageIndex > 0) && (IsMenuButtonClicked(getViewerPrevButtonRect()) || KeyLeft.down()))
		{
			--m_previewProgress.activePageIndex;
			return;
		}

		const bool hasNextPage = ((m_previewProgress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
		if ((IsMenuButtonClicked(getViewerNextButtonRect()) || KeyRight.down() || KeyEnter.down()) && hasNextPage)
		{
			++m_previewProgress.activePageIndex;
			return;
		}

		if (IsMenuButtonClicked(getViewerCloseButtonRect()) || KeyEscape.down())
		{
			m_previewMode = previewModeFromViewerSource();
		}
		break;
	}
	}
}

RectF BonusRoomEditorScene::getLeftPanelRect()
{
	return RectF{ 12, 12, 340, Scene::Height() - 24 };
}

RectF BonusRoomEditorScene::getRightPanelRect()
{
	const RectF leftPanel = getLeftPanelRect();
	return RectF{ leftPanel.rightX() + 12, 12, Scene::Width() - leftPanel.rightX() - 24, Scene::Height() - 24 };
}

RectF BonusRoomEditorScene::getTopButtonRect(const int32 index)
{
	const RectF panel = getLeftPanelRect();
	return RectF{ panel.x + 16, panel.y + 78 + (index * 30), panel.w - 32, 26 };
}

RectF BonusRoomEditorScene::getRoomRowRect(const int32 index)
{
	const RectF panel = getLeftPanelRect();
	return RectF{ panel.x + 16, panel.y + 258 + (index * 30), panel.w - 32, 26 };
}

RectF BonusRoomEditorScene::getModeButtonRect(const int32 index)
{
	const RectF panel = getRightPanelRect();
	const double spacing = 8.0;
	const double innerWidth = (panel.w - 48.0);
	const double buttonWidth = ((innerWidth - (spacing * 2.0)) / 3.0);
	return RectF{ panel.x + 24 + ((buttonWidth + spacing) * index), panel.y + 16, buttonWidth, 28 };
}

RectF BonusRoomEditorScene::getPreviewContentRect()
{
	const RectF panel = getRightPanelRect();
	return RectF{ panel.x + 16, panel.y + 58, panel.w - 32, panel.h - 74 };
}

RectF BonusRoomEditorScene::getPreviewBackButtonRect()
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.x + 16, content.y + 6, 180, 36 };
}

RectF BonusRoomEditorScene::getSelectionCardRect(const int32 index)
{
	const RectF content = getPreviewContentRect();
	const double cardWidth = 300.0;
	const double cardHeight = 260.0;
	const double gap = 28.0;
	const double totalWidth = (cardWidth * 3.0) + (gap * 2.0);
	const double startX = content.center().x - (totalWidth * 0.5);
	return RectF{ startX + (index * (cardWidth + gap)), content.y + 124, cardWidth, cardHeight };
}

RectF BonusRoomEditorScene::getGalleryRowRect(const int32 index)
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.x + 120, content.y + 126 + (index * 84), content.w - 240, 68 };
}

RectF BonusRoomEditorScene::getViewerPageRect()
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.x + 48, content.y + 92, content.w - 96, content.h - 180 };
}

RectF BonusRoomEditorScene::getViewerPrevButtonRect()
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.x + 240, content.bottomY() - 44, 160, 40 };
}

RectF BonusRoomEditorScene::getViewerCloseButtonRect()
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.center().x - 80, content.bottomY() - 44, 160, 40 };
}

RectF BonusRoomEditorScene::getViewerNextButtonRect()
{
	const RectF content = getPreviewContentRect();
	return RectF{ content.rightX() - 400, content.bottomY() - 44, 160, 40 };
}
