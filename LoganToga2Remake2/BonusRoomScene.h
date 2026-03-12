#pragma once

#include "GameData.h"

class BonusRoomScene : public SceneBase
{
public:
	explicit BonusRoomScene(const SceneBase::InitData& init)
		: SceneBase{ init } {}

	void update() override
	{
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

	void draw() const override
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
			return;
		}

		drawViewer();
	}

private:
	[[nodiscard]] static RectF getSelectionCardRect(const int32 index)
	{
		const double cardWidth = 300.0;
		const double cardHeight = 260.0;
		const double gap = 28.0;
		const double totalWidth = (cardWidth * 3.0) + (gap * 2.0);
		const double startX = (Scene::Width() - totalWidth) * 0.5;
		return RectF{ startX + (index * (cardWidth + gap)), 230, cardWidth, cardHeight };
	}

	[[nodiscard]] Array<const BonusRoomDefinition*> viewedRooms() const
	{
		const auto& data = getData();
		return CollectViewedBonusRooms(data.bonusRooms, data.bonusRoomProgress);
	}

	void updateSelection()
	{
		auto& progress = getData().bonusRoomProgress;
		if (progress.pendingRoomIds.isEmpty())
		{
			changeScene(U"Title");
			return;
		}

		for (int32 index = 0; index < static_cast<int32>(progress.pendingRoomIds.size()); ++index)
		{
			const RectF cardRect = getSelectionCardRect(index);
			if (cardRect.intersects(Cursor::PosF()) && MouseL.down())
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

		if (SimpleGUI::Button(U"Back to Title", Vec2{ 40, 40 }, 180) || KeyEscape.down())
		{
			ResetBonusRoomSceneState(progress);
			changeScene(U"Title");
		}
	}

	void updateGallery()
	{
		auto& progress = getData().bonusRoomProgress;
		const Array<const BonusRoomDefinition*> rooms = viewedRooms();
		if (rooms.isEmpty())
		{
			changeScene(U"Title");
			return;
		}

		if (SimpleGUI::Button(U"Back to Title", Vec2{ 40, 40 }, 180) || KeyEscape.down())
		{
			ResetBonusRoomSceneState(progress);
			progress.sceneMode = BonusRoomSceneMode::Gallery;
			changeScene(U"Title");
			return;
		}

		for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
		{
			const Vec2 buttonPos{ 180, 170 + (index * 84) };
			if (SimpleGUI::Button(rooms[index]->title, buttonPos, 420))
			{
				openRoom(rooms[index]->id);
				return;
			}
		}
	}

	void updateViewer()
	{
		auto& progress = getData().bonusRoomProgress;
		const auto* room = FindBonusRoomDefinition(getData().bonusRooms, progress.activeRoomId);
		if (!room)
		{
			closeRoom();
			return;
		}

		if ((progress.activePageIndex > 0) && SimpleGUI::Button(U"Prev", Vec2{ 320, 640 }, 160))
		{
			--progress.activePageIndex;
			return;
		}

		const bool hasNextPage = ((progress.activePageIndex + 1) < static_cast<int32>(room->pages.size()));
		const String nextLabel = hasNextPage
			? U"Next"
			: (progress.sceneMode == BonusRoomSceneMode::Selection ? U"Finish" : U"Close");
		if (SimpleGUI::Button(nextLabel, Vec2{ 800, 640 }, 160) || KeyEnter.down())
		{
			if (hasNextPage)
			{
				++progress.activePageIndex;
			}
			else
			{
				closeRoom();
			}
			return;
		}

		if (SimpleGUI::Button(progress.sceneMode == BonusRoomSceneMode::Selection ? U"Skip" : U"Back", Vec2{ 560, 640 }, 160) || KeyEscape.down())
		{
			closeRoom();
		}
	}

	void drawSelection() const
	{
		const auto& data = getData();
		const auto& progress = data.bonusRoomProgress;
		data.titleFont(U"Bonus Room").drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
		data.uiFont(U"Choose 1 room after clearing the run").drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
		data.smallFont(U"Viewed rooms are removed from future clear rewards").drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);

		for (int32 index = 0; index < static_cast<int32>(progress.pendingRoomIds.size()); ++index)
		{
			const auto* room = FindBonusRoomDefinition(data.bonusRooms, progress.pendingRoomIds[index]);
			if (!room)
			{
				continue;
			}

			const RectF cardRect = getSelectionCardRect(index);
			const bool isHovered = cardRect.intersects(Cursor::PosF());
			RoundRect{ cardRect, 18 }.draw(ColorF{ 0.11, 0.13, 0.19, 0.97 });
			RoundRect{ cardRect, 18 }.drawFrame(isHovered ? 5 : 3, 0, ColorF{ 0.82, 0.72, 0.38 });
			RectF{ cardRect.x, cardRect.y, cardRect.w, 14 }.draw(ColorF{ 0.82, 0.72, 0.38 });
			data.uiFont(room->title).draw(cardRect.x + 18, cardRect.y + 28, Palette::White);
			data.smallFont(room->teaser).draw(cardRect.x + 18, cardRect.y + 84, ColorF{ 0.90, 0.93, 0.98 });
			data.smallFont(s3d::Format(U"Press ", index + 1)).draw(cardRect.x + 18, cardRect.bottomY() - 34, Palette::Yellow);
		}
	}

	void drawGallery() const
	{
		const auto& data = getData();
		const Array<const BonusRoomDefinition*> rooms = viewedRooms();
		data.titleFont(U"Bonus Room Gallery").drawAt(Scene::CenterF().movedBy(0, -250), Palette::White);
		data.uiFont(U"Revisit viewed rooms from the title menu").drawAt(Scene::CenterF().movedBy(0, -205), ColorF{ 0.84, 0.90, 1.0 });
		data.smallFont(s3d::Format(U"Viewed ", rooms.size(), U" / ", data.bonusRooms.size(), U" rooms")).drawAt(Scene::CenterF().movedBy(0, -172), Palette::Yellow);

		for (int32 index = 0; index < static_cast<int32>(rooms.size()); ++index)
		{
			const RectF panel{ 160, 160 + (index * 84), 820, 68 };
			panel.draw(ColorF{ 0.11, 0.13, 0.19, 0.97 });
			panel.drawFrame(2, ColorF{ 0.34, 0.42, 0.62 });
			data.uiFont(rooms[index]->title).draw(panel.x + 20, panel.y + 14, Palette::White);
			data.smallFont(rooms[index]->teaser).draw(panel.x + 20, panel.y + 42, ColorF{ 0.88, 0.91, 0.96 });
		}
	}

	void drawViewer() const
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
	}

	void openRoom(const String& roomId)
	{
		auto& progress = getData().bonusRoomProgress;
		progress.activeRoomId = roomId;
		progress.activePageIndex = 0;
		MarkBonusRoomViewed(progress, roomId);
	}

	void closeRoom()
	{
		auto& progress = getData().bonusRoomProgress;
		progress.activeRoomId.clear();
		progress.activePageIndex = 0;
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			progress.pendingRoomIds.clear();
			changeScene(U"Title");
		}
	}
};
