#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseAssetHelpers.h"
# include "MapEditorResourceDraw.h"
# include "BuildingEditor.h"

namespace LT3
{
	inline void DrawEditorButton(const RectF& rect, const String& text, bool active, const Font& uiFont)
	{
		ColorF backColor{ 0.08, 0.09, 0.11, 0.92 };
		if (active)
		{
			backColor = ColorF{ 0.12, 0.22, 0.18, 0.96 };
		}
		ColorF frameColor{ 1, 1, 1, 0.16 };
		if (rect.mouseOver())
		{
			frameColor = ColorF{ 1.0, 0.84, 0.0 };
		}

		rect.draw(backColor).drawFrame(2, frameColor);
		uiFont(text).drawAt(14, rect.center(), active ? Palette::White : Palette::Lightgray);
	}

	inline void DrawMapEditorToolbar(const MapEditorState& editor, const Font& uiFont)
	{
		if (IsEditorBarPreviewHidden(editor))
		{
			return;
		}

		EditorToolbarRect().draw(ColorF{ 0.025, 0.03, 0.045, 0.93 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
		DrawEditorButton(EditorToolbarButtonRect(editor, 0), U"Map Editor", editor.enabled, uiFont);
		if (editor.enabled)
		{
			DrawEditorButton(EditorToolbarButtonRect(editor, 1), U"Save TOML", false, uiFont);
			DrawEditorButton(EditorToolbarButtonRect(editor, 2), U"W -", false, uiFont);
			DrawEditorButton(EditorToolbarButtonRect(editor, 3), U"W +", false, uiFont);
			DrawEditorButton(EditorToolbarButtonRect(editor, 4), U"H -", false, uiFont);
			DrawEditorButton(EditorToolbarButtonRect(editor, 5), U"H +", false, uiFont);
		}
		DrawEditorButton(EditorToolbarButtonRect(editor, 6), U"Unit List", editor.showUnitList, uiFont);
		DrawEditorButton(EditorToolbarButtonRect(editor, 7), U"Build Edit", editor.showBuildingEditor, uiFont);
		DrawEditorButton(EditorToolbarButtonRect(editor, 8), U"Debug Info", editor.showDebugInfo, uiFont);
		DrawEditorButton(EditorToolbarButtonRect(editor, 9), U"UI Edit", editor.uiLayoutEditEnabled, uiFont);
		DrawEditorButton(EditorToolbarButtonRect(editor, 10), U"Battle Grid", editor.showBattleGrid, uiFont);
		DrawEditorButton(EditorToolbarPreviewHideButtonRect(), U"Hide Bar 3s", false, uiFont);

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

		const bool panelHovered = EditorPalettePanelRect().mouseOver()
			|| EditorResourcePanelsToggleRect().mouseOver()
			|| EditorStarToolMenuRect().mouseOver()
			|| EditorPerlinNoisePanelRect().mouseOver()
			|| EditorFogPanelRect().mouseOver()
			|| EditorUnitListPanelRect().mouseOver()
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
		if (panelHovered)
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

		const RectF panel = EditorZOrderPanelRect(maxStackSize);
		const RectF downRect = EditorZOrderDownRect(panel);
		const RectF upRect = EditorZOrderUpRect(panel);
		const RectF sendToBackRect = EditorZOrderSendToBackRect(panel);
		const RectF bringToFrontRect = EditorZOrderBringToFrontRect(panel);
		const RectF closeRect = EditorZOrderCloseRect(panel);

		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		uiFont(U"Decal Z-Order").draw(panel.x + 18.0, panel.y + 14.0, Palette::White);
		uiFont(U"click row to select  |  ⇤ Back … Front ⇥  to reorder").draw(10, panel.x + 18.0, panel.y + 34.0, ColorF{ 0.65, 0.65, 0.65 });
		closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);

		// レイヤーリスト（index 0 = back, index N-1 = front）
		for (int32 i = 0; i < visibleRows; ++i)
		{
			const RectF rowRect = EditorZOrderLayerRowRect(panel, i);
			const bool isSelected = (i == selectedIndex);

			// 行背景
			ColorF rowBack = isSelected ? ColorF{ 0.18, 0.22, 0.14, 0.96 } : ColorF{ 0.06, 0.07, 0.09, 0.90 };
			ColorF rowFrame = isSelected ? ColorF{ 1.0, 0.84, 0.0 } : (rowRect.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.14 });
			rowRect.draw(rowBack).drawFrame(1.5, rowFrame);

			// レイヤー番号バッジ（1=back … N=front）
			const bool isFront = (i == visibleRows - 1);
			const bool isBack  = (i == 0);
			const String layerLabel = isFront ? U"FRONT" : (isBack ? U"BACK " : U"  {}  "_fmt(i + 1));
			const ColorF labelColor = isFront ? ColorF{ 1.0, 0.55, 0.2 } : (isBack ? ColorF{ 0.4, 0.7, 1.0 } : ColorF{ 0.8, 0.8, 0.8 });
			uiFont(layerLabel).draw(10, rowRect.x + 6.0, rowRect.y + 4.0, labelColor);

			// サムネイル + ファイル名表示
			// 集計結果から最多セル数のアセットを代表として表示
			const double thumbSize = EditorZOrderLayerRowHeight - 10.0;
			const double thumbX = rowRect.x + 48.0;
			const double thumbY = rowRect.y + (rowRect.h - thumbSize) * 0.5;

			if (layerInfo[i].empty())
			{
				uiFont(U"(empty)").draw(12, thumbX + thumbSize + 8.0, rowRect.y + 8.0, ColorF{ 0.45, 0.45, 0.45 });
			}
			else
			{
				// 最多セルのアセットを代表表示
				int32 repAsset = InvalidMapEditorAsset;
				int32 repCount = 0;
				for (const auto& [ai, cnt] : layerInfo[i])
				{
					if (cnt > repCount)
					{
						repCount = cnt;
						repAsset = ai;
					}
				}

				if (repAsset >= 0 && repAsset < static_cast<int32>(editor.assets.size()))
				{
					const MapEditorAsset& asset = editor.assets[repAsset];
					DrawAssetPreview(asset, Vec2{ thumbX + thumbSize * 0.5, thumbY + thumbSize * 0.5 }, SizeF{ thumbSize, thumbSize });

					const String nameText = asset.fileName;
					const String countText = (layerInfo[i].size() > 1)
						? U"{} (+{} types)"_fmt(nameText, layerInfo[i].size() - 1)
						: nameText;
					uiFont(countText).draw(11, thumbX + thumbSize + 8.0, rowRect.y + 4.0, Palette::White);

					// セル数
					const int32 totalCells = [&] {
						int32 s = 0;
						for (const auto& [ai, cnt] : layerInfo[i]) { s += cnt; }
						return s;
					}();
					uiFont(U"{} cell{}"_fmt(totalCells, totalCells > 1 ? U"s" : U"")).draw(10, thumbX + thumbSize + 8.0, rowRect.y + 20.0, ColorF{ 0.65, 0.65, 0.65 });
				}
			}
		}

		// フッター: ⇤ SendToBack / ← Back / Front → / BringToFront ⇥
		const bool atBack  = (selectedIndex == 0);
		const bool atFront = (selectedIndex == visibleRows - 1);

		auto drawFooterBtn = [&](const RectF& r, const String& label, bool disabled)
		{
			const ColorF bg    = disabled ? ColorF{ 0.06, 0.06, 0.07, 0.70 } : ColorF{ 0.08, 0.09, 0.11, 0.92 };
			const ColorF frame = disabled ? ColorF{ 1, 1, 1, 0.08 } : (r.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			const ColorF text  = disabled ? ColorF{ 0.40, 0.40, 0.40 } : ColorF{ Palette::White };
			r.draw(bg).drawFrame(2, frame);
			uiFont(label).drawAt(11, r.center(), text);
		};

		drawFooterBtn(sendToBackRect,   U"⇤ BACK端",  atBack);
		drawFooterBtn(downRect,         U"← Back",    atBack);
		drawFooterBtn(upRect,           U"Front →",   atFront);
		drawFooterBtn(bringToFrontRect, U"FRONT端 ⇥", atFront);

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

		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		decRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, decRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		incRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, incRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		valueRect.draw(ColorF{ 0.06, 0.08, 0.10, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });

		uiFont(U"-").drawAt(20, decRect.center(), Palette::White);
		uiFont(U"+").drawAt(20, incRect.center(), Palette::White);
		uiFont(U"Grid {} px"_fmt(editor.uiLayoutGridSize)).drawAt(14, valueRect.center(), Palette::White);
	}
}
