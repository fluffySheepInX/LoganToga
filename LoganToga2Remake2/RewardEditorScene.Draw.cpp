#include "RewardEditorScene.h"

void RewardEditorScene::draw() const
{
	RewardUi::DrawRewardSelectionScreen(getData(), m_previewRunState, m_selectedCardIndex, m_selectionEffectTime, m_layout);
	drawPreviewGrid();
	drawSelectionHighlight();
	drawPanels();
	DrawSceneTransitionOverlay(getData());
}

void RewardEditorScene::drawPanels() const
{
	const RectF panel = getLeftPanelRect();
	panel.draw(ColorF{ 0.05, 0.07, 0.09, 0.96 });
	panel.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });

	const auto& data = getData();
    data.uiFont(Localization::GetText(U"reward_editor.panel_title")).draw(panel.x + 16, panel.y + 14, Palette::White);
	data.smallFont(m_hasUnsavedChanges
       ? Localization::GetText(U"reward_editor.unsaved_changes")
		: Localization::GetText(U"reward_editor.saved_state"))
		.draw(panel.x + 16, panel.y + 46, ColorF{ 0.82, 0.88, 0.96 });

  drawButton(getTopButtonRect(0), Localization::GetText(U"reward_editor.save_layout_button"), data.smallFont, true);
	drawButton(getTopButtonRect(1), Localization::GetText(U"reward_editor.reload_layout_button"), data.smallFont);
	drawButton(getTopButtonRect(2), Localization::GetText(U"reward_editor.reset_layout_button"), data.smallFont);
	drawButton(getTopButtonRect(3), Localization::GetText(U"reward_editor.reroll_button"), data.smallFont);
	drawButton(getTopButtonRect(4), Localization::GetText(U"reward_editor.reset_preview_button"), data.smallFont);
	drawButton(getTopButtonRect(5), Localization::GetText(U"reward_editor.back_button"), data.smallFont);

    data.smallFont(Localization::GetText(U"reward_editor.element_list_title")).draw(panel.x + 16, panel.y + 236, Palette::White);
	const auto& elements = getEditableElements();
	for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
	{
		drawButton(getSelectionRowRect(i), elements[i].name, data.smallFont, (i == m_selectedElementIndex));
	}

   data.smallFont(Localization::GetText(U"reward_editor.editor_hint"))
		.draw(panel.x + 16, panel.bottomY() - 74, ColorF{ 0.88, 0.92, 1.0 });
   data.smallFont(Localization::GetText(U"reward_editor.grid_hint"))
		.draw(panel.x + 16, panel.bottomY() - 60, ColorF{ 0.84, 0.90, 0.98 });
    data.smallFont(Localization::FormatText(U"reward_editor.selected_cards", m_previewRunState.selectedCardIds.size()))
		.draw(panel.x + 16, panel.bottomY() - 38, ColorF{ 0.88, 0.93, 1.0 });
	data.smallFont(m_statusMessage).draw(panel.x + 16, panel.bottomY() - 20, ColorF{ 0.84, 0.90, 0.98 });
}

void RewardEditorScene::drawPreviewGrid() const
{
	const RectF gridRect = getPreviewGridRect();
	for (double x = gridRect.x; x <= gridRect.rightX(); x += EditorGridCellSize)
	{
		const int32 cellIndex = static_cast<int32>(Math::Round((x - gridRect.x) / EditorGridCellSize));
		const bool isMajorLine = ((Abs(cellIndex) % EditorGridMajorLineSpan) == 0);
		Line{ x, gridRect.y, x, gridRect.bottomY() }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}

	for (double y = gridRect.y; y <= gridRect.bottomY(); y += EditorGridCellSize)
	{
		const int32 cellIndex = static_cast<int32>(Math::Round((y - gridRect.y) / EditorGridCellSize));
		const bool isMajorLine = ((Abs(cellIndex) % EditorGridMajorLineSpan) == 0);
		Line{ gridRect.x, y, gridRect.rightX(), y }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
	}
}

void RewardEditorScene::drawSelectionHighlight() const
{
	const RectF* rect = getSelectedRect();
	if (rect)
	{
		rect->stretched(4.0).drawFrame(3.0, 0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
		return;
	}

	const Vec2* point = getSelectedPoint();
	if (point)
	{
		Circle{ *point, 9.0 }.draw(ColorF{ 0.98, 0.90, 0.42, 0.26 });
		Circle{ *point, 5.0 }.drawFrame(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
		Line{ point->movedBy(-12, 0), point->movedBy(12, 0) }.draw(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
		Line{ point->movedBy(0, -12), point->movedBy(0, 12) }.draw(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
	}
}

void RewardEditorScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}
