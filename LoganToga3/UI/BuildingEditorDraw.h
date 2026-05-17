#pragma once
# include <Siv3D.hpp>
# include "BuildingEditorCommon.h"

namespace LT3
{
	inline void DrawBuildingEditor(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showBuildingEditor)
		{
			return;
		}

		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		const RectF previewRect = BuildingEditorPreviewRect(editor);

		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

		const bool visualTab = (editor.buildingEditorTab == 0);
		const bool shadowTab = (editor.buildingEditorTab == 1);
		const bool lineHorizontalTab = (editor.buildingEditorTab == 2);
		const bool lineRightTab = (editor.buildingEditorTab == 3);
		const bool lineLeftTab = (editor.buildingEditorTab == 4);

		const RectF visualTabRect = BuildingEditorTabRect(editor, 0);
		const RectF shadowTabRect = BuildingEditorTabRect(editor, 1);
		const RectF lineHorizontalTabRect = BuildingEditorTabRect(editor, 2);
		const RectF lineRightTabRect = BuildingEditorTabRect(editor, 3);
		const RectF lineLeftTabRect = BuildingEditorTabRect(editor, 4);

		visualTabRect.draw(visualTab ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, visualTabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		shadowTabRect.draw(shadowTab ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, shadowTabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		lineHorizontalTabRect.draw(lineHorizontalTab ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, lineHorizontalTabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		lineRightTabRect.draw(lineRightTab ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, lineRightTabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		lineLeftTabRect.draw(lineLeftTab ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, lineLeftTabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });

		uiFont(U"Visual").drawAt(11, visualTabRect.center(), Palette::White);
		uiFont(U"Shadow").drawAt(11, shadowTabRect.center(), Palette::White);
		uiFont(U"H").drawAt(11, lineHorizontalTabRect.center(), Palette::White);
		uiFont(U"R").drawAt(11, lineRightTabRect.center(), Palette::White);
		uiFont(U"L").drawAt(11, lineLeftTabRect.center(), Palette::White);

		previewRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });

		if (!HasSelectedCatalogEntry(editor, catalog))
		{
			uiFont(U"Unit List から対象を選択してください").draw(12, panel.x + 24.0, panel.y + 182.0, Palette::Lightgray);
			return;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];

		Point offset{ 0, 0 };
		String offsetLabel;

		if (visualTab)
		{
			offset = entry.visualOffset;
			offsetLabel = U"Visual Offset X:{}  Y:{}"_fmt(offset.x, offset.y);
		}
		else if (shadowTab)
		{
			offset = entry.shadowOffset;
			offsetLabel = U"Shadow Offset X:{}  Y:{}"_fmt(offset.x, offset.y);
		}
		else if (lineHorizontalTab)
		{
			offset = entry.lineIconHorizontalOffset;
			offsetLabel = U"Line Horizontal Offset X:{}  Y:{}"_fmt(offset.x, offset.y);
		}
		else if (lineRightTab)
		{
			offset = entry.lineIconDiagUpRightOffset;
			offsetLabel = U"Line Right Offset X:{}  Y:{}"_fmt(offset.x, offset.y);
		}
		else if (lineLeftTab)
		{
			offset = entry.lineIconDiagUpLeftOffset;
			offsetLabel = U"Line Left Offset X:{}  Y:{}"_fmt(offset.x, offset.y);
		}

		uiFont(entry.name).draw(14, panel.x + 24.0, panel.y + 178.0, Palette::White);
		uiFont(U"tag:{}  image:{}"_fmt(entry.tag, entry.image)).draw(11, panel.x + 24.0, panel.y + 196.0, Palette::Lightgray);
		uiFont(offsetLabel).draw(13, panel.x + 24.0, panel.y + 212.0, Palette::Gold);

		uiFont(U"Anchor").draw(12, panel.x + 24.0, panel.y + 360.0, Palette::Aqua);
		const bool anchorCenter = (entry.placementAnchor == UnitPlacementAnchor::Center);
		const RectF anchorCenterRect = BuildingEditorAnchorButtonRect(editor, 0);
		const RectF anchorBottomRect = BuildingEditorAnchorButtonRect(editor, 1);
		anchorCenterRect.draw(anchorCenter ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, anchorCenterRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		anchorBottomRect.draw(!anchorCenter ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, anchorBottomRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Center").drawAt(12, anchorCenterRect.center(), Palette::White);
		uiFont(U"Bottom").drawAt(12, anchorBottomRect.center(), Palette::White);

		uiFont(U"Size Mode").draw(12, panel.x + 24.0, panel.y + 404.0, Palette::Aqua);
		const bool gameplayMode = (entry.renderSizeMode == UnitRenderSizeMode::Gameplay);
		const RectF gameplayRect = BuildingEditorRenderSizeModeButtonRect(editor, 0);
		const RectF artRect = BuildingEditorRenderSizeModeButtonRect(editor, 1);
		gameplayRect.draw(gameplayMode ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, gameplayRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		artRect.draw(!gameplayMode ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, artRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Gameplay").drawAt(11, gameplayRect.center(), Palette::White);
		uiFont(U"Art").drawAt(12, artRect.center(), Palette::White);

		const RectF refCellRect = BuildingEditorArtWidthRefButtonRect(editor, 0);
		const RectF refPixelRect = BuildingEditorArtWidthRefButtonRect(editor, 1);
		const bool cellRef = (entry.artWidthReference == UnitArtWidthReference::Cell);
		const double uiAlpha = gameplayMode ? 0.35 : 1.0;
		uiFont(U"Art Ref").draw(12, panel.x + 24.0, panel.y + 448.0, ColorF{ Palette::Aqua, uiAlpha });
		refCellRect.draw(cellRef ? ColorF{ 0.12, 0.22, 0.18, 0.96 * uiAlpha } : ColorF{ 0.08, 0.09, 0.11, 0.92 * uiAlpha }).drawFrame(2, refCellRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0, uiAlpha } : ColorF{ 1, 1, 1, 0.16 * uiAlpha });
		refPixelRect.draw(!cellRef ? ColorF{ 0.12, 0.22, 0.18, 0.96 * uiAlpha } : ColorF{ 0.08, 0.09, 0.11, 0.92 * uiAlpha }).drawFrame(2, refPixelRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0, uiAlpha } : ColorF{ 1, 1, 1, 0.16 * uiAlpha });
		uiFont(U"Cell").drawAt(12, refCellRect.center(), ColorF{ Palette::White, uiAlpha });
		uiFont(U"Pixel").drawAt(12, refPixelRect.center(), ColorF{ Palette::White, uiAlpha });

		const RectF keepAspectRect = BuildingEditorKeepAspectButtonRect(editor);
		const String keepAspectText = entry.artKeepAspect ? U"Aspect ON" : U"Aspect OFF";
		keepAspectRect.draw(entry.artKeepAspect ? ColorF{ 0.12, 0.22, 0.18, 0.96 * uiAlpha } : ColorF{ 0.08, 0.09, 0.11, 0.92 * uiAlpha }).drawFrame(2, keepAspectRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0, uiAlpha } : ColorF{ 1, 1, 1, 0.16 * uiAlpha });
		uiFont(keepAspectText).drawAt(11, keepAspectRect.center(), ColorF{ Palette::White, uiAlpha });

		const bool showGameplayValue = gameplayMode;
		const double artWidthValueForTab = GetSelectedUnitArtWidthValueForTab(entry, editor.buildingEditorTab);
		const String artWidthLabel = lineHorizontalTab
			? U"Art Width(H): {:.2f} {}"_fmt(artWidthValueForTab, cellRef ? U"cell" : U"px")
			: (lineRightTab
				? U"Art Width(R): {:.2f} {}"_fmt(artWidthValueForTab, cellRef ? U"cell" : U"px")
				: (lineLeftTab
					? U"Art Width(L): {:.2f} {}"_fmt(artWidthValueForTab, cellRef ? U"cell" : U"px")
					: U"Art Width: {:.2f} {}"_fmt(artWidthValueForTab, cellRef ? U"cell" : U"px")));
		const String sizeValueText = showGameplayValue
			? U"Gameplay Mul: {:.2f}"_fmt(entry.gameplaySizeMul)
			: artWidthLabel;
		uiFont(sizeValueText).draw(12, panel.x + 142.0, panel.y + 492.0, ColorF{ Palette::Gold, gameplayMode ? 1.0 : uiAlpha });
		if (!showGameplayValue)
		{
			uiFont(sizeValueText).draw(12, panel.x + 142.0, panel.y + 492.0, Palette::Gold);
		}
		DrawBuildingEditorButton(BuildingEditorSizeValueButtonRect(editor, 0), U"-4", uiFont);
		DrawBuildingEditorButton(BuildingEditorSizeValueButtonRect(editor, 1), U"-1", uiFont);
		DrawBuildingEditorButton(BuildingEditorSizeValueButtonRect(editor, 2), U"+1", uiFont);
		DrawBuildingEditorButton(BuildingEditorSizeValueButtonRect(editor, 3), U"+4", uiFont);

		const Optional<const BuildActionDef*> lineAction = FindLineBuildActionForCatalogEntry(entry, defs);
		if (lineAction)
		{
			uiFont(U"Line Icons ({})"_fmt((*lineAction)->id)).draw(12, panel.x + 24.0, panel.y + panel.h - 106.0, Palette::Aqua);
			uiFont(U"横: {}"_fmt(editor.buildingEditorIconHorizontal.isEmpty() ? U"<none>" : editor.buildingEditorIconHorizontal)).draw(10, BuildingEditorLineIconPathRect(editor, 0).pos, Palette::Lightgray);
			uiFont(U"右肩: {}"_fmt(editor.buildingEditorIconDiagUpRight.isEmpty() ? U"<none>" : editor.buildingEditorIconDiagUpRight)).draw(10, BuildingEditorLineIconPathRect(editor, 1).pos, Palette::Lightgray);
			uiFont(U"左肩: {}"_fmt(editor.buildingEditorIconDiagUpLeft.isEmpty() ? U"<none>" : editor.buildingEditorIconDiagUpLeft)).draw(10, BuildingEditorLineIconPathRect(editor, 2).pos, Palette::Lightgray);
			DrawBuildingEditorButton(BuildingEditorLineIconBrowseRect(editor, 0), U"参照", uiFont);
			DrawBuildingEditorButton(BuildingEditorLineIconBrowseRect(editor, 1), U"参照", uiFont);
			DrawBuildingEditorButton(BuildingEditorLineIconBrowseRect(editor, 2), U"参照", uiFont);
		}
		else
		{
			uiFont(U"このユニットにline build actionはありません").draw(11, panel.x + 24.0, panel.y + panel.h - 74.0, Palette::Lightgray);
		}

		const Vec2 anchor{ previewRect.center().x, previewRect.y + previewRect.h * 0.72 };
		Line{ previewRect.x + 14.0, anchor.y, previewRect.x + previewRect.w - 14.0, anchor.y }.draw(1.0, ColorF{ 0.20, 0.65, 1.0, 0.35 });
		Line{ anchor.x, previewRect.y + 12.0, anchor.x, previewRect.y + previewRect.h - 12.0 }.draw(1.0, ColorF{ 0.20, 0.65, 1.0, 0.35 });
		Circle{ anchor, 3.0 }.draw(ColorF{ 1.0, 0.84, 0.0, 0.95 });
		Circle{ anchor.movedBy(static_cast<double>(offset.x), static_cast<double>(offset.y)), 3.5 }.draw(ColorF{ 0.95, 0.90, 0.12, 0.90 });

		// H/R/Lタブのとき対応するline icon画像を使い、それ以外は通常のunit画像を使う
		const auto resolveLineIconPath = [](const String& iconName) -> FilePath
		{
			if (iconName.isEmpty()) return FilePath{};
			const Array<FilePath> candidates = {
				ResolveBuildIconPath(iconName),
				ResolveUnitChipPath(iconName),
				ResolveBuildingChipPath(iconName)
			};
			for (const auto& p : candidates)
			{
				if (FileSystem::Exists(p)) return p;
			}
			return FilePath{};
		};

		FilePath imagePath;
		bool drawAsBuilding = (entry.kind.lowercased() == U"building");
		if (lineHorizontalTab)
		{
			imagePath = resolveLineIconPath(editor.buildingEditorIconHorizontal);
			drawAsBuilding = true;
		}
		else if (lineRightTab)
		{
			imagePath = resolveLineIconPath(editor.buildingEditorIconDiagUpRight);
			drawAsBuilding = true;
		}
		else if (lineLeftTab)
		{
			imagePath = resolveLineIconPath(editor.buildingEditorIconDiagUpLeft);
			drawAsBuilding = true;
		}
		else
		{
			imagePath = ResolveCatalogVisualPath(entry.kind, entry.image);
		}

		if (!imagePath.isEmpty() && FileSystem::Exists(imagePath))
		{
			auto& textureCache = BuildingEditorTextureCache();
			if (!textureCache.contains(imagePath))
			{
				textureCache.emplace(imagePath, Texture{ imagePath });
			}

			const Texture& texture = textureCache.at(imagePath);
			const double fitScale = Min((previewRect.w - 48.0) / Max(1.0, static_cast<double>(texture.width())), (previewRect.h - 28.0) / Max(1.0, static_cast<double>(texture.height())));
			const double drawScale = Min(fitScale, 2.0 * entry.visualScale);
			const Vec2 previewOffset = Vec2{ static_cast<double>(offset.x), static_cast<double>(offset.y) } * drawScale;
			const TextureRegion previewTexture = texture.scaled(drawScale);
			if (drawAsBuilding)
			{
				previewTexture.draw(Arg::bottomCenter = anchor + previewOffset);
			}
			else
			{
				previewTexture.drawAt(anchor + previewOffset);
			}
		}
		else
		{
			RectF{ Arg::center = anchor, 92.0, 72.0 }.draw(ColorF{ 0.18, 0.18, 0.20 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
			uiFont(lineHorizontalTab || lineRightTab || lineLeftTab ? U"icon n/a" : U"image n/a").drawAt(12, anchor, Palette::Lightgray);
		}

		uiFont(U"X").draw(13, panel.x + 24.0, panel.y + 230.0 + 9.0, Palette::White);
		uiFont(U"Y").draw(13, panel.x + 24.0, panel.y + 290.0 + 9.0, Palette::White);
		uiFont(U"step: -4 -1 +1 +4").draw(11, panel.x + 24.0, panel.y + panel.h - 36.0, Palette::Lightgray);

		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 0, 0), U"-4", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 0, 1), U"-1", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 0, 2), U"+1", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 0, 3), U"+4", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 1, 0), U"-4", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 1, 1), U"-1", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 1, 2), U"+1", uiFont);
		DrawBuildingEditorButton(BuildingEditorAxisButtonRect(editor, 1, 3), U"+4", uiFont);
		DrawBuildingEditorButton(BuildingEditorResetRect(editor), U"Reset", uiFont);

		// UI Layout Edit Mode: Building Editor ドラッグハンドル
		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorBuildingEditorDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			dragHandle.draw(editor.uiLayoutDraggingBuildingEditor ? ColorF{ 1.0, 0.84, 0.0, 0.9 } : ColorF{ 1.0, 0.84, 0.0, 0.4 });
			topAnchorToggle.draw(editor.uiBuildingEditorTopAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, topAnchorToggle.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"⋮").drawAt(12, dragHandle.center(), Palette::White);
			uiFont(U"↑").drawAt(11, topAnchorToggle.center(), editor.uiBuildingEditorTopAnchor ? Palette::White : Palette::Lightgray);
		}
	}

}
