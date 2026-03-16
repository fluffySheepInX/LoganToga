#include "TitleUiEditorScene.h"

namespace TitleUiEditorSceneInputDetail
{
	constexpr int32 EditorGridCellSize = 8;
	constexpr int32 EditorGridMajorLineSpan = 4;

	[[nodiscard]] double SnapToEditorGrid(const double value)
	{
		return (Math::Round(value / EditorGridCellSize) * EditorGridCellSize);
	}

	[[nodiscard]] Vec2 SnapToEditorGrid(const Vec2& value)
	{
		return Vec2{ SnapToEditorGrid(value.x), SnapToEditorGrid(value.y) };
	}
}

void TitleUiEditorScene::handleSelectionInput()
{
	const auto& elements = getEditableElements();
	if (getSelectionListRect().mouseOver())
	{
		m_selectionScrollRow = Clamp(m_selectionScrollRow - static_cast<int32>(Mouse::Wheel()), 0, getMaxSelectionScrollRow());
	}

	const int32 visibleRowCount = getSelectionVisibleRowCount();
	for (int32 visibleIndex = 0; visibleIndex < visibleRowCount; ++visibleIndex)
	{
		const int32 actualIndex = (m_selectionScrollRow + visibleIndex);
		if (static_cast<int32>(elements.size()) <= actualIndex)
		{
			break;
		}

		if (isButtonClicked(getSelectionRowRect(visibleIndex)))
		{
			m_selectedElementIndex = actualIndex;
			m_statusMessage = U"Selected: " + elements[static_cast<size_t>(actualIndex)].name;
			return;
		}
	}

	if (KeyW.down())
	{
		m_selectedElementIndex = Max(0, m_selectedElementIndex - 1);
		ensureSelectedElementVisible();
		m_statusMessage = U"Selected: " + getSelectedElement().name;
	}
	else if (KeyS.down())
	{
		m_selectedElementIndex = Min(static_cast<int32>(elements.size()) - 1, m_selectedElementIndex + 1);
		ensureSelectedElementVisible();
		m_statusMessage = U"Selected: " + getSelectedElement().name;
	}
}

void TitleUiEditorScene::handleTopButtonInput()
{
	if (KeyEscape.down() || isButtonClicked(getTopButtonRect(0)))
	{
		requestReturnToTitle();
		return;
	}

	if (isButtonClicked(getTopButtonRect(1)))
	{
		saveLayout();
		return;
	}

	if (isButtonClicked(getTopButtonRect(2)))
	{
		reloadLayout();
		return;
	}

	if (isButtonClicked(getTopButtonRect(3)))
	{
		resetSelectedElement();
		return;
	}

	if (isButtonClicked(getTopButtonRect(4)))
	{
		resetAllElements();
	}
}

void TitleUiEditorScene::handlePreviewToggleInput()
{
	if (isButtonClicked(getToggleButtonRect(0)))
	{
		m_previewHasContinue = !m_previewHasContinue;
		m_statusMessage = U"Preview: toggled continue state";
		return;
	}

	if (isButtonClicked(getToggleButtonRect(1)))
	{
		m_previewHasViewedBonusRooms = !m_previewHasViewedBonusRooms;
		m_statusMessage = U"Preview: toggled bonus room state";
		return;
	}

	if (isButtonClicked(getToggleButtonRect(2)))
	{
		m_previewQuickGuideOpen = !m_previewQuickGuideOpen;
		m_statusMessage = U"Preview: toggled quick guide";
		return;
	}

	if (isButtonClicked(getToggleButtonRect(3)))
	{
		m_previewDataClearDialogOpen = !m_previewDataClearDialogOpen;
		m_statusMessage = U"Preview: toggled data clear dialog";
		return;
	}

	if (isButtonClicked(getToggleButtonRect(4)))
	{
		m_previewExitDialogOpen = !m_previewExitDialogOpen;
		m_statusMessage = U"Preview: toggled exit dialog";
		return;
	}

#ifdef _DEBUG
	if (isButtonClicked(getToggleButtonRect(5)))
	{
		m_previewDebugButtons = !m_previewDebugButtons;
		m_statusMessage = U"Preview: toggled debug buttons";
	}
#endif
}

void TitleUiEditorScene::handleInfoPanelInput()
{
	if (!getSelectedRect())
	{
		return;
	}

	if (isButtonClicked(getInfoPresetButtonRect(0)))
	{
		applySelectedRectSizePreset(Vec2{ 220, 36 }, U"220x36");
		return;
	}

	if (isButtonClicked(getInfoPresetButtonRect(1)))
	{
		applySelectedRectSizePreset(Vec2{ 170, 32 }, U"170x32");
		return;
	}

	if (isButtonClicked(getInfoPresetButtonRect(2)))
	{
		applySelectedRectSizePreset(Vec2{ 140, 40 }, U"140x40");
		return;
	}

	if (isButtonClicked(getInfoPresetButtonRect(3)))
	{
		applySelectedRectSizePreset(Vec2{ 128, 30 }, U"128x30");
		return;
	}

	if (isButtonClicked(getInfoPresetButtonRect(4)))
	{
		if (const auto defaultRect = getSelectedDefaultRect())
		{
			applySelectedRectSizePreset(defaultRect->size, U"Default Size");
		}
	}
}

void TitleUiEditorScene::handleEditorShortcuts()
{
	const bool controlPressed = s3d::KeyControl.pressed();
	if (controlPressed && s3d::KeyS.down())
	{
		saveLayout();
		return;
	}

	if (controlPressed && KeyR.down())
	{
		reloadLayout();
		return;
	}

	if (!controlPressed && KeyR.down())
	{
		if (KeyShift.pressed())
		{
			resetAllElements();
		}
		else
		{
			resetSelectedElement();
		}
		return;
	}

	const int32 step = KeyShift.pressed()
		? (TitleUiEditorSceneInputDetail::EditorGridCellSize * TitleUiEditorSceneInputDetail::EditorGridMajorLineSpan)
		: TitleUiEditorSceneInputDetail::EditorGridCellSize;
	Vec2 delta{ 0, 0 };
	if (s3d::KeyLeft.pressed())
	{
		delta.x -= step;
	}
	if (s3d::KeyRight.pressed())
	{
		delta.x += step;
	}
	if (s3d::KeyUp.pressed())
	{
		delta.y -= step;
	}
	if (s3d::KeyDown.pressed())
	{
		delta.y += step;
	}

	if (delta == Vec2{ 0, 0 })
	{
		return;
	}

	if (controlPressed)
	{
		applySelectionResize(delta);
	}
	else
	{
		applySelectionDelta(delta);
	}
}

void TitleUiEditorScene::handleDragInput()
{
	if (isCursorOnControlPanel())
	{
		if (!MouseL.pressed())
		{
			m_dragOffset.reset();
			m_isDraggingPoint = false;
			m_previewPanAnchor.reset();
			m_isPanningPreview = false;
		}
		return;
	}

	const Vec2 cursorPos = Cursor::PosF();
	const Vec2 cursorWorldPos = toPreviewWorldPos(cursorPos);

	if (const Vec2* point = getSelectedPoint())
	{
		const Circle handle{ toPreviewScreenPos(*point), 12 };
		if (!m_dragOffset && !m_isPanningPreview && MouseL.down() && handle.mouseOver())
		{
			m_dragOffset = (*point - cursorWorldPos);
			m_isDraggingPoint = true;
		}
	}
	else if (const RectF* rect = getSelectedRect())
	{
		const RectF screenRect = toPreviewScreenRect(*rect);
		if (!m_dragOffset && !m_isPanningPreview && MouseL.down() && screenRect.mouseOver())
		{
			m_dragOffset = (cursorWorldPos - rect->pos);
			m_isDraggingPoint = false;
		}
	}

	if (!m_dragOffset && !m_isPanningPreview && MouseL.down() && getPreviewViewportRect().mouseOver())
	{
		m_previewPanAnchor = cursorPos;
		m_previewPanStartOffset = m_previewCameraOffset;
		m_isPanningPreview = true;
		m_statusMessage = U"Panning preview camera";
	}

	if (!MouseL.pressed())
	{
		m_dragOffset.reset();
		m_previewPanAnchor.reset();
		m_isPanningPreview = false;
		return;
	}

	if (m_isPanningPreview && m_previewPanAnchor)
	{
		m_previewCameraOffset = (m_previewPanStartOffset + (cursorPos - *m_previewPanAnchor));
		return;
	}

	if (!m_dragOffset)
	{
		return;
	}

	if (Vec2* point = getSelectedPoint(); m_isDraggingPoint && point)
	{
		*point = TitleUiEditorSceneInputDetail::SnapToEditorGrid(cursorWorldPos + *m_dragOffset);
		markEdited(U"Dragged point element");
	}
	else if (RectF* rect = getSelectedRect(); rect)
	{
		rect->pos = TitleUiEditorSceneInputDetail::SnapToEditorGrid(cursorWorldPos - *m_dragOffset);
		markEdited(U"Dragged rectangle element");
	}
}
