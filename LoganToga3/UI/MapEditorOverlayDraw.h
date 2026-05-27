#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapLayersDraw.h"
# include "MapEditorPerlinNoise.h"
# include "MapEditorDescriptionEditor.h"
# include "AiEditor.h"

namespace LT3
{
	inline void DrawMapEditorOverlay(MapEditorState& editor, const UnitCatalog& unitCatalog, const DefinitionStores& defs, const Vec2& screenMouse, const Font& uiFont)
	{
		DrawMapEditorToolbar(editor, uiFont);
		if (editor.editorToolbarAllowed)
		{
			DrawUiLayoutGridControl(editor, uiFont);
		}
		if (editor.showResourcePanels)
		{
			DrawMapEditorResourceNodeList(editor, uiFont);
			DrawMapEditorResourceValidation(editor, uiFont);
			DrawMapEditorResourcePalette(editor, uiFont);
		}
		DrawUnitCatalogList(editor, unitCatalog, uiFont);
		DrawCommandEditor(editor, unitCatalog, defs, uiFont);
		DrawSkillEditor(editor, unitCatalog, defs, uiFont);
		DrawAiEditor(editor, defs, uiFont);
		DrawUnitBuildingEditorTabBar(editor, uiFont);
		DrawUnitParameterEditor(editor, unitCatalog, uiFont);
		DrawUniqueEditor(editor, unitCatalog, uiFont);
		DrawBuildingEditor(editor, unitCatalog, defs, uiFont);
		if (editor.showResourcePanels)
		{
			DrawMapEditorResourceNodeEditor(editor, uiFont);
		}
		DrawMapEditorDecalEditor(editor, uiFont);
		DrawMapEditorZOrderOverlay(editor, screenMouse, uiFont);
		DrawDescriptionEditor(editor, uiFont);
		if (!editor.enabled)
		{
			return;
		}

		DrawMapEditorCurrentCellMarker(editor, screenMouse);
		DrawHomeMarker(editor.playerHomePosition, U"P Home", ColorF{ 0.15, 0.8, 1.0, 0.92 }, uiFont);
		DrawHomeMarker(editor.enemyHomePosition, U"E Home", ColorF{ 1.0, 0.36, 0.30, 0.92 }, uiFont);

		const RectF palettePanel = EditorPalettePanelRect();
		const RectF paletteViewport = EditorPaletteViewportRect();
		const RectF resourceToggleRect = EditorResourcePanelsToggleRect();
		palettePanel.draw(ColorF{ 0.02, 0.03, 0.045, 0.88 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		resourceToggleRect.draw(editor.showResourcePanels ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, resourceToggleRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"★").drawAt(36, resourceToggleRect.center(), editor.showResourcePanels ? Palette::Yellow : Palette::Lightgray);
		DrawStarToolMenu(editor, uiFont);
		DrawPerlinNoisePanel(editor, uiFont);
		DrawFogPanel(editor, uiFont);

		uiFont(U"Map Assets").draw(palettePanel.x + 20.0, palettePanel.y + 10.0, Palette::White);

		for (int32 tabIndex = 0; tabIndex < 2; ++tabIndex)
		{
			const RectF tabRect = EditorPaletteTabRect(tabIndex);
			const bool active = editor.paletteTabIndex == tabIndex;
			tabRect.draw(active ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, tabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(MapEditorPaletteTabLabel(tabIndex)).drawAt(12, tabRect.center(), active ? Palette::White : Palette::Lightgray);
		}

		paletteViewport.draw(ColorF{ 0, 0, 0, 0.08 });
		const MapEditorAssetKind activeKind = CurrentMapEditorPaletteKind(editor);
		double y = paletteViewport.y - editor.paletteScroll;
		const double viewportBottom = (paletteViewport.y + paletteViewport.h);
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			const MapEditorAsset& asset = editor.assets[i];
			if (asset.kind != activeKind)
			{
				continue;
			}

			const RectF rect{ paletteViewport.x, y, paletteViewport.w, 46.0 };
			y += 54.0;
			if (!((paletteViewport.y <= rect.y) && ((rect.y + rect.h) <= viewportBottom)))
			{
				continue;
			}

			const bool selected = editor.selectedAsset == i;
			ColorF frameColor = selected ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 };
			if (rect.mouseOver())
			{
				frameColor = ColorF{ 0.0, 0.75, 1.0 };
			}

			rect.draw(selected ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, frameColor);
			DrawAssetPreview(asset, rect.pos + Vec2{ 24, 23 }, SizeF{ 38, 38 });
			uiFont(asset.fileName).draw(13, rect.pos + Vec2{ 54, 6 }, Palette::White);
			uiFont(asset.kind == MapEditorAssetKind::Terrain ? U"terrain" : U"object / obstacle / decal").draw(11, rect.pos + Vec2{ 54, 25 }, Palette::Lightgray);
			if (IsMapEditorDecalAsset(editor, i))
			{
				uiFont(U"decal").draw(10, rect.pos + Vec2{ rect.w - 132.0, 6 }, Palette::Orange);
				const RectF passageCheckRect = EditorPaletteDecalPassageCheckboxRect(rect);
				passageCheckRect.draw(asset.decalBlocksPassage ? ColorF{ 0.18, 0.12, 0.04, 0.96 } : ColorF{ 0.05, 0.06, 0.08, 0.96 })
					.drawFrame(2, passageCheckRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.22 });
				if (asset.decalBlocksPassage)
				{
					Line{ passageCheckRect.tl().movedBy(3.0, 10.0), passageCheckRect.tl().movedBy(7.0, 14.0) }.draw(2.6, Palette::Orange);
					Line{ passageCheckRect.tl().movedBy(7.0, 14.0), passageCheckRect.tl().movedBy(15.0, 4.0) }.draw(2.6, Palette::Orange);
				}
				uiFont(U"block").draw(10, rect.pos + Vec2{ rect.w - 108.0, 26.0 }, asset.decalBlocksPassage ? ColorF{ Palette::Orange } : ColorF{ 0.45, 0.45, 0.45 });
			}
		}

		const double contentHeight = MapEditorPaletteContentHeight(editor);
		if (contentHeight > paletteViewport.h)
		{
			const double scrollRate = editor.paletteScroll / (contentHeight - paletteViewport.h);
			const double handleHeight = Max(32.0, paletteViewport.h * paletteViewport.h / contentHeight);
			const double handleY = paletteViewport.y + (paletteViewport.h - handleHeight) * scrollRate;
			RectF{ paletteViewport.x + paletteViewport.w + 6.0, paletteViewport.y, 6.0, paletteViewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
			RectF{ paletteViewport.x + paletteViewport.w + 6.0, handleY, 6.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
		}

	}
}
