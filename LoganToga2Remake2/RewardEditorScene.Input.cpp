#include "RewardEditorScene.h"

void RewardEditorScene::handleTopButtonInput()
{
	if (isButtonClicked(getTopButtonRect(0)))
	{
		if (RewardUi::SaveRewardUiLayout(m_layout))
		{
			m_hasUnsavedChanges = false;
            m_statusMessage = Localization::GetText(U"reward_editor.status_saved_layout");
		}
		else
		{
         m_statusMessage = Localization::GetText(U"reward_editor.status_save_failed");
		}
		return;
	}

	if (isButtonClicked(getTopButtonRect(1)))
	{
		m_layout = RewardUi::ReloadRewardUiLayout();
		m_hasUnsavedChanges = false;
       m_statusMessage = Localization::GetText(U"reward_editor.status_reloaded_layout");
		return;
	}

	if (isButtonClicked(getTopButtonRect(2)))
	{
		m_layout = RewardUi::MakeDefaultRewardUiLayout();
		m_hasUnsavedChanges = true;
     m_statusMessage = Localization::GetText(U"reward_editor.status_reset_layout");
		return;
	}

	if (isButtonClicked(getTopButtonRect(3)) || KeyR.down())
	{
        rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_rerolled"));
		return;
	}

	if (isButtonClicked(getTopButtonRect(4)) || KeyC.down())
	{
		resetPreviewState();
		return;
	}

	if (isButtonClicked(getTopButtonRect(5)) || KeyEscape.down())
	{
		requestReturnToTitle();
	}
}

void RewardEditorScene::handleSelectionInput()
{
	const auto& elements = getEditableElements();
	for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
	{
		if (isButtonClicked(getSelectionRowRect(i)))
		{
			m_selectedElementIndex = i;
			return;
		}
	}
}

void RewardEditorScene::handleDragInput()
{
	if (isCursorOnControlPanel())
	{
		if (!MouseL.pressed())
		{
			m_dragOffset.reset();
		}
		return;
	}

	if (Vec2* point = getSelectedPoint())
	{
		const Circle handle{ *point, 10.0 };
		if (!m_dragOffset && MouseL.down() && handle.mouseOver())
		{
			m_dragOffset = (Cursor::PosF() - *point);
			m_isDraggingPoint = true;
		}

		if (m_dragOffset)
		{
			if (MouseL.pressed())
			{
				*point = snapToGrid(Cursor::PosF() - *m_dragOffset);
				m_hasUnsavedChanges = true;
             m_statusMessage = Localization::GetText(U"reward_editor.status_dragging");
			}
			else
			{
				m_dragOffset.reset();
			}
		}
		return;
	}

	if (RectF* rect = getSelectedRect())
	{
		if (!m_dragOffset && MouseL.down() && rect->mouseOver())
		{
			m_dragOffset = (Cursor::PosF() - rect->pos);
			m_isDraggingPoint = false;
		}

		if (m_dragOffset)
		{
			if (MouseL.pressed())
			{
				const Vec2 snappedPos = snapToGrid(Cursor::PosF() - *m_dragOffset);
				rect->x = snappedPos.x;
				rect->y = snappedPos.y;
				m_hasUnsavedChanges = true;
             m_statusMessage = Localization::GetText(U"reward_editor.status_dragging");
			}
			else
			{
				m_dragOffset.reset();
			}
		}
	}
}

RectF* RewardEditorScene::getSelectedRect()
{
	const EditableElement& element = getEditableElements()[m_selectedElementIndex];
	return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
}

const RectF* RewardEditorScene::getSelectedRect() const
{
	const EditableElement& element = getEditableElements()[m_selectedElementIndex];
	return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
}

Vec2* RewardEditorScene::getSelectedPoint()
{
	const EditableElement& element = getEditableElements()[m_selectedElementIndex];
	return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
}

const Vec2* RewardEditorScene::getSelectedPoint() const
{
	const EditableElement& element = getEditableElements()[m_selectedElementIndex];
	return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
}

bool RewardEditorScene::isCursorOnControlPanel() const
{
	return getLeftPanelRect().mouseOver();
}

bool RewardEditorScene::isButtonClicked(const RectF& rect)
{
	return IsMenuButtonClicked(rect);
}

RectF RewardEditorScene::getLeftPanelRect()
{
	return RectF{ 12, 12, 300, Scene::Height() - 24 };
}

RectF RewardEditorScene::getPreviewGridRect()
{
	const RectF panel = getLeftPanelRect();
	return RectF{ panel.rightX() + 12, 12, Scene::Width() - panel.rightX() - 24, Scene::Height() - 24 };
}

double RewardEditorScene::snapToGridValue(const double value)
{
	return (Math::Round(value / EditorGridCellSize) * EditorGridCellSize);
}

Vec2 RewardEditorScene::snapToGrid(const Vec2& value)
{
	return Vec2{ snapToGridValue(value.x), snapToGridValue(value.y) };
}

RectF RewardEditorScene::getTopButtonRect(const int32 index)
{
	const RectF panel = getLeftPanelRect();
	return RectF{ panel.x + 16, panel.y + 76 + (index * 30), panel.w - 32, 26 };
}

RectF RewardEditorScene::getSelectionRowRect(const int32 index)
{
	const RectF panel = getLeftPanelRect();
	return RectF{ panel.x + 16, panel.y + 262 + (index * 24), panel.w - 32, 22 };
}

const Array<RewardEditorScene::EditableElement>& RewardEditorScene::getEditableElements()
{
	static const Array<EditableElement> elements =
	{
		{ U"Title", nullptr, &RewardUiLayout::titlePos },
		{ U"Subtitle", nullptr, &RewardUiLayout::subtitlePos },
		{ U"Hint", nullptr, &RewardUiLayout::hintPos },
		{ U"Card 1", &RewardUiLayout::card1Rect, nullptr },
		{ U"Card 2", &RewardUiLayout::card2Rect, nullptr },
		{ U"Card 3", &RewardUiLayout::card3Rect, nullptr },
		{ U"Acquired Title", nullptr, &RewardUiLayout::acquiredLabelPos },
		{ U"Acquired Name", nullptr, &RewardUiLayout::acquiredCardNamePos },
	};
	return elements;
}
