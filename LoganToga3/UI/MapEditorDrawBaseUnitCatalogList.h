#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseUnitEditorHelpers.h"

namespace LT3
{
	inline void DrawUnitCatalogList(MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
	{
		if (!editor.showUnitList)
		{
			return;
		}

		const RectF panel = EditorUnitListPanelRect();
		const RectF viewport = EditorUnitListViewportRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Unit List").draw(44, 86, Palette::White);
		uiFont(U"Click row: parameter / BuildingEditor").draw(160, 86, Palette::Lightgray);
		uiFont(U"{} entries"_fmt(catalog.entries.size())).draw(560, 86, Palette::Gold);
		uiFont(catalog.statusText).draw(11, 44, 106, Palette::Lightgray);

		const RectF normalizeIdsRect = EditorUnitNormalizeIdsRect();
		normalizeIdsRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, normalizeIdsRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"IDを一括変更").drawAt(13, normalizeIdsRect.center(), Palette::White);
		const RectF storeIdToTagRect = EditorUnitStoreIdToTagRect();
		storeIdToTagRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, storeIdToTagRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"unit_idにIDを格納").drawAt(13, storeIdToTagRect.center(), Palette::White);

		const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
		if (panel.mouseOver())
		{
			editor.unitListScroll = Clamp(editor.unitListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
		}

		viewport.draw(ColorF{ 0, 0, 0, 0.10 });
		const double viewportBottom = viewport.y + viewport.h;
		for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
		{
			const auto& entry = catalog.entries[i];
			const RectF row = EditorUnitListRowRect(viewport, i, editor.unitListScroll);
			if (!((viewport.y <= row.y) && ((row.y + row.h) <= viewportBottom)))
			{
				continue;
			}

			const bool isBuilding = entry.kind == U"building";
			const bool selected = editor.showUnitParameterEditor && editor.selectedUnitCatalogIndex == i;
			ColorF frameColor = selected ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 };
			if (row.mouseOver())
			{
				frameColor = ColorF{ 0.0, 0.75, 1.0 };
			}
			row.draw(selected ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : (isBuilding ? ColorF{ 0.14, 0.10, 0.08, 0.92 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })).drawFrame(1, frameColor);
			const RectF previewRect = EditorUnitListPreviewRect(row);
			previewRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });

			const FilePath imagePath = ResolveCatalogVisualPath(entry.kind, entry.image);
			if (!imagePath.isEmpty() && FileSystem::Exists(imagePath))
			{
				auto& textureCache = BuildingEditorTextureCache();
				const bool isGif = (FileSystem::Extension(imagePath).lowercased() == U"gif");
				const FilePath textureCacheKey = isGif ? (U"__unit_list_gif_preview__:" + imagePath) : imagePath;
				if (!textureCache.contains(textureCacheKey))
				{
					if (isGif)
					{
						AnimatedGIFReader reader{ imagePath };
						Array<Image> frames;
						Array<int32> frameDelaysMillisec;
						int32 durationMillisec = 0;
						if (reader && reader.read(frames, frameDelaysMillisec, durationMillisec) && !frames.isEmpty())
						{
							textureCache.emplace(textureCacheKey, Texture{ frames.front() });
						}
						else
						{
							textureCache.emplace(textureCacheKey, Texture{ imagePath });
						}
					}
					else
					{
						textureCache.emplace(textureCacheKey, Texture{ imagePath });
					}
				}

				const Texture& texture = textureCache.at(textureCacheKey);
				const double fitScale = Min((previewRect.w - 6.0) / Max(1.0, static_cast<double>(texture.width())), (previewRect.h - 6.0) / Max(1.0, static_cast<double>(texture.height())));
				const TextureRegion previewTexture = texture.scaled(Min(fitScale, 1.0));
				if (isBuilding)
				{
					previewTexture.draw(Arg::bottomCenter = Vec2{ previewRect.center().x, previewRect.y + previewRect.h - 3.0 });
				}
				else
				{
					previewTexture.drawAt(previewRect.center());
				}
			}
			else
			{
				previewRect.draw(isBuilding ? ColorF{ 0.72, 0.45, 0.18 } : ColorF{ 0.18, 0.42, 0.72 });
			}
			uiFont(U"[{}] {}  scale:{:.2f}"_fmt(entry.id, entry.name, entry.visualScale)).draw(15, row.x + 70, row.y + 8, Palette::White);
			uiFont(U"{}  {}  image:{}"_fmt(entry.kind, entry.unit_id, entry.image)).draw(11, row.x + 70, row.y + 31, Palette::Lightgray);
			uiFont(U"HP:{} / BHP:{}  ATK:{}  DEF:{}  SPD:{}  MOVE:{}  RANGE:{}"_fmt(entry.hp, entry.buildingHp, entry.attack, entry.defense, entry.speed, entry.move, entry.maintainRange)).draw(11, row.x + 70, row.y + 51, Palette::Gold);

			const RectF moveUpRect = EditorUnitRowMoveUpRect(row);
			const RectF moveDownRect = EditorUnitRowMoveDownRect(row);
			const bool canMoveUp = (i > 0);
			const bool canMoveDown = (i + 1 < static_cast<int32>(catalog.entries.size()));
			moveUpRect.draw(canMoveUp ? ColorF{ 0.08, 0.09, 0.11, 0.92 } : ColorF{ 0.05, 0.05, 0.06, 0.70 })
				.drawFrame(2, canMoveUp && moveUpRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, canMoveUp ? 0.16 : 0.08 });
			moveDownRect.draw(canMoveDown ? ColorF{ 0.08, 0.09, 0.11, 0.92 } : ColorF{ 0.05, 0.05, 0.06, 0.70 })
				.drawFrame(2, canMoveDown && moveDownRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, canMoveDown ? 0.16 : 0.08 });
			uiFont(U"↑").drawAt(14, moveUpRect.center(), canMoveUp ? Palette::White : Palette::Gray);
			uiFont(U"↓").drawAt(14, moveDownRect.center(), canMoveDown ? Palette::White : Palette::Gray);

			if (editor.unitRenameTargetIndex == i)
			{
				const RectF renameRect = EditorUnitRenameOverlayRect(row);
				renameRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.98 }).drawFrame(2, ColorF{ 1.0, 0.84, 0.0 });
				uiFont(editor.unitRenameEditText + U"|").draw(14, renameRect.x + 8.0, renameRect.y + 6.0, Palette::White);
				uiFont(U"Enter:確定  Esc:キャンセル").draw(10, renameRect.x + renameRect.w + 6.0, renameRect.y + 9.0, ColorF{ 1, 1, 1, 0.55 });
			}
		}

		if (maxScroll > 0.0)
		{
			const double scrollRate = editor.unitListScroll / maxScroll;
			const double handleHeight = Max(32.0, viewport.h * viewport.h / EditorUnitListContentHeight(catalog));
			const double handleY = viewport.y + (viewport.h - handleHeight) * scrollRate;
			RectF{ viewport.x + viewport.w + 6.0, viewport.y, 6.0, viewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
			RectF{ viewport.x + viewport.w + 6.0, handleY, 6.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
		}

		if (editor.unitContextMenuTargetIndex)
		{
			const RectF menuRect = EditorUnitContextMenuRect(editor.unitContextMenuPos);
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			const RectF dupItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 0);
			dupItem.draw(dupItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"Duplicate").draw(13, dupItem.x + 8.0, dupItem.y + 5.0, Palette::White);
			const RectF renameItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 1);
			renameItem.draw(renameItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"名前編集").draw(13, renameItem.x + 8.0, renameItem.y + 5.0, Palette::White);
		}
	}
}
