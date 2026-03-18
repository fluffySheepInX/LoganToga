#pragma once

#include "Localization.h"
#include "Remake2Common.h"

struct BonusRoomDefinition
{
	String id;
   LocalizedText title;
	LocalizedText teaser;
	Array<LocalizedText> pages;
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
		const String keyPrefix = table[U"key_prefix"].getOr<String>(U"bonus_room.rooms." + room.id);
        const String titleKey = table[U"title_key"].getOr<String>(keyPrefix + U".title");
		const String teaserKey = table[U"teaser_key"].getOr<String>(keyPrefix + U".teaser");
		const String titleFallback = table[U"title"].getOr<String>(U"");
		const String teaserFallback = table[U"teaser"].getOr<String>(U"");
		room.title = titleFallback.isEmpty() ? LocalizedText{ titleKey } : LocalizedText{ titleKey, titleFallback, titleFallback };
		room.teaser = teaserFallback.isEmpty() ? LocalizedText{ teaserKey } : LocalizedText{ teaserKey, teaserFallback, teaserFallback };

		if (table[U"page_keys"].isArray())
		{
			for (const auto& pageKey : table[U"page_keys"].arrayView())
			{
				const String pageKeyStr = pageKey.get<String>();
              room.pages << LocalizedText{ pageKeyStr };
			}
		}
		else if (table[U"pages"].isArray())
		{
         int32 pageIndex = 0;
			for (const auto& page : table[U"pages"].arrayView())
			{
             const String pageKey = keyPrefix + U".page" + Format(pageIndex + 1);
				const String pageFallback = page.get<String>();
              room.pages << (pageFallback.isEmpty() ? LocalizedText{ pageKey } : LocalizedText{ pageKey, pageFallback, pageFallback });
				++pageIndex;
			}
		}
		else
		{
			const int32 pageCount = table[U"page_count"].get<int32>();
			for (int32 pageIndex = 0; pageIndex < pageCount; ++pageIndex)
			{
				const String pageKey = keyPrefix + U".page" + Format(pageIndex + 1);
               room.pages << LocalizedText{ pageKey };
			}
		}
		if (!room.pages.isEmpty())
		{
			rooms << room;
		}
	}

	return rooms;
}
