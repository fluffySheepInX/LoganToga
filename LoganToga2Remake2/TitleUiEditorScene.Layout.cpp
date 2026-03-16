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

RectF TitleUiEditorScene::getPreviewViewportRect()
{
	const RectF left = getLeftPanelRect();
	const RectF right = getRightPanelRect();
	return RectF{ left.x + left.w + 12, 12, right.x - (left.x + left.w) - 24, Scene::Height() - 24 };
}

RectF TitleUiEditorScene::getSelectionListRect()
{
	const RectF left = getLeftPanelRect();
	return RectF{ left.x + 16, left.y + 246, left.w - 32, left.h - 262 };
}

RectF TitleUiEditorScene::getInfoPresetButtonRect(const int32 index)
{
	const RectF right = getRightPanelRect();
	const RectF infoRect{ right.x + 16, right.y + 214, right.w - 32, right.h - 230 };
	const double marginX = 12.0;
	const double spacing = 8.0;
	const double buttonWidth = ((infoRect.w - (marginX * 2.0) - spacing) * 0.5);
	const double buttonHeight = 26.0;
	const double fullWidth = (infoRect.w - (marginX * 2.0));
	const double startY = (infoRect.y + 178.0);

	switch (index)
	{
	case 0:
		return RectF{ infoRect.x + marginX, startY, buttonWidth, buttonHeight };
	case 1:
		return RectF{ infoRect.x + marginX + buttonWidth + spacing, startY, buttonWidth, buttonHeight };
	case 2:
		return RectF{ infoRect.x + marginX, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
	case 3:
		return RectF{ infoRect.x + marginX + buttonWidth + spacing, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
	case 4:
	default:
		return RectF{ infoRect.x + marginX, startY + ((buttonHeight + spacing) * 2.0), fullWidth, buttonHeight };
	}
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
