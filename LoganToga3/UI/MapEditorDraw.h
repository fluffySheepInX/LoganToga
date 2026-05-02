#pragma once
# include <Siv3D.hpp>
# include "MapEditorInput.h"

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
        EditorToolbarRect().draw(ColorF{ 0.025, 0.03, 0.045, 0.93 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
        DrawEditorButton(EditorToolbarButtonRect(editor, 0), editor.enabled ? U"Map Editor: ON" : U"Map Editor: OFF", editor.enabled, uiFont);
        if (editor.enabled)
        {
            DrawEditorButton(EditorToolbarButtonRect(editor, 1), U"Save TOML", false, uiFont);
            DrawEditorButton(EditorToolbarButtonRect(editor, 2), U"W -", false, uiFont);
            DrawEditorButton(EditorToolbarButtonRect(editor, 3), U"W +", false, uiFont);
            DrawEditorButton(EditorToolbarButtonRect(editor, 4), U"H -", false, uiFont);
            DrawEditorButton(EditorToolbarButtonRect(editor, 5), U"H +", false, uiFont);
        }
        DrawEditorButton(EditorToolbarButtonRect(editor, 6), U"Unit List", editor.showUnitList, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(editor, 7), U"Debug Info", editor.showDebugInfo, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(editor, 8), U"UI Edit", editor.uiLayoutEditEnabled, uiFont);

        const double mapTextX = editor.enabled ? 1040.0 : 500.0;
        const double statusTextX = editor.enabled ? 1180.0 : 640.0;
        uiFont(U"Map: {} x {}"_fmt(editor.mapWidth, editor.mapHeight)).draw(mapTextX, 23, Palette::White);
        uiFont(editor.statusText).draw(statusTextX, 23, Palette::Lightgray);
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
        uiFont(U"Click row: parameter editor").draw(160, 86, Palette::Lightgray);
        uiFont(U"{} entries"_fmt(catalog.entries.size())).draw(560, 86, Palette::Gold);
        uiFont(catalog.statusText).draw(11, 44, 106, Palette::Lightgray);

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
            RectF{ row.x + 10, row.y + 11, 48, 48 }.draw(isBuilding ? ColorF{ 0.72, 0.45, 0.18 } : ColorF{ 0.18, 0.42, 0.72 });
            uiFont(U"{}  scale:{:.2f}"_fmt(entry.name, entry.visualScale)).draw(15, row.x + 70, row.y + 8, Palette::White);
            uiFont(U"{}  {}  image:{}"_fmt(entry.kind, entry.tag, entry.image)).draw(11, row.x + 70, row.y + 31, Palette::Lightgray);
            uiFont(U"HP:{} / BHP:{}  ATK:{}  DEF:{}  SPD:{}  COST:{}  skills:{}"_fmt(entry.hp, entry.buildingHp, entry.attack, entry.defense, entry.speed, entry.cost, entry.skills.join(U","))).draw(11, row.x + 70, row.y + 51, Palette::Gold);
        }

        if (maxScroll > 0.0)
        {
            const double scrollRate = editor.unitListScroll / maxScroll;
            const double handleHeight = Max(32.0, viewport.h * viewport.h / EditorUnitListContentHeight(catalog));
            const double handleY = viewport.y + (viewport.h - handleHeight) * scrollRate;
            RectF{ viewport.x + viewport.w + 6.0, viewport.y, 6.0, viewport.h }.draw(ColorF{ 1, 1, 1, 0.08 });
            RectF{ viewport.x + viewport.w + 6.0, handleY, 6.0, handleHeight }.draw(ColorF{ 1.0, 0.84, 0.0, 0.70 });
        }
    }

    inline void DrawUnitParameterEditor(const MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
    {
        if (!editor.showUnitParameterEditor || editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
        {
            return;
        }

        const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        const RectF panel = EditorUnitParameterPanelRect();
        const RectF decRect = EditorUnitScaleDecrementRect();
        const RectF incRect = EditorUnitScaleIncrementRect();
        const RectF resetRect = EditorUnitScaleResetRect();
        const RectF closeRect = EditorUnitParameterCloseRect();

        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
        uiFont(U"Unit Parameter Editor").draw(720, 88, Palette::White);
        closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);

        uiFont(entry.name).draw(14, panel.x + 24, panel.y + 52, Palette::White);
        uiFont(U"tag:{}  image:{}"_fmt(entry.tag, entry.image)).draw(12, panel.x + 24, panel.y + 78, Palette::Lightgray);
        uiFont(U"Visual Scale").draw(13, panel.x + 24, panel.y + 108, Palette::Gold);

        decRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, decRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        incRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, incRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        resetRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, resetRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"-").drawAt(24, decRect.center(), Palette::White);
        uiFont(U"+").drawAt(24, incRect.center(), Palette::White);
        uiFont(U"{:.2f}"_fmt(entry.visualScale)).drawAt(26, Vec2{ panel.x + panel.w * 0.5, decRect.center().y }, Palette::White);
        uiFont(U"Reset").drawAt(13, resetRect.center(), Palette::White);
    }

    inline void DrawAssetPreview(const MapEditorAsset& asset, const Vec2& center, const SizeF& size)
    {
        if (asset.texture)
        {
            asset.texture.resized(size).drawAt(center);
        }
        else
        {
            RectF{ Arg::center = center, size }.draw(ColorF{ 0.18, 0.18, 0.20 });
        }
    }

    inline void DrawPlacedMapAsset(const MapEditorAsset& asset, const Vec2& bottomCenter)
    {
        if (asset.texture)
        {
            asset.texture.draw(Arg::bottomCenter = bottomCenter);
        }
        else
        {
            RectF{ Arg::bottomCenter = bottomCenter, 96, 48 }.draw(ColorF{ 0.18, 0.18, 0.20 });
        }
    }

    inline void DrawMapEditorTerrainLayer(const MapEditorState& editor)
    {
        if (editor.cells.isEmpty())
        {
            return;
        }

        for (int32 diagonal = 0; diagonal < (editor.mapWidth + editor.mapHeight - 1); ++diagonal)
        {
            const int32 xBegin = Max(0, diagonal - (editor.mapHeight - 1));
            const int32 xEnd = Min(editor.mapWidth - 1, diagonal);
            for (int32 x = xBegin; x <= xEnd; ++x)
            {
                const int32 y = diagonal - x;
                const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
                const Vec2 worldCenter = MapEditorCellCenter(x, y);
                const Vec2 bottomCenter = ToQuarterScreen(worldCenter);
                const Quad tile = ToQuarterTile(worldCenter);

                tile.draw(ColorF{ 1, 1, 1, 0.08 });
                if (0 <= cell.terrainAsset && cell.terrainAsset < static_cast<int32>(editor.assets.size()))
                {
                    DrawPlacedMapAsset(editor.assets[cell.terrainAsset], bottomCenter);
                }
                tile.drawFrame(2.0, ColorF{ 0.0, 0.75, 1.0, 0.22 });
            }
        }
    }

    inline void DrawMapEditorObjectLayer(const MapEditorState& editor)
    {
        if (editor.cells.isEmpty())
        {
            return;
        }

        Array<std::pair<Vec2, int32>> objectRenderList;
        for (int32 diagonal = 0; diagonal < (editor.mapWidth + editor.mapHeight - 1); ++diagonal)
        {
            const int32 xBegin = Max(0, diagonal - (editor.mapHeight - 1));
            const int32 xEnd = Min(editor.mapWidth - 1, diagonal);
            for (int32 x = xBegin; x <= xEnd; ++x)
            {
                const int32 y = diagonal - x;
                const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
                if (0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(editor.assets.size()))
                {
                    const Vec2 bottomCenter = ToQuarterScreen(MapEditorCellCenter(x, y));
                    objectRenderList << std::pair<Vec2, int32>{ bottomCenter.movedBy(0, -QuarterTileThickness), cell.objectAsset };
                }
            }
        }

        for (const auto& object : objectRenderList)
        {
            DrawPlacedMapAsset(editor.assets[object.second], object.first);
        }
    }

    inline void DrawMapEditorOverlay(MapEditorState& editor, const UnitCatalog& unitCatalog, const Vec2& screenMouse, const Font& uiFont)
    {
        DrawMapEditorToolbar(editor, uiFont);
        DrawUiLayoutGridControl(editor, uiFont);
        DrawUnitCatalogList(editor, unitCatalog, uiFont);
        DrawUnitParameterEditor(editor, unitCatalog, uiFont);
        if (!editor.enabled)
        {
            return;
        }

        const RectF palettePanel = EditorPalettePanelRect();
        const RectF paletteViewport = EditorPaletteViewportRect();
        palettePanel.draw(ColorF{ 0.02, 0.03, 0.045, 0.88 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
        uiFont(U"Map Assets").draw(1240, 42, Palette::White);
        uiFont(U"Wheel: scroll").draw(1420, 42, Palette::Lightgray);

        paletteViewport.draw(ColorF{ 0, 0, 0, 0.08 });
        double y = paletteViewport.y - editor.paletteScroll;
        Optional<MapEditorAssetKind> previousKind;
        const double viewportBottom = (paletteViewport.y + paletteViewport.h);
        for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
        {
            const MapEditorAsset& asset = editor.assets[i];
            if (!previousKind || *previousKind != asset.kind)
            {
                const RectF headerRect{ paletteViewport.x, y + 2.0, paletteViewport.w, 24.0 };
                if ((paletteViewport.y <= headerRect.y) && ((headerRect.y + headerRect.h) <= viewportBottom))
                {
                    headerRect.draw(ColorF{ 0.05, 0.07, 0.09, 0.94 });
                    uiFont(MapEditorAssetKindLabel(asset.kind)).draw(13, paletteViewport.x + 8.0, y + 4.0, asset.kind == MapEditorAssetKind::Terrain ? Palette::Skyblue : Palette::Orange);
                }
                y += 30.0;
                previousKind = asset.kind;
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
