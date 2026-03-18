#include "BonusRoomEditorScene.h"

#include "AudioManager.h"

BonusRoomEditorScene::BonusRoomEditorScene(const SceneBase::InitData& init)
	: SceneBase{ init }
{
	PlayMenuBgm();

	if (!getData().bonusRooms.isEmpty())
	{
		setSelectedRoom(getData().bonusRooms.front().id);
	}
}

void BonusRoomEditorScene::update()
{
	auto& data = getData();
	if (UpdateSceneTransition(data, [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	handleModeInput();
	handleTopButtonInput();
	handleRoomSelectionInput();
	handlePreviewInput();
}

void BonusRoomEditorScene::requestReturnToTitle()
{
	RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
	{
		changeScene(sceneName);
	});
}

void BonusRoomEditorScene::setSelectedRoom(const String& roomId)
{
	m_previewProgress.activeRoomId = roomId;
	m_previewProgress.activePageIndex = 0;
}

void BonusRoomEditorScene::toggleSelectedRoomViewed()
{
	const auto* room = selectedRoom();
	if (!room)
	{
		return;
	}

	for (auto it = m_previewProgress.viewedRoomIds.begin(); it != m_previewProgress.viewedRoomIds.end(); ++it)
	{
		if (*it == room->id)
		{
			m_previewProgress.viewedRoomIds.erase(it);
			return;
		}
	}

	m_previewProgress.viewedRoomIds << room->id;
}

void BonusRoomEditorScene::markAllRoomsViewed()
{
	m_previewProgress.viewedRoomIds.clear();
	for (const auto& room : getData().bonusRooms)
	{
		m_previewProgress.viewedRoomIds << room.id;
	}
}

void BonusRoomEditorScene::resetViewedRooms()
{
	m_previewProgress.viewedRoomIds.clear();
}

const BonusRoomDefinition* BonusRoomEditorScene::selectedRoom() const
{
	if (m_previewProgress.activeRoomId.isEmpty())
	{
		return getData().bonusRooms.isEmpty() ? nullptr : &getData().bonusRooms.front();
	}

	return FindBonusRoomDefinition(getData().bonusRooms, m_previewProgress.activeRoomId);
}

bool BonusRoomEditorScene::isRoomViewed(const String& roomId) const
{
	for (const auto& viewedRoomId : m_previewProgress.viewedRoomIds)
	{
		if (viewedRoomId == roomId)
		{
			return true;
		}
	}

	return false;
}

Array<const BonusRoomDefinition*> BonusRoomEditorScene::collectSelectionPreviewRooms() const
{
	Array<const BonusRoomDefinition*> rooms;
	const auto* selected = selectedRoom();
	if (selected && !isRoomViewed(selected->id))
	{
		rooms << selected;
	}

	for (const auto& room : getData().bonusRooms)
	{
		if (isRoomViewed(room.id))
		{
			continue;
		}
		if (selected && (room.id == selected->id))
		{
			continue;
		}

		rooms << &room;
		if (rooms.size() >= 3)
		{
			break;
		}
	}

	return rooms;
}

Array<const BonusRoomDefinition*> BonusRoomEditorScene::collectViewedPreviewRooms() const
{
	Array<const BonusRoomDefinition*> rooms;
	for (const auto& room : getData().bonusRooms)
	{
		if (isRoomViewed(room.id))
		{
			rooms << &room;
		}
	}
	return rooms;
}

BonusRoomEditorScene::PreviewMode BonusRoomEditorScene::previewModeFromViewerSource() const
{
	return (m_previewViewerSourceMode == BonusRoomSceneMode::Gallery)
		? PreviewMode::Gallery
		: PreviewMode::Selection;
}
