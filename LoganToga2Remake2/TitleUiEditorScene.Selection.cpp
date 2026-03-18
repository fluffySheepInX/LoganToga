#include "TitleUiEditorScene.h"

namespace TitleUiEditorSceneSelectionDetail
{
	constexpr int32 EditorGridCellSize = 8;

	[[nodiscard]] double SnapToEditorGrid(const double value)
	{
		return (Math::Round(value / EditorGridCellSize) * EditorGridCellSize);
	}

	[[nodiscard]] Vec2 SnapToEditorGrid(const Vec2& value)
	{
		return Vec2{ SnapToEditorGrid(value.x), SnapToEditorGrid(value.y) };
	}
}

void TitleUiEditorScene::ensureSelectedElementVisible()
{
	const int32 visibleRowCount = getSelectionVisibleRowCount();
	if (m_selectedElementIndex < m_selectionScrollRow)
	{
		m_selectionScrollRow = m_selectedElementIndex;
	}
	else if ((m_selectionScrollRow + visibleRowCount) <= m_selectedElementIndex)
	{
		m_selectionScrollRow = (m_selectedElementIndex - visibleRowCount + 1);
	}

	m_selectionScrollRow = Clamp(m_selectionScrollRow, 0, getMaxSelectionScrollRow());
}

void TitleUiEditorScene::applySelectionDelta(const Vec2& delta)
{
	if (RectF* rect = getSelectedRect())
	{
		rect->x = TitleUiEditorSceneSelectionDetail::SnapToEditorGrid(rect->x + delta.x);
		rect->y = TitleUiEditorSceneSelectionDetail::SnapToEditorGrid(rect->y + delta.y);
		markEdited(U"Moved selected rectangle");
		return;
	}

	if (Vec2* point = getSelectedPoint())
	{
		*point = TitleUiEditorSceneSelectionDetail::SnapToEditorGrid(*point + delta);
		markEdited(U"Moved selected point");
	}
}

void TitleUiEditorScene::applySelectionResize(const Vec2& delta)
{
	if (RectF* rect = getSelectedRect())
	{
		rect->w = Max(8.0, rect->w + delta.x);
		rect->h = Max(8.0, rect->h + delta.y);
		markEdited(U"Resized selected rectangle");
	}
}

void TitleUiEditorScene::markEdited(const String& message)
{
	m_hasUnsavedChanges = true;
	m_statusMessage = message;
}

bool TitleUiEditorScene::isRectLikelyTooSmall(const RectF& rect, const RectF& defaultRect)
{
	if ((defaultRect.w > 260.0) || (defaultRect.h > 56.0))
	{
		return false;
	}

	const double minWidth = Max(80.0, defaultRect.w * 0.5);
	const double minHeight = Max(24.0, defaultRect.h * 0.75);
	return (rect.w < minWidth) || (rect.h < minHeight);
}

bool TitleUiEditorScene::isCursorOnControlPanel() const
{
	return getLeftPanelRect().mouseOver() || getRightPanelRect().mouseOver();
}

RectF* TitleUiEditorScene::getSelectedRect()
{
	const EditableElement& element = getSelectedElement();
	return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
}

const RectF* TitleUiEditorScene::getSelectedRect() const
{
	const EditableElement& element = getSelectedElement();
	return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
}

Vec2* TitleUiEditorScene::getSelectedPoint()
{
	const EditableElement& element = getSelectedElement();
	return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
}

const Vec2* TitleUiEditorScene::getSelectedPoint() const
{
	const EditableElement& element = getSelectedElement();
	return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
}

Optional<RectF> TitleUiEditorScene::getSelectedDefaultRect() const
{
	const TitleUiLayout defaults = TitleUi::MakeDefaultTitleUiLayout();
	const EditableElement& element = getSelectedElement();
	if (!element.rectMember)
	{
		return none;
	}

	return defaults.*(element.rectMember);
}

Optional<Vec2> TitleUiEditorScene::getSelectedDefaultPoint() const
{
	const TitleUiLayout defaults = TitleUi::MakeDefaultTitleUiLayout();
	const EditableElement& element = getSelectedElement();
	if (!element.pointMember)
	{
		return none;
	}

	return defaults.*(element.pointMember);
}

Optional<TitleUiEditorScene::ValidationIssue> TitleUiEditorScene::findValidationIssue() const
{
	const auto& elements = getEditableElements();
	const TitleUiLayout defaults = TitleUi::MakeDefaultTitleUiLayout();

	for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
	{
		const EditableElement& element = elements[static_cast<size_t>(i)];
		if (!element.rectMember)
		{
			continue;
		}

		const RectF& rect = (m_layout.*(element.rectMember));
		const RectF defaultRect = defaults.*(element.rectMember);
		if (!isRectLikelyTooSmall(rect, defaultRect))
		{
			continue;
		}

		return ValidationIssue{
			.elementIndex = i,
			.message = U"Save blocked: " + element.name + U" is too small. Use Default Size or a size preset."
		};
	}

	return none;
}

Vec2 TitleUiEditorScene::toPreviewScreenPos(const Vec2& pos) const
{
	return (pos + m_previewCameraOffset);
}

Vec2 TitleUiEditorScene::toPreviewWorldPos(const Vec2& pos) const
{
	return (pos - m_previewCameraOffset);
}

RectF TitleUiEditorScene::toPreviewScreenRect(const RectF& rect) const
{
	RectF screenRect = rect;
	screenRect.pos.moveBy(m_previewCameraOffset);
	return screenRect;
}

RectF TitleUiEditorScene::getSelectionHandleRect() const
{
	if (const RectF* rect = getSelectedRect())
	{
		return toPreviewScreenRect(*rect);
	}

	if (const Vec2* point = getSelectedPoint())
	{
		const Vec2 screenPoint = toPreviewScreenPos(*point);
		return RectF{ screenPoint.x - 10, screenPoint.y - 10, 20, 20 };
	}

	return RectF{};
}

const TitleUiEditorScene::EditableElement& TitleUiEditorScene::getSelectedElement() const
{
	const auto& elements = getEditableElements();
	return elements[static_cast<size_t>(Clamp(m_selectedElementIndex, 0, static_cast<int32>(elements.size()) - 1))];
}

const Array<TitleUiEditorScene::EditableElement>& TitleUiEditorScene::getEditableElements()
{
	static const Array<EditableElement> elements =
	{
		{ U"Panel", &TitleUiLayout::panelRect, nullptr },
		{ U"Title", nullptr, &TitleUiLayout::titlePos },
		{ U"Subtitle", nullptr, &TitleUiLayout::subtitlePos },
		{ U"Summary 1", nullptr, &TitleUiLayout::summaryLine1Pos },
		{ U"Summary 2", nullptr, &TitleUiLayout::summaryLine2Pos },
		{ U"Summary 3", nullptr, &TitleUiLayout::summaryLine3Pos },
		{ U"Viewed Bonus", nullptr, &TitleUiLayout::viewedBonusRoomsPos },
		{ U"Enter Hint", nullptr, &TitleUiLayout::enterHintPos },
		{ U"Continue Button", &TitleUiLayout::continueButtonRect, nullptr },
		{ U"Tutorial Btn (Cont)", &TitleUiLayout::tutorialButtonRectWithContinue, nullptr },
		{ U"Tutorial Btn (New)", &TitleUiLayout::tutorialButtonRectWithoutContinue, nullptr },
		{ U"QuickGuide Btn (Cont)", &TitleUiLayout::quickGuideButtonRectWithContinue, nullptr },
		{ U"QuickGuide Btn (New)", &TitleUiLayout::quickGuideButtonRectWithoutContinue, nullptr },
		{ U"Start Btn (Cont)", &TitleUiLayout::startButtonRectWithContinue, nullptr },
		{ U"Start Btn (New)", &TitleUiLayout::startButtonRectWithoutContinue, nullptr },
		{ U"Bonus Btn (Cont)", &TitleUiLayout::bonusButtonRectWithContinue, nullptr },
		{ U"Bonus Btn (New)", &TitleUiLayout::bonusButtonRectWithoutContinue, nullptr },
		{ U"Bonus Hint (Cont)", nullptr, &TitleUiLayout::bonusRoomHintPosWithContinue },
		{ U"Bonus Hint (New)", nullptr, &TitleUiLayout::bonusRoomHintPosWithoutContinue },
		{ U"Continue Preview", &TitleUiLayout::continuePreviewRect, nullptr },
		{ U"QuickGuide Panel", &TitleUiLayout::quickGuidePanelRect, nullptr },
		{ U"QuickGuide Body", nullptr, &TitleUiLayout::quickGuideBodyPos },
		{ U"QuickGuide Flow", nullptr, &TitleUiLayout::quickGuideFlowPos },
		{ U"QuickGuide Tutorial", &TitleUiLayout::quickGuideTutorialButtonRect, nullptr },
		{ U"QuickGuide Close", &TitleUiLayout::quickGuideCloseButtonRect, nullptr },
		{ U"QuickGuide Esc", nullptr, &TitleUiLayout::quickGuideEscHintPos },
		{ U"DataClear Dialog", &TitleUiLayout::dataClearDialogRect, nullptr },
		{ U"DataClear Yes", &TitleUiLayout::dataClearDialogYesButtonRect, nullptr },
		{ U"DataClear No", &TitleUiLayout::dataClearDialogNoButtonRect, nullptr },
		{ U"Exit Dialog", &TitleUiLayout::exitDialogRect, nullptr },
		{ U"Exit Yes", &TitleUiLayout::exitDialogYesButtonRect, nullptr },
		{ U"Exit No", &TitleUiLayout::exitDialogNoButtonRect, nullptr },
		{ U"Resolution Label", nullptr, &TitleUiLayout::resolutionLabelPos },
		{ U"Resolution Value", nullptr, &TitleUiLayout::resolutionValuePos },
		{ U"Resolution Small", &TitleUiLayout::resolutionSmallButtonRect, nullptr },
		{ U"Resolution Medium", &TitleUiLayout::resolutionMediumButtonRect, nullptr },
		{ U"Resolution Large", &TitleUiLayout::resolutionLargeButtonRect, nullptr },
		{ U"SaveLocation Label", nullptr, &TitleUiLayout::saveLocationLabelPos },
		{ U"SaveLocation Value", nullptr, &TitleUiLayout::saveLocationValuePos },
		{ U"SaveLocation Button", &TitleUiLayout::saveLocationButtonRect, nullptr },
		{ U"Data Manage Label", nullptr, &TitleUiLayout::dataManagementLabelPos },
		{ U"Clear Continue", &TitleUiLayout::clearContinueRunButtonRect, nullptr },
		{ U"Clear Settings", &TitleUiLayout::clearSettingsButtonRect, nullptr },
		{ U"Exit Button", &TitleUiLayout::exitButtonRect, nullptr },
		{ U"Data Manage Hint", nullptr, &TitleUiLayout::dataManagementHintPos },
		{ U"Map Edit", &TitleUiLayout::mapEditButtonRect, nullptr },
		{ U"Balance Edit", &TitleUiLayout::balanceEditButtonRect, nullptr },
		{ U"Transition Preset", &TitleUiLayout::transitionPresetButtonRect, nullptr },
		{ U"TitleUi Editor", &TitleUiLayout::titleUiEditorButtonRect, nullptr },
       { U"Reward Editor", &TitleUiLayout::rewardEditorButtonRect, nullptr },
		{ U"BonusRoom Editor", &TitleUiLayout::bonusRoomEditorButtonRect, nullptr },
#ifdef _DEBUG
		{ U"Debug Hint (Cont)", nullptr, &TitleUiLayout::debugHintPosWithContinue },
		{ U"Debug Hint (New)", nullptr, &TitleUiLayout::debugHintPosWithoutContinue },
		{ U"Debug Btn (Cont)", &TitleUiLayout::debugButtonRectWithContinue, nullptr },
		{ U"Debug Btn (New)", &TitleUiLayout::debugButtonRectWithoutContinue, nullptr },
#endif
	};
	return elements;
}
