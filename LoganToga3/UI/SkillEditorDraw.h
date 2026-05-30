#pragma once
# include "SkillEditorDraw.Sandbox.h"
# include "SkillEditorDraw.DetailPanel.h"

namespace LT3
{
	inline void DrawSkillEditor(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showSkillEditor)
		{
			return;
		}

		const RectF panel = SkillEditorPanelRect();
		const RectF list = SkillEditorListViewportRect();
		const RectF detail = SkillEditorDetailRect();
		const RectF unitViewport = SkillEditorUnitViewportRect();
		const RectF detailViewport = SkillEditorDetailViewportRect();
		DrawSkillEditorSandboxPreview(editor, defs, catalog, uiFont);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Skill Editor").draw(16, panel.x + 18.0, panel.y + 14.0, Palette::White);
		DrawRectButton(SkillEditorSandboxToggleRect(), editor.showSkillSandboxPreview ? U"Preview ON" : U"Preview OFF", editor.showSkillSandboxPreview, uiFont, RectButtonStyle{ .fontSize = 12 });
		DrawRectButton(SkillEditorCloseRect(), U"×", false, uiFont);
		DrawRectButton(SkillEditorSaveRect(), U"Save TOML", false, uiFont, RectButtonStyle{ .fontSize = 12 });

		unitViewport.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		list.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detail.draw(ColorF{ 0.04, 0.05, 0.065, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
		detailViewport.draw(ColorF{ 0, 0, 0, 0.08 });
		uiFont(U"Units").draw(11, unitViewport.x + 10.0, unitViewport.y - 18.0, Palette::Aqua);
		uiFont(U"Skills").draw(11, list.x + 8.0, list.y - 18.0, Palette::Aqua);

		DrawSkillEditorUnitPanel(editor, catalog, defs, uiFont);
		DrawSkillEditorSkillListPanel(editor, defs, uiFont);
		DrawSkillEditorContextMenu(editor, uiFont);
		DrawSkillEditorUnitContextMenu(editor, defs, uiFont);
		DrawSkillEditorDetailPanel(editor, defs, uiFont, detail, detailViewport);
	}
}
