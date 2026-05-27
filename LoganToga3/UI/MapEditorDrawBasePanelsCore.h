#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseAssetHelpers.h"
# include "MapEditorResourceDraw.h"
# include "BuildingEditor.h"
# include "MapEditorToolbarModel.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline void DrawEditorButton(const RectF& rect, const String& text, bool active, const Font& uiFont)
	{
		DrawRectButton(rect, text, active, uiFont);
	}

	inline void DrawEditorPanelFrame(const RectF& rect, const ColorF& backColor, const ColorF& frameColor, double frameThickness = 1.0)
	{
		rect.draw(backColor).drawFrame(frameThickness, frameColor);
	}

	inline void DrawEditorIconButton(const RectF& rect, const String& label, const Font& uiFont, int32 fontSize, const ColorF& backColor)
	{
		DrawEditorPanelFrame(rect, backColor, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(label).drawAt(fontSize, rect.center(), Palette::White);
	}

	inline void DrawEditorDisabledButton(const RectF& rect, const String& label, bool disabled, const Font& uiFont, int32 fontSize)
	{
		const ColorF backColor = disabled ? ColorF{ 0.06, 0.06, 0.07, 0.70 } : ColorF{ 0.08, 0.09, 0.11, 0.92 };
		const ColorF frameColor = disabled ? ColorF{ 1, 1, 1, 0.08 } : (rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		const ColorF textColor = disabled ? ColorF{ 0.40, 0.40, 0.40 } : ColorF{ Palette::White };
		rect.draw(backColor).drawFrame(2, frameColor);
		uiFont(label).drawAt(fontSize, rect.center(), textColor);
	}

	inline bool IsMapEditorUiPanelHovered(const MapEditorState& editor)
	{
		return EditorPalettePanelRect().mouseOver()
			|| EditorResourcePanelsToggleRect().mouseOver()
			|| EditorStarToolMenuRect().mouseOver()
			|| EditorPerlinNoisePanelRect().mouseOver()
			|| EditorFogPanelRect().mouseOver()
			|| EditorUnitListPanelRect().mouseOver()
			|| EditorCommandPanelRect().mouseOver()
			|| EditorUnitParameterPanelRect(editor).mouseOver()
			|| BuildingEditorPanelWithPosRect(editor).mouseOver()
			|| EditorResourceNodeListPanelRect().mouseOver()
			|| EditorResourceNodeFilterRect(0).mouseOver()
			|| EditorResourceNodeFilterRect(1).mouseOver()
			|| EditorResourceNodeFilterRect(2).mouseOver()
			|| EditorResourceNodeFilterRect(3).mouseOver()
			|| EditorResourceValidationPanelRect().mouseOver()
			|| EditorResourcePalettePanelRect().mouseOver()
			|| (IsValidSelectedResourceNodeIndex(editor) && EditorResourceNodePanelRect().mouseOver())
			|| (HasOpenDecalEditorTarget(editor) && EditorDecalEditorPanelRect().mouseOver())
			|| (editor.zOrderMode && EditorZOrderPanelRectFallback().mouseOver());
	}

	inline void DrawZOrderLayerRowFrame(const RectF& rowRect, bool selected)
	{
		const ColorF rowBack = selected ? ColorF{ 0.18, 0.22, 0.14, 0.96 } : ColorF{ 0.06, 0.07, 0.09, 0.90 };
		const ColorF rowFrame = selected ? ColorF{ 1.0, 0.84, 0.0 } : (rowRect.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.14 });
		rowRect.draw(rowBack).drawFrame(1.5, rowFrame);
	}

	inline String ZOrderLayerLabel(int32 index, int32 visibleRows)
	{
		if (index == visibleRows - 1)
		{
			return U"FRONT";
		}

		if (index == 0)
		{
			return U"BACK ";
		}

		return U"  {}  "_fmt(index + 1);
	}

	inline ColorF ZOrderLayerLabelColor(int32 index, int32 visibleRows)
	{
		if (index == visibleRows - 1)
		{
			return ColorF{ 1.0, 0.55, 0.2 };
		}

		if (index == 0)
		{
			return ColorF{ 0.4, 0.7, 1.0 };
		}

		return ColorF{ 0.8, 0.8, 0.8 };
	}

	inline int32 FindDominantZOrderAsset(const HashTable<int32, int32>& assetCounts)
	{
		int32 repAsset = InvalidMapEditorAsset;
		int32 repCount = 0;
		for (const auto& [assetIndex, count] : assetCounts)
		{
			if (count > repCount)
			{
				repCount = count;
				repAsset = assetIndex;
			}
		}

		return repAsset;
	}

	inline int32 CountZOrderLayerCells(const HashTable<int32, int32>& assetCounts)
	{
		int32 total = 0;
		for (const auto& [assetIndex, count] : assetCounts)
		{
			total += count;
		}
		return total;
	}

	inline void DrawZOrderLayerRow(const MapEditorState& editor, const RectF& rowRect, int32 index, int32 visibleRows, const HashTable<int32, int32>& assetCounts, bool selected, const Font& uiFont)
	{
		DrawZOrderLayerRowFrame(rowRect, selected);
		uiFont(ZOrderLayerLabel(index, visibleRows)).draw(10, rowRect.x + 6.0, rowRect.y + 4.0, ZOrderLayerLabelColor(index, visibleRows));

		const double thumbSize = EditorZOrderLayerRowHeight - 10.0;
		const double thumbX = rowRect.x + 48.0;
		const double thumbY = rowRect.y + (rowRect.h - thumbSize) * 0.5;

		if (assetCounts.empty())
		{
			uiFont(U"(empty)").draw(12, thumbX + thumbSize + 8.0, rowRect.y + 8.0, ColorF{ 0.45, 0.45, 0.45 });
			return;
		}

		const int32 repAsset = FindDominantZOrderAsset(assetCounts);
		if (repAsset < 0 || repAsset >= static_cast<int32>(editor.assets.size()))
		{
			return;
		}

		const MapEditorAsset& asset = editor.assets[repAsset];
		DrawAssetPreview(asset, Vec2{ thumbX + thumbSize * 0.5, thumbY + thumbSize * 0.5 }, SizeF{ thumbSize, thumbSize });

		const String nameText = asset.fileName;
		const String countText = (assetCounts.size() > 1)
			? U"{} (+{} types)"_fmt(nameText, assetCounts.size() - 1)
			: nameText;
		uiFont(countText).draw(11, thumbX + thumbSize + 8.0, rowRect.y + 4.0, Palette::White);

		const int32 totalCells = CountZOrderLayerCells(assetCounts);
		uiFont(U"{} cell{}"_fmt(totalCells, totalCells > 1 ? U"s" : U"")).draw(10, thumbX + thumbSize + 8.0, rowRect.y + 20.0, ColorF{ 0.65, 0.65, 0.65 });
	}

	inline void DrawMapEditorToolbar(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.editorToolbarAllowed)
		{
			return;
		}

		if (IsEditorBarPreviewHidden(editor))
		{
			return;
		}

		EditorToolbarRect().draw(ColorF{ 0.025, 0.03, 0.045, 0.93 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
		for (const auto& spec : MapEditorToolbarButtonSpecs())
		{
			if (!IsMapEditorToolbarButtonVisible(editor, spec))
			{
				continue;
			}

			DrawEditorButton(
				MapEditorToolbarButtonRect(editor, spec),
				String{ spec.label },
				IsMapEditorToolbarButtonActive(editor, spec),
				uiFont);
		}

		const MapEditorToolbarButtonSpec previewHideSpec = MapEditorToolbarPreviewHideButtonSpec();
		DrawEditorButton(MapEditorToolbarButtonRect(editor, previewHideSpec), String{ previewHideSpec.label }, false, uiFont);

		if (editor.showDebugInfo)
		{
			const RectF statusBar = EditorStatusBarRect();
			statusBar.draw(ColorF{ 0.025, 0.03, 0.045, 0.93 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
			uiFont(editor.statusText).draw(statusBar.x + 24.0, statusBar.y + 7.0, Palette::Lightgray);
		}
	}

	inline void DrawMapEditorCurrentCellMarker(const MapEditorState& editor, const Vec2& screenMouse)
	{
		if (!editor.enabled)
		{
			return;
		}

		if (IsMapEditorUiPanelHovered(editor))
		{
			return;
		}

		const Optional<Point> cell = PickMapEditorCell(editor, screenMouse);
		if (!cell)
		{
			return;
		}

		const Vec2 center = ToQuarterViewportScreen(MapEditorCellCenter(cell->x, cell->y));
		Circle{ center.movedBy(0, -20), 10.0 }.draw(ColorF{ 1.0, 0.2, 0.2, 0.92 }).drawFrame(2.0, ColorF{ 1.0, 0.95, 0.95, 0.95 });
		Circle{ center.movedBy(0, -20), 22.0 }.drawFrame(2.0, ColorF{ 1.0, 0.2, 0.2, 0.45 });
	}

	inline void DrawMapEditorZOrderOverlay(const MapEditorState& editor, const Vec2& screenMouse, const Font& uiFont)
	{
		if (!editor.enabled || !editor.zOrderMode)
		{
			return;
		}

		Optional<Rect> drawRect = editor.zOrderSelectionRect;
		if (!drawRect && editor.zOrderDragStartCell)
		{
			if (const Optional<Point> hoverCell = PickMapEditorCell(editor, screenMouse))
			{
				drawRect = NormalizeMapEditorCellRect(*editor.zOrderDragStartCell, *hoverCell);
			}
		}

		// タイルハイライトはカメラ変換スコープ内で描画する（座標ズレ修正）
		if (drawRect)
		{
			const auto cameraTransform = CreateQuarterViewTransformer();
			for (int32 y = drawRect->y; y < drawRect->y + drawRect->h; ++y)
			{
				for (int32 x = drawRect->x; x < drawRect->x + drawRect->w; ++x)
				{
					if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
					{
						continue;
					}
					ToQuarterTile(MapEditorCellCenter(x, y)).drawFrame(2.5, ColorF{ 1.0, 0.84, 0.0, 0.85 });
				}
			}
		}

		if (!editor.zOrderSelectionRect)
		{
			return;
		}

		const int32 maxStackSize = MaxDecalStackSizeInRect(editor, *editor.zOrderSelectionRect);
		const int32 selectedIndex = Clamp(editor.zOrderSelectedStackIndex, 0, Max(0, maxStackSize - 1));
		const int32 visibleRows = Clamp(maxStackSize, 1, EditorZOrderMaxVisibleLayers);

		// レイヤーごとに「どのファイルが何セルにあるか」を集計する
		// layer_info[layerIndex] = { assetIndex -> cellCount }
		Array<HashTable<int32, int32>> layerInfo(visibleRows);
		{
			const Rect& r = *editor.zOrderSelectionRect;
			for (int32 cy = r.y; cy < r.y + r.h; ++cy)
			{
				for (int32 cx = r.x; cx < r.x + r.w; ++cx)
				{
					if ((cx < 0) || (cy < 0) || (cx >= editor.mapWidth) || (cy >= editor.mapHeight))
					{
						continue;
					}
					const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, cx, cy)];
					for (int32 li = 0; li < visibleRows && li < static_cast<int32>(cell.decals.size()); ++li)
					{
						++layerInfo[li][cell.decals[li].assetIndex];
					}
				}
			}
		}

		const RectF panel = EditorZOrderPanelRect(editor, maxStackSize);
		const RectF downRect = EditorZOrderDownRect(panel);
		const RectF upRect = EditorZOrderUpRect(panel);
		const RectF sendToBackRect = EditorZOrderSendToBackRect(panel);
		const RectF bringToFrontRect = EditorZOrderBringToFrontRect(panel);
		const RectF closeRect = EditorZOrderCloseRect(panel);

		DrawEditorPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.94 }, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Decal Z-Order").draw(panel.x + 18.0, panel.y + 14.0, Palette::White);
		uiFont(U"click row to select  |  ⇤ Back … Front ⇥  to reorder").draw(10, panel.x + 18.0, panel.y + 34.0, ColorF{ 0.65, 0.65, 0.65 });
		DrawEditorIconButton(closeRect, U"×", uiFont, 18, ColorF{ 0.12, 0.05, 0.05, 0.95 });
		if (editor.uiLayoutEditEnabled)
		{
			DrawUiLayoutDragHandleOnly(EditorZOrderDragHandleRect(editor, maxStackSize), editor.uiLayoutDraggingZOrderPanel, uiFont, 11);
		}

		// レイヤーリスト（index 0 = back, index N-1 = front）
		for (int32 i = 0; i < visibleRows; ++i)
		{
			const RectF rowRect = EditorZOrderLayerRowRect(panel, i);
			DrawZOrderLayerRow(editor, rowRect, i, visibleRows, layerInfo[i], (i == selectedIndex), uiFont);
		}

		// フッター: ⇤ SendToBack / ← Back / Front → / BringToFront ⇥
		const bool atBack  = (selectedIndex == 0);
		const bool atFront = (selectedIndex == visibleRows - 1);

		DrawEditorDisabledButton(sendToBackRect,   U"⇤ BACK端",  atBack, uiFont, 11);
		DrawEditorDisabledButton(downRect,         U"← Back",    atBack, uiFont, 11);
		DrawEditorDisabledButton(upRect,           U"Front →",   atFront, uiFont, 11);
		DrawEditorDisabledButton(bringToFrontRect, U"FRONT端 ⇥", atFront, uiFont, 11);

		const Rect& r = *editor.zOrderSelectionRect;
		uiFont(U"cells ({},{})–({},{})"_fmt(r.x, r.y, r.x + r.w - 1, r.y + r.h - 1)).drawAt(10, Vec2{ panel.x + panel.w * 0.5, sendToBackRect.center().y + 22.0 }, Palette::Lightgray);
	}

	inline void DrawUiLayoutGridControl(const MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.uiLayoutEditEnabled)
		{
			return;
		}

		const RectF panel = EditorUiLayoutGridPanelRect();
		const RectF decRect = EditorUiLayoutGridDecrementRect();
		const RectF incRect = EditorUiLayoutGridIncrementRect();
		const RectF valueRect = EditorUiLayoutGridValueRect();

		DrawEditorPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.92 }, ColorF{ 1, 1, 1, 0.16 });
		DrawEditorIconButton(decRect, U"-", uiFont, 20, ColorF{ 0.08, 0.09, 0.11, 0.92 });
		DrawEditorIconButton(incRect, U"+", uiFont, 20, ColorF{ 0.08, 0.09, 0.11, 0.92 });
		DrawEditorPanelFrame(valueRect, ColorF{ 0.06, 0.08, 0.10, 0.96 }, ColorF{ 1, 1, 1, 0.12 });
		uiFont(U"Grid {} px"_fmt(editor.uiLayoutGridSize)).drawAt(14, valueRect.center(), Palette::White);
	}
}
