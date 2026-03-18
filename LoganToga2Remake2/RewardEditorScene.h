#pragma once

#include "GameData.h"
#include "MenuButtonUi.h"
#include "RewardUiLayout.h"
#include "SceneTransition.h"

class RewardEditorScene : public SceneBase
{
private:
	struct EditableElement
	{
		String name;
		RectF RewardUiLayout::* rectMember = nullptr;
		Vec2 RewardUiLayout::* pointMember = nullptr;
	};

public:
	explicit RewardEditorScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	static constexpr double SelectionEffectDuration = 0.42;
	static constexpr int32 EditorGridCellSize = 8;
	static constexpr int32 EditorGridMajorLineSpan = 4;
	RewardUiLayout m_layout;
	RunState m_previewRunState;
	Optional<int32> m_selectedCardIndex;
	double m_selectionEffectTime = 0.0;
	String m_statusMessage = U"Reward editor ready";
	bool m_hasUnsavedChanges = false;
	int32 m_selectedElementIndex = 0;
	Optional<Vec2> m_dragOffset;
	bool m_isDraggingPoint = false;

	void initializePreviewRun();
	void rebuildPreviewChoices(const String& statusMessage);
	void resetPreviewState();
	void choosePreviewCard(int32 index);
	void finishSelectedCardPreview();
	void requestReturnToTitle();
	void handleTopButtonInput();
	void handleSelectionInput();
	void handleDragInput();
	[[nodiscard]] RectF* getSelectedRect();
	[[nodiscard]] const RectF* getSelectedRect() const;
	[[nodiscard]] Vec2* getSelectedPoint();
	[[nodiscard]] const Vec2* getSelectedPoint() const;
	void drawPanels() const;
	void drawPreviewGrid() const;
	void drawSelectionHighlight() const;
	[[nodiscard]] bool isCursorOnControlPanel() const;
	[[nodiscard]] static bool isButtonClicked(const RectF& rect);
	static void drawButton(const RectF& rect, const String& label, const Font& font, bool selected = false);
	[[nodiscard]] static RectF getLeftPanelRect();
	[[nodiscard]] static RectF getPreviewGridRect();
	[[nodiscard]] static double snapToGridValue(double value);
	[[nodiscard]] static Vec2 snapToGrid(const Vec2& value);
	[[nodiscard]] static RectF getTopButtonRect(int32 index);
	[[nodiscard]] static RectF getSelectionRowRect(int32 index);
	[[nodiscard]] static const Array<EditableElement>& getEditableElements();
};
