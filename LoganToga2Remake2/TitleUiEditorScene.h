#pragma once

#include "GameData.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"
#include "TitleUiLayout.h"

class TitleUiEditorScene : public SceneBase
{
private:
	struct EditableElement
	{
		String name;
		RectF TitleUiLayout::* rectMember = nullptr;
		Vec2 TitleUiLayout::* pointMember = nullptr;
	};

public:
	explicit TitleUiEditorScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	TitleUiLayout m_layout;
	int32 m_selectedElementIndex = 0;
	int32 m_selectionScrollRow = 0;
	String m_statusMessage = U"Title UI editor ready";
	bool m_hasUnsavedChanges = false;
	bool m_previewHasContinue = true;
	bool m_previewHasViewedBonusRooms = true;
	bool m_previewQuickGuideOpen = false;
	bool m_previewDataClearDialogOpen = false;
	bool m_previewExitDialogOpen = false;
#ifdef _DEBUG
	bool m_previewDebugButtons = true;
#endif
	Optional<Vec2> m_dragOffset;
	bool m_isDraggingPoint = false;

	void reloadLayout();
	void saveLayout();
	void resetSelectedElement();
	void resetAllElements();
	void ensureSelectedElementVisible();
	void handleSelectionInput();
	void handleTopButtonInput();
	void handlePreviewToggleInput();
	void handleEditorShortcuts();
	void handleDragInput();
	void applySelectionDelta(const Vec2& delta);
	void applySelectionResize(const Vec2& delta);
	void markEdited(const String& message);
	void requestReturnToTitle();
	[[nodiscard]] bool isCursorOnControlPanel() const;

	[[nodiscard]] RectF* getSelectedRect();
	[[nodiscard]] const RectF* getSelectedRect() const;
	[[nodiscard]] Vec2* getSelectedPoint();
	[[nodiscard]] const Vec2* getSelectedPoint() const;
	[[nodiscard]] RectF getSelectionHandleRect() const;
	[[nodiscard]] const EditableElement& getSelectedElement() const;
	[[nodiscard]] static const Array<EditableElement>& getEditableElements();

	void drawPreview() const;
	void drawPanels() const;
	void drawSelectionHighlight() const;

	[[nodiscard]] static bool isButtonClicked(const RectF& rect);
	static void drawButton(const RectF& rect, const String& label, const Font& font, bool selected = false);
	[[nodiscard]] static RectF getLeftPanelRect();
	[[nodiscard]] static RectF getRightPanelRect();
	[[nodiscard]] static RectF getTopButtonRect(int32 index);
	[[nodiscard]] static RectF getToggleButtonRect(int32 index);
	[[nodiscard]] static RectF getSelectionListRect();
	[[nodiscard]] int32 getMaxSelectionScrollRow() const;
	[[nodiscard]] static int32 getSelectionVisibleRowCount();
	[[nodiscard]] static RectF getSelectionRowRect(int32 index);
};
