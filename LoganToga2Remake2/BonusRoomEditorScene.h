#pragma once

#include "GameData.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class BonusRoomEditorScene : public SceneBase
{
public:
	explicit BonusRoomEditorScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	enum class PreviewMode
	{
		Selection,
		Gallery,
		Viewer,
	};

	BonusRoomProgress m_previewProgress;
	PreviewMode m_previewMode = PreviewMode::Selection;
	BonusRoomSceneMode m_previewViewerSourceMode = BonusRoomSceneMode::Selection;

	void requestReturnToTitle();
	void handleModeInput();
	void handleTopButtonInput();
	void handleRoomSelectionInput();
	void handlePreviewInput();
	void setSelectedRoom(const String& roomId);
	void toggleSelectedRoomViewed();
	void markAllRoomsViewed();
	void resetViewedRooms();
	[[nodiscard]] const BonusRoomDefinition* selectedRoom() const;
	[[nodiscard]] bool isRoomViewed(const String& roomId) const;
	[[nodiscard]] Array<const BonusRoomDefinition*> collectSelectionPreviewRooms() const;
	[[nodiscard]] Array<const BonusRoomDefinition*> collectViewedPreviewRooms() const;
	[[nodiscard]] PreviewMode previewModeFromViewerSource() const;
	void drawSelectionPreview(const GameData& data) const;
	void drawGalleryPreview(const GameData& data) const;
	void drawViewerPreview(const GameData& data) const;
	[[nodiscard]] static MenuButtonStyle getPreviewButtonStyle();
	[[nodiscard]] static RectF getLeftPanelRect();
	[[nodiscard]] static RectF getRightPanelRect();
	[[nodiscard]] static RectF getTopButtonRect(int32 index);
	[[nodiscard]] static RectF getRoomRowRect(int32 index);
	[[nodiscard]] static RectF getModeButtonRect(int32 index);
	[[nodiscard]] static RectF getPreviewContentRect();
	[[nodiscard]] static RectF getPreviewBackButtonRect();
	[[nodiscard]] static RectF getSelectionCardRect(int32 index);
	[[nodiscard]] static RectF getGalleryRowRect(int32 index);
	[[nodiscard]] static RectF getViewerPageRect();
	[[nodiscard]] static RectF getViewerPrevButtonRect();
	[[nodiscard]] static RectF getViewerCloseButtonRect();
	[[nodiscard]] static RectF getViewerNextButtonRect();
};
