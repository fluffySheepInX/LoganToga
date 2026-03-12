#pragma once

#include "Remake2Common.h"

struct BonusRoomDefinition
{
	String id;
	String title;
	String teaser;
	Array<String> pages;
};

enum class BonusRoomSceneMode
{
	Selection,
	Gallery
};

struct BonusRoomProgress
{
	Array<String> viewedRoomIds;
	Array<String> pendingRoomIds;
	String activeRoomId;
	int32 activePageIndex = 0;
	BonusRoomSceneMode sceneMode = BonusRoomSceneMode::Selection;
};

[[nodiscard]] inline const BonusRoomDefinition* FindBonusRoomDefinition(const Array<BonusRoomDefinition>& rooms, const String& id)
{
	for (const auto& room : rooms)
	{
		if (room.id == id)
		{
			return &room;
		}
	}

	return nullptr;
}

[[nodiscard]] inline bool HasViewedBonusRoom(const BonusRoomProgress& progress, const String& id)
{
	for (const auto& viewedId : progress.viewedRoomIds)
	{
		if (viewedId == id)
		{
			return true;
		}
	}

	return false;
}

inline void MarkBonusRoomViewed(BonusRoomProgress& progress, const String& id)
{
	if (!HasViewedBonusRoom(progress, id))
	{
		progress.viewedRoomIds << id;
	}
}

inline void ResetBonusRoomSceneState(BonusRoomProgress& progress)
{
	progress.pendingRoomIds.clear();
	progress.activeRoomId.clear();
	progress.activePageIndex = 0;
	progress.sceneMode = BonusRoomSceneMode::Selection;
}

[[nodiscard]] inline Array<String> BuildBonusRoomChoices(const BonusRoomProgress& progress, const Array<BonusRoomDefinition>& rooms, const int32 maxChoices = 3)
{
	Array<String> remainingRoomIds;
	for (const auto& room : rooms)
	{
		if (!HasViewedBonusRoom(progress, room.id))
		{
			remainingRoomIds << room.id;
		}
	}

	Array<String> choices;
	while (!remainingRoomIds.isEmpty() && (choices.size() < maxChoices))
	{
		const int32 choiceIndex = Random(static_cast<int32>(remainingRoomIds.size() - 1));
		choices << remainingRoomIds[choiceIndex];
		remainingRoomIds.remove_at(choiceIndex);
	}

	return choices;
}

[[nodiscard]] inline bool PrepareBonusRoomSelection(BonusRoomProgress& progress, const Array<BonusRoomDefinition>& rooms)
{
	ResetBonusRoomSceneState(progress);
	progress.pendingRoomIds = BuildBonusRoomChoices(progress, rooms);
	progress.sceneMode = BonusRoomSceneMode::Selection;
	return !progress.pendingRoomIds.isEmpty();
}

[[nodiscard]] inline Array<const BonusRoomDefinition*> CollectViewedBonusRooms(const Array<BonusRoomDefinition>& rooms, const BonusRoomProgress& progress)
{
	Array<const BonusRoomDefinition*> viewedRooms;
	for (const auto& room : rooms)
	{
		if (HasViewedBonusRoom(progress, room.id))
		{
			viewedRooms << &room;
		}
	}

	return viewedRooms;
}

[[nodiscard]] inline Array<BonusRoomDefinition> LoadBonusRoomDefinitions(const String& path)
{
	const TOMLReader toml{ path };
	if (!toml)
	{
		throw Error{ U"Failed to load bonus room config: " + path };
	}

	Array<BonusRoomDefinition> rooms;
	for (const auto& table : toml[U"rooms"].tableArrayView())
	{
		BonusRoomDefinition room;
		room.id = table[U"id"].get<String>();
		room.title = table[U"title"].get<String>();
		room.teaser = table[U"teaser"].get<String>();
		for (const auto& page : table[U"pages"].arrayView())
		{
			room.pages << page.get<String>();
		}
		if (!room.pages.isEmpty())
		{
			rooms << room;
		}
	}

	return rooms;
}
