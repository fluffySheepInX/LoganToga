#pragma once

#include "GameData.h"

class BonusRoomScene : public SceneBase
{
public:
	explicit BonusRoomScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	[[nodiscard]] static RectF getSelectionCardRect(int32 index);
	[[nodiscard]] Array<const BonusRoomDefinition*> viewedRooms() const;
	void updateSelection();
	void updateGallery();
	void updateViewer();
	void drawSelection() const;
	void drawGallery() const;
	void drawViewer() const;
	void openRoom(const String& roomId);
	void closeRoom();
};
