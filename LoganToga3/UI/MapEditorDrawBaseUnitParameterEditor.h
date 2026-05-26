#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseUnitEditorHelpers.h"
# include "MapEditorUnitParamEditorCommon.h"

namespace LT3
{
	inline void DrawUnitBuildingEditorTabBar(const MapEditorState& editor, const Font& uiFont)
	{
		const bool show = editor.showUnitParameterEditor || editor.showBuildingEditor || editor.showUniqueEditor;
		if (!show)
		{
			return;
		}

		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		bar.draw(ColorF{ 0.04, 0.05, 0.07, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

		const Array<String> tabLabels = { U"Unit Param", U"Building Edit", U"Unique Edit" };
		for (int32 i = 0; i < 3; ++i)
		{
			const RectF tab = EditorUnitBuildingTabRect(editor, i);
			const bool active = (i == 0) ? editor.showUnitParameterEditor : ((i == 1) ? editor.showBuildingEditor : editor.showUniqueEditor);
			DrawRectTabButton(tab, tabLabels[i], active, uiFont, 11);
		}

		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorUnitParameterDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			DrawUiLayoutEditHandle(dragHandle, topAnchorToggle, editor.uiLayoutDraggingParamEditor, editor.uiParamEditorTopAnchor, uiFont, 11);
		}
		else
		{
			const RectF closeBtn = EditorUnitBuildingCloseRect(editor);
			DrawRectPanelCloseButton(closeBtn, uiFont, 14, ColorF{ 0.12, 0.05, 0.05, 0.95 }, 1.0);
		}
	}

	inline void DrawUnitParameterEditor(const MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
	{
		if (!editor.showUnitParameterEditor || editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		const RectF panel = EditorUnitParameterPanelRect(editor);
		DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.94 }, ColorF{ 1, 1, 1, 0.18 });
		EditorUnitParamHeaderDividerRect(editor).draw(ColorF{ 1, 1, 1, 0.22 });

		const Array<String> tabLabels = { U"Basic", U"Combat", U"MoveVis", U"Audio", U"Economy" };
		for (int32 i = 0; i < static_cast<int32>(tabLabels.size()); ++i)
		{
			const RectF tabRect = EditorUnitParamInnerTabRect(editor, i);
			const bool active = (editor.unitParamEditorTab == i);
			DrawRectTabButton(tabRect, tabLabels[i], active, uiFont, 11);
		}

		uiFont(entry.name).draw(14, panel.x + 18.0, panel.y + 52.0, Palette::White);
		uiFont(U"unit_id:{}  image:{}"_fmt(entry.unit_id, entry.image)).draw(11, panel.x + 18.0, panel.y + 72.0, Palette::Lightgray);
		const RectF voiceButton = EditorUnitParamVoiceButtonRect(editor);
		voiceButton.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, voiceButton.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Voice...").drawAt(11, voiceButton.center(), Palette::White);

		const RectF voiceClear = EditorUnitParamVoiceClearRect(editor);
		DrawRectIconButton(voiceClear, U"clr", uiFont, 9, ColorF{ 0.12, 0.05, 0.05, 0.90 }, 1.0, Palette::White);
		const String voiceLabel = entry.spawnVoice.isEmpty() ? U"voice:(none)" : U"voice:{}"_fmt(entry.spawnVoice);
		uiFont(voiceLabel).draw(10, panel.x + 18.0, panel.y + 94.0, entry.spawnVoice.isEmpty() ? Palette::Gray : Palette::Aqua);

		const RectF listHeader = EditorUnitParamListHeaderRect(editor);
		listHeader.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		uiFont(U"name").draw(12, listHeader.x + 10.0, listHeader.y + 8.0, Palette::White);
		uiFont(U"value").draw(12, listHeader.x + 122.0, listHeader.y + 8.0, Palette::White);

		const RectF viewport = EditorUnitParamListViewportRect(editor);
		viewport.draw(ColorF{ 0, 0, 0, 0.10 });

		const Array<UnitParamRowSpec> rows = UnitParamRowSpecs(editor.unitParamEditorTab);

		String hoverHelp;
		static bool hatenaLoaded = false;
		static Texture hatenaTexture;
		if (!hatenaLoaded)
		{
			hatenaLoaded = true;
			const FilePath hatenaPath = ResolveSystemImagePath(U"hatena.png");
			if (FileSystem::Exists(hatenaPath))
			{
				hatenaTexture = Texture{ hatenaPath };
			}
		}

		for (int32 i = 0; i < static_cast<int32>(rows.size()); ++i)
		{
			const RectF row = EditorUnitParamRowRect(viewport, i);
			const RectF nameRect = EditorUnitParamRowNameRect(row);
			row.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, row.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.10 });
			uiFont(rows[i].label).draw(12, nameRect.x + 6.0, nameRect.y + 7.0, Palette::White);
			if (rows[i].kind == UnitParamRowKind::SpawnVoiceForEnemy)
			{
				DrawRectCheckRow(EditorUnitParamRowValueRect(row), U"", entry.spawnVoiceForEnemy, uiFont, 10, 18.0, 18.0, false);
			}
			const String valueText = (editor.unitParamEditingRow == i) ? editor.unitParamEditingText : UnitParamValueText(entry, rows[i].kind);
			DrawRectNumberStepper(EditorUnitParamRowStepperRects(row), valueText, U"x{}"_fmt(UnitParamStep(editor, editor.unitParamEditorTab, i)), editor.unitParamEditingRow == i, editor.unitParamStepMenuRow && *editor.unitParamStepMenuRow == i, true, uiFont, RectNumberStepperStyle{ .buttonStyle = RectButtonStyle{ .fontSize = 9 }, .valueFontSize = 10 });

			for (int32 buttonIndex = 0; buttonIndex < 3; ++buttonIndex)
			{
				const RectF buttonRect = EditorUnitParamRowButtonRect(row, buttonIndex);
				buttonRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				if (buttonIndex == 0)
				{
					uiFont(U"R").drawAt(16, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 1)
				{
					uiFont(U"U").drawAt(16, buttonRect.center(), rows[i].useIcon ? Palette::White : Palette::Gray);
				}
				else
				{
					if (hatenaTexture)
					{
						hatenaTexture.resized(18, 18).drawAt(buttonRect.center());
					}
					else
					{
						uiFont(U"?").drawAt(16, buttonRect.center(), Palette::White);
					}
				}

				if (buttonIndex == 2 && buttonRect.mouseOver())
				{
					hoverHelp = rows[i].helpText;
				}
			}
		}

		if (editor.unitParamStepMenuRow)
		{
			const Array<double>& steps = UnitParamDefaultSteps();
			const RectF menuRect = EditorUnitParamStepMenuRect(editor.unitParamStepMenuPos, static_cast<int32>(steps.size()));
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
			{
				const RectF item = EditorUnitParamStepMenuItemRect(editor.unitParamStepMenuPos, i);
				item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
				uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 3.0, Palette::White);
			}
		}

		if (!hoverHelp.isEmpty())
		{
			uiFont(hoverHelp).draw(11, panel.x + 18.0, panel.y + panel.h - 16.0, Palette::Aqua);
		}

		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorUnitParameterDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			DrawUiLayoutEditHandle(dragHandle, topAnchorToggle, editor.uiLayoutDraggingParamEditor, editor.uiParamEditorTopAnchor, uiFont, 11);
		}
	}

	inline void DrawUniqueEditor(const MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
	{
		if (!editor.showUniqueEditor || editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		const RectF panel = EditorUniquePanelRect(editor);
		DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.94 }, ColorF{ 1, 1, 1, 0.18 });
		EditorUniqueHeaderDividerRect(editor).draw(ColorF{ 1, 1, 1, 0.22 });

		uiFont(entry.name).draw(14, panel.x + 18.0, panel.y + 20.0, Palette::White);
		uiFont(U"unit_id:{}"_fmt(entry.unit_id)).draw(11, panel.x + 18.0, panel.y + 40.0, Palette::Lightgray);

		const RectF uniqueCheck = EditorUniqueCheckRect(editor);
		DrawRectCheckRow(uniqueCheck, U"Unique Unit", entry.unique, uiFont, 11, 4.0, 10.0, false);

		const RectF portraitButton = EditorUniquePortraitButtonRect(editor);
		portraitButton.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, portraitButton.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Face...").drawAt(11, portraitButton.center(), Palette::White);

		const RectF portraitPreview = EditorUniquePortraitPreviewRect(editor);
		portraitPreview.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
		const FilePath portraitPath = ResolveUnitPortraitPath(entry.portraitImage);
		if (!portraitPath.isEmpty() && FileSystem::Exists(portraitPath))
		{
			auto& textureCache = BuildingEditorTextureCache();
			if (!textureCache.contains(portraitPath))
			{
				textureCache.emplace(portraitPath, Texture{ portraitPath });
			}
			const Texture& texture = textureCache.at(portraitPath);
			const double fitScale = Min((portraitPreview.w - 4.0) / Max(1.0, static_cast<double>(texture.width())), (portraitPreview.h - 4.0) / Max(1.0, static_cast<double>(texture.height())));
			texture.scaled(fitScale).drawAt(portraitPreview.center());
		}
		else
		{
			uiFont(U"顔").drawAt(12, portraitPreview.center(), Palette::Gray);
		}

		const RectF portraitClear = EditorUniquePortraitClearRect(editor);
		DrawRectIconButton(portraitClear, U"clr", uiFont, 9, ColorF{ 0.12, 0.05, 0.05, 0.90 }, 1.0, Palette::White);

		const RectF respawnCheck = EditorUniqueRespawnCheckRect(editor);
		DrawRectCheckRow(respawnCheck, U"Respawn OK", entry.uniqueRespawnAllowed, uiFont, 11, 4.0, 10.0, false);

		const RectF valueViewport = EditorUniqueValueViewportRect(editor);
		valueViewport.draw(ColorF{ 0, 0, 0, 0.10 });
		const Array<std::pair<String, String>> valueRows = {
			{ U"Interval", U"{:.1f}"_fmt(entry.uniqueSpeechIntervalSec) },
			{ U"Visible", U"{:.1f}"_fmt(entry.uniqueSpeechVisibleSec) },
			{ U"Bubble W", U"{:.0f}"_fmt(entry.uniqueSpeechBubbleWidth) },
			{ U"Bubble H", U"{:.0f}"_fmt(entry.uniqueSpeechBubbleHeight) },
		};
		for (int32 i = 0; i < static_cast<int32>(valueRows.size()); ++i)
		{
			const RectF row = EditorUniqueValueRowRect(valueViewport, i);
			const RectF nameRect = EditorUniqueValueRowNameRect(row);
			row.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, row.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.10 });
			uiFont(valueRows[i].first).draw(12, nameRect.x, nameRect.y + 2.0, Palette::White);
			const String valueText = (editor.uniqueEditorValueEditingRow == i) ? editor.uniqueEditorValueEditingText : valueRows[i].second;
			DrawRectNumberStepper(EditorUniqueValueRowStepperRects(row), valueText, U"x1", editor.uniqueEditorValueEditingRow == i, false, true, uiFont, RectNumberStepperStyle{ .buttonStyle = RectButtonStyle{ .fontSize = 9 }, .valueFontSize = 10 });
			const RectF resetButton = EditorUniqueValueRowButtonRect(row);
			DrawRectIconButton(resetButton, U"R", uiFont, 14, ColorF{ 0.08, 0.09, 0.11, 0.92 }, 2.0, Palette::White);
		}

		const RectF speechHeader = EditorUniqueSpeechHeaderRect(editor);
		speechHeader.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		uiFont(U"Speech Lines").draw(12, speechHeader.x + 10.0, speechHeader.y + 7.0, Palette::White);
		const RectF addRect = EditorUniqueSpeechAddRect(editor);
		addRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, addRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"+ Add").drawAt(10, addRect.center(), Palette::White);

		const RectF speechViewport = EditorUniqueSpeechViewportRect(editor);
		speechViewport.draw(ColorF{ 0, 0, 0, 0.10 });
		const double speechViewportBottom = speechViewport.y + speechViewport.h;
		for (int32 i = 0; i < static_cast<int32>(entry.uniqueSpeechLines.size()); ++i)
		{
			const RectF row = EditorUniqueSpeechRowRect(speechViewport, i).movedBy(0.0, -editor.uniqueSpeechScroll);
			if (!((speechViewport.y <= row.y) && ((row.y + row.h) <= speechViewportBottom)))
			{
				continue;
			}
			const RectF textRect = EditorUniqueSpeechTextRect(row);
			row.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, row.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.10 });
			const String speechText = (editor.uniqueSpeechEditingIndex == i) ? (editor.uniqueSpeechEditingText + U"|") : entry.uniqueSpeechLines[i];
			uiFont(speechText.isEmpty() ? U"(empty)" : speechText).draw(12, textRect.x, textRect.y + 5.0, Palette::White);
			const RectF delRect = EditorUniqueSpeechDeleteRect(row);
			DrawRectIconButton(delRect, U"×", uiFont, 13, ColorF{ 0.12, 0.05, 0.05, 0.90 }, 1.0, Palette::White);
		}

		const double speechContentHeight = static_cast<double>(entry.uniqueSpeechLines.size()) * 54.0;
		DrawRectVerticalScrollbar(speechViewport, speechContentHeight, editor.uniqueSpeechScroll, ColorF{ 1, 1, 1, 0.08 }, ColorF{ 1.0, 0.84, 0.0, 0.70 }, 6.0, 6.0, 32.0);

		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorUnitParameterDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			dragHandle.draw(editor.uiLayoutDraggingParamEditor ? ColorF{ 1.0, 0.84, 0.0, 0.9 } : ColorF{ 1.0, 0.84, 0.0, 0.4 })
				.drawFrame(1, ColorF{ 1, 1, 1, 0.2 });
			uiFont(U"↕").drawAt(11, dragHandle.center(), Palette::White);
			topAnchorToggle.draw(editor.uiParamEditorTopAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, topAnchorToggle.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"↑").drawAt(11, topAnchorToggle.center(), editor.uiParamEditorTopAnchor ? Palette::White : Palette::Lightgray);
		}
	}
}
