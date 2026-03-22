#include "TitleUiEditorScene.h"

#include "Localization.h"

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
        markEdited(Localization::GetText(U"title_ui_editor.status.moved_rectangle"));
		return;
	}

	if (Vec2* point = getSelectedPoint())
	{
		*point = TitleUiEditorSceneSelectionDetail::SnapToEditorGrid(*point + delta);
        markEdited(Localization::GetText(U"title_ui_editor.status.moved_point"));
	}
}

void TitleUiEditorScene::applySelectionResize(const Vec2& delta)
{
	if (RectF* rect = getSelectedRect())
	{
		rect->w = Max(8.0, rect->w + delta.x);
		rect->h = Max(8.0, rect->h + delta.y);
      markEdited(Localization::GetText(U"title_ui_editor.status.resized_rectangle"));
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
          .message = Localization::FormatText(U"title_ui_editor.validation.too_small", Localization::GetText(element.name))
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
       { U"title_ui_editor.element.panel", &TitleUiLayout::panelRect, nullptr },
		{ U"title_ui_editor.element.title", nullptr, &TitleUiLayout::titlePos },
		{ U"title_ui_editor.element.subtitle", nullptr, &TitleUiLayout::subtitlePos },
		{ U"title_ui_editor.element.summary1", nullptr, &TitleUiLayout::summaryLine1Pos },
		{ U"title_ui_editor.element.summary2", nullptr, &TitleUiLayout::summaryLine2Pos },
		{ U"title_ui_editor.element.summary3", nullptr, &TitleUiLayout::summaryLine3Pos },
		{ U"title_ui_editor.element.viewed_bonus", nullptr, &TitleUiLayout::viewedBonusRoomsPos },
		{ U"title_ui_editor.element.enter_hint", nullptr, &TitleUiLayout::enterHintPos },
		{ U"title_ui_editor.element.continue_button", &TitleUiLayout::continueButtonRect, nullptr },
		{ U"title_ui_editor.element.tutorial_button_continue", &TitleUiLayout::tutorialButtonRectWithContinue, nullptr },
		{ U"title_ui_editor.element.tutorial_button_new", &TitleUiLayout::tutorialButtonRectWithoutContinue, nullptr },
		{ U"title_ui_editor.element.quick_guide_button_continue", &TitleUiLayout::quickGuideButtonRectWithContinue, nullptr },
		{ U"title_ui_editor.element.quick_guide_button_new", &TitleUiLayout::quickGuideButtonRectWithoutContinue, nullptr },
		{ U"title_ui_editor.element.start_button_continue", &TitleUiLayout::startButtonRectWithContinue, nullptr },
		{ U"title_ui_editor.element.start_button_new", &TitleUiLayout::startButtonRectWithoutContinue, nullptr },
		{ U"title_ui_editor.element.bonus_button_continue", &TitleUiLayout::bonusButtonRectWithContinue, nullptr },
		{ U"title_ui_editor.element.bonus_button_new", &TitleUiLayout::bonusButtonRectWithoutContinue, nullptr },
		{ U"title_ui_editor.element.bonus_hint_continue", nullptr, &TitleUiLayout::bonusRoomHintPosWithContinue },
		{ U"title_ui_editor.element.bonus_hint_new", nullptr, &TitleUiLayout::bonusRoomHintPosWithoutContinue },
		{ U"title_ui_editor.element.continue_preview", &TitleUiLayout::continuePreviewRect, nullptr },
		{ U"title_ui_editor.element.quick_guide_panel", &TitleUiLayout::quickGuidePanelRect, nullptr },
		{ U"title_ui_editor.element.quick_guide_body", nullptr, &TitleUiLayout::quickGuideBodyPos },
		{ U"title_ui_editor.element.quick_guide_flow", nullptr, &TitleUiLayout::quickGuideFlowPos },
		{ U"title_ui_editor.element.quick_guide_tutorial", &TitleUiLayout::quickGuideTutorialButtonRect, nullptr },
		{ U"title_ui_editor.element.quick_guide_close", &TitleUiLayout::quickGuideCloseButtonRect, nullptr },
		{ U"title_ui_editor.element.quick_guide_esc", nullptr, &TitleUiLayout::quickGuideEscHintPos },
		{ U"title_ui_editor.element.data_clear_dialog", &TitleUiLayout::dataClearDialogRect, nullptr },
		{ U"title_ui_editor.element.data_clear_yes", &TitleUiLayout::dataClearDialogYesButtonRect, nullptr },
		{ U"title_ui_editor.element.data_clear_no", &TitleUiLayout::dataClearDialogNoButtonRect, nullptr },
		{ U"title_ui_editor.element.exit_dialog", &TitleUiLayout::exitDialogRect, nullptr },
		{ U"title_ui_editor.element.exit_yes", &TitleUiLayout::exitDialogYesButtonRect, nullptr },
		{ U"title_ui_editor.element.exit_no", &TitleUiLayout::exitDialogNoButtonRect, nullptr },
		{ U"title_ui_editor.element.resolution_label", nullptr, &TitleUiLayout::resolutionLabelPos },
		{ U"title_ui_editor.element.resolution_value", nullptr, &TitleUiLayout::resolutionValuePos },
		{ U"title_ui_editor.element.resolution_small", &TitleUiLayout::resolutionSmallButtonRect, nullptr },
		{ U"title_ui_editor.element.resolution_medium", &TitleUiLayout::resolutionMediumButtonRect, nullptr },
		{ U"title_ui_editor.element.resolution_large", &TitleUiLayout::resolutionLargeButtonRect, nullptr },
		{ U"title_ui_editor.element.save_location_label", nullptr, &TitleUiLayout::saveLocationLabelPos },
		{ U"title_ui_editor.element.save_location_value", nullptr, &TitleUiLayout::saveLocationValuePos },
		{ U"title_ui_editor.element.save_location_button", &TitleUiLayout::saveLocationButtonRect, nullptr },
		{ U"title_ui_editor.element.data_manage_label", nullptr, &TitleUiLayout::dataManagementLabelPos },
		{ U"title_ui_editor.element.clear_continue", &TitleUiLayout::clearContinueRunButtonRect, nullptr },
		{ U"title_ui_editor.element.clear_settings", &TitleUiLayout::clearSettingsButtonRect, nullptr },
		{ U"title_ui_editor.element.exit_button", &TitleUiLayout::exitButtonRect, nullptr },
		{ U"title_ui_editor.element.data_manage_hint", nullptr, &TitleUiLayout::dataManagementHintPos },
		{ U"title_ui_editor.element.map_edit", &TitleUiLayout::mapEditButtonRect, nullptr },
		{ U"title_ui_editor.element.balance_edit", &TitleUiLayout::balanceEditButtonRect, nullptr },
		{ U"title_ui_editor.element.transition_preset", &TitleUiLayout::transitionPresetButtonRect, nullptr },
		{ U"title_ui_editor.element.title_ui_editor", &TitleUiLayout::titleUiEditorButtonRect, nullptr },
		{ U"title_ui_editor.element.reward_editor", &TitleUiLayout::rewardEditorButtonRect, nullptr },
		{ U"title_ui_editor.element.bonus_room_editor", &TitleUiLayout::bonusRoomEditorButtonRect, nullptr },
#ifdef _DEBUG
        { U"title_ui_editor.element.debug_hint_continue", nullptr, &TitleUiLayout::debugHintPosWithContinue },
		{ U"title_ui_editor.element.debug_hint_new", nullptr, &TitleUiLayout::debugHintPosWithoutContinue },
		{ U"title_ui_editor.element.debug_button_continue", &TitleUiLayout::debugButtonRectWithContinue, nullptr },
		{ U"title_ui_editor.element.debug_button_new", &TitleUiLayout::debugButtonRectWithoutContinue, nullptr },
#endif
	};
	return elements;
}
