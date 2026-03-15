#include "TitleUiEditorScene.h"

bool TitleUiEditorScene::isButtonClicked(const RectF& rect)
{
	return IsMenuButtonClicked(rect);
}

RectF TitleUiEditorScene::getLeftPanelRect()
{
	return RectF{ 12, 12, 280, Scene::Height() - 24 };
}

RectF TitleUiEditorScene::getRightPanelRect()
{
	return RectF{ Scene::Width() - 312, 12, 300, Scene::Height() - 24 };
}

RectF TitleUiEditorScene::getTopButtonRect(const int32 index)
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 84 + (index * 30), left.w - 32, 26 };
}

RectF TitleUiEditorScene::getToggleButtonRect(const int32 index)
{
	const RectF right = getRightPanelRect();
	return RectF{ right.x + 16, right.y + 44 + (index * 28), right.w - 32, 24 };
}

RectF TitleUiEditorScene::getSelectionListRect()
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 246, left.w - 32, left.h - 262 };
}

int32 TitleUiEditorScene::getMaxSelectionScrollRow() const
{
	return Max(0, static_cast<int32>(getEditableElements().size()) - getSelectionVisibleRowCount());
}

int32 TitleUiEditorScene::getSelectionVisibleRowCount()
{
	return Max(1, static_cast<int32>((getSelectionListRect().h + 2) / 24));
}

RectF TitleUiEditorScene::getSelectionRowRect(const int32 index)
{
	const RectF listRect = getSelectionListRect();
	return RectF{ listRect.x, listRect.y + (index * 24), listRect.w, 22 };
}
