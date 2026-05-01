#pragma once
# include <Siv3D.hpp>
# include "../Data/UnitCatalog.h"
# include "QuarterView.h"

namespace LT3
{
    inline constexpr int32 DefaultMapEditorWidth = 12;
    inline constexpr int32 DefaultMapEditorHeight = 8;
    inline constexpr int32 InvalidMapEditorAsset = -1;

    enum class MapEditorAssetKind
    {
        Terrain,
        Object,
    };

    struct MapEditorAsset
    {
        FilePath path;
        String fileName;
        Size imageSize{ 0, 0 };
        Texture texture;
        MapEditorAssetKind kind = MapEditorAssetKind::Terrain;
    };

    struct MapEditorCell
    {
        int32 terrainAsset = InvalidMapEditorAsset;
        int32 objectAsset = InvalidMapEditorAsset;
    };

    struct MapEditorState
    {
        bool enabled = false;
        Array<MapEditorAsset> assets;
        Array<MapEditorCell> cells;
        int32 mapWidth = DefaultMapEditorWidth;
        int32 mapHeight = DefaultMapEditorHeight;
        int32 selectedAsset = InvalidMapEditorAsset;
        double paletteScroll = 0.0;
        bool showUnitList = false;
        double unitListScroll = 0.0;
        FilePath assetDirectory;
        FilePath savePath;
        String statusText = U"Map editor ready";
    };

    inline RectF EditorToolbarRect()
    {
        return RectF{ 0, 8, 1600, 52 };
    }

    inline RectF EditorToolbarButtonRect(int32 index)
    {
        return RectF{ 24.0 + index * 126.0, 18.0, 116.0, 32.0 };
    }

    inline RectF EditorPaletteRect(int32 index)
    {
        return RectF{ 1240.0, 298.0 + index * 54.0, 330.0, 46.0 };
    }

    inline RectF EditorPalettePanelRect()
    {
        return RectF{ 1220, 34, 370, 846 };
    }

    inline RectF EditorPaletteViewportRect()
    {
        return RectF{ 1240, 70, 330, 798 };
    }

    inline RectF EditorUnitListPanelRect()
    {
        return RectF{ 24, 72, 650, 610 };
    }

    inline RectF EditorUnitListViewportRect()
    {
        return RectF{ 44, 126, 610, 530 };
    }

    inline double EditorUnitListContentHeight(const UnitCatalog& catalog)
    {
        return catalog.entries.size() * 86.0;
    }

    inline double MapEditorPaletteContentHeight(const MapEditorState& editor)
    {
        if (editor.assets.isEmpty())
        {
            return 0.0;
        }

        double height = 0.0;
        Optional<MapEditorAssetKind> previousKind;
        for (const auto& asset : editor.assets)
        {
            if (!previousKind || *previousKind != asset.kind)
            {
                height += 30.0;
                previousKind = asset.kind;
            }
            height += 54.0;
        }
        return height;
    }

    inline String MapEditorAssetKindLabel(MapEditorAssetKind kind)
    {
        return kind == MapEditorAssetKind::Terrain ? U"Terrain / base size" : U"Object / decal / building";
    }

    inline Vec2 MapEditorCellCenter(int32 x, int32 y)
    {
        return Vec2{ 200.0 + x * QuarterTileStep, 90.0 + y * QuarterTileStep };
    }

    inline Optional<Point> PickMapEditorCell(const MapEditorState& editor, const Vec2& screenPos)
    {
        const Vec2 preCameraScreenPos = ToQuarterPreCameraScreen(screenPos);

        for (int32 y = 0; y < editor.mapHeight; ++y)
        {
            for (int32 x = 0; x < editor.mapWidth; ++x)
            {
                if (ToQuarterTile(MapEditorCellCenter(x, y)).intersects(preCameraScreenPos))
                {
                    return Point{ x, y };
                }
            }
        }

        return none;
    }

    inline size_t MapEditorCellIndex(const MapEditorState& editor, int32 x, int32 y)
    {
        return static_cast<size_t>(y * editor.mapWidth + x);
    }

    inline FilePath ResolveMapEditorAssetDirectory()
    {
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
        if (FileSystem::IsDirectory(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
        if (FileSystem::IsDirectory(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
    }

    inline bool IsMapEditorImageFile(const FilePath& path)
    {
        const String extension = FileSystem::Extension(path).lowercased();
        return extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"webp";
    }

    inline String TomlEscape(StringView text)
    {
        String result;
        for (const char32 ch : text)
        {
            if (ch == U'\\')
            {
                result += U"\\\\";
            }
            else if (ch == U'\"')
            {
                result += U"\\\"";
            }
            else
            {
                result += ch;
            }
        }
        return result;
    }

    inline void InitializeMapEditorCells(MapEditorState& editor)
    {
        editor.cells.assign(editor.mapWidth * editor.mapHeight, MapEditorCell{});

        int32 defaultTerrain = InvalidMapEditorAsset;
        for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
        {
            if (editor.assets[i].fileName == U"grass1.png")
            {
                defaultTerrain = i;
                break;
            }
        }
        if (defaultTerrain == InvalidMapEditorAsset)
        {
            for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
            {
                if (editor.assets[i].kind == MapEditorAssetKind::Terrain)
                {
                    defaultTerrain = i;
                    break;
                }
            }
        }

        for (auto& cell : editor.cells)
        {
            cell.terrainAsset = defaultTerrain;
        }
        editor.selectedAsset = defaultTerrain;
    }

    inline void ResizeMapEditorCells(MapEditorState& editor, int32 newWidth, int32 newHeight)
    {
        newWidth = Clamp(newWidth, 4, 40);
        newHeight = Clamp(newHeight, 4, 40);
        if (newWidth == editor.mapWidth && newHeight == editor.mapHeight)
        {
            return;
        }

        const int32 oldWidth = editor.mapWidth;
        const int32 oldHeight = editor.mapHeight;
        const Array<MapEditorCell> oldCells = editor.cells;

        editor.mapWidth = newWidth;
        editor.mapHeight = newHeight;
        editor.cells.assign(editor.mapWidth * editor.mapHeight, MapEditorCell{});

        const int32 copyWidth = Min(oldWidth, editor.mapWidth);
        const int32 copyHeight = Min(oldHeight, editor.mapHeight);
        for (int32 y = 0; y < copyHeight; ++y)
        {
            for (int32 x = 0; x < copyWidth; ++x)
            {
                editor.cells[MapEditorCellIndex(editor, x, y)] = oldCells[static_cast<size_t>(y * oldWidth + x)];
            }
        }

        editor.statusText = U"Map size: {} x {}"_fmt(editor.mapWidth, editor.mapHeight);
    }

    inline void LoadMapEditorAssets(MapEditorState& editor)
    {
        editor.assetDirectory = ResolveMapEditorAssetDirectory();
        editor.savePath = editor.assetDirectory + U"map_editor_map.toml";
        editor.assets.clear();

        Size baseSize{ 0, 0 };
        const FilePath basePath = editor.assetDirectory + U"grass1.png";
        if (FileSystem::Exists(basePath))
        {
            const Image baseImage{ basePath };
            if (baseImage)
            {
                baseSize = baseImage.size();
            }
        }

        if (FileSystem::IsDirectory(editor.assetDirectory))
        {
            for (const auto& path : FileSystem::DirectoryContents(editor.assetDirectory, Recursive::No))
            {
                if (!IsMapEditorImageFile(path))
                {
                    continue;
                }

                const Image image{ path };
                if (!image)
                {
                    continue;
                }

                MapEditorAsset asset;
                asset.path = path;
                asset.fileName = FileSystem::FileName(path);
                asset.imageSize = image.size();
                asset.texture = Texture{ image };
                asset.kind = (baseSize != Size{ 0, 0 } && asset.imageSize != baseSize) ? MapEditorAssetKind::Object : MapEditorAssetKind::Terrain;
                editor.assets << asset;
            }
        }

        editor.assets.sort_by([](const MapEditorAsset& a, const MapEditorAsset& b)
        {
            if (a.kind != b.kind)
            {
                return a.kind < b.kind;
            }
            return a.fileName < b.fileName;
        });

        InitializeMapEditorCells(editor);
        editor.statusText = U"Loaded {} map images from {}"_fmt(editor.assets.size(), editor.assetDirectory);
        if (editor.assets.isEmpty())
        {
            editor.statusText = U"No map images found: {}"_fmt(editor.assetDirectory);
        }
    }

    inline bool SaveMapEditorToml(MapEditorState& editor)
    {
        TextWriter writer{ editor.savePath };
        if (!writer)
        {
            editor.statusText = U"Save failed: {}"_fmt(editor.savePath);
            return false;
        }

        writer << U"[map]\n";
        writer << U"width = " << editor.mapWidth << U"\n";
        writer << U"height = " << editor.mapHeight << U"\n";
        writer << U"tile_step = " << QuarterTileStep << U"\n";
        writer << U"asset_directory = \"" << TomlEscape(editor.assetDirectory) << U"\"\n\n";

        for (int32 y = 0; y < editor.mapHeight; ++y)
        {
            for (int32 x = 0; x < editor.mapWidth; ++x)
            {
                const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
                const String terrain = (0 <= cell.terrainAsset && cell.terrainAsset < static_cast<int32>(editor.assets.size())) ? editor.assets[cell.terrainAsset].fileName : U"";
                const String object = (0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(editor.assets.size())) ? editor.assets[cell.objectAsset].fileName : U"";

                writer << U"[[tiles]]\n";
                writer << U"x = " << x << U"\n";
                writer << U"y = " << y << U"\n";
                writer << U"terrain = \"" << TomlEscape(terrain) << U"\"\n";
                writer << U"object = \"" << TomlEscape(object) << U"\"\n\n";
            }
        }

        editor.statusText = U"Saved TOML: {}"_fmt(editor.savePath);
        return true;
    }

    inline bool ProcessMapEditorInput(MapEditorState& editor, const Vec2& screenMouse)
    {
        bool consumed = false;
        if (EditorToolbarButtonRect(0).leftClicked())
        {
            editor.enabled = !editor.enabled;
            editor.statusText = editor.enabled ? U"Map editor ON" : U"Map editor OFF";
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(1).leftClicked())
        {
            SaveMapEditorToml(editor);
            consumed = true;
        }
        if (EditorToolbarButtonRect(2).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth - 1, editor.mapHeight);
            consumed = true;
        }
        if (EditorToolbarButtonRect(3).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth + 1, editor.mapHeight);
            consumed = true;
        }
        if (EditorToolbarButtonRect(4).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight - 1);
            consumed = true;
        }
        if (EditorToolbarButtonRect(5).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight + 1);
            consumed = true;
        }
        if (EditorToolbarButtonRect(6).leftClicked())
        {
            editor.showUnitList = !editor.showUnitList;
            editor.statusText = editor.showUnitList ? U"Unit List ON" : U"Unit List OFF";
            consumed = true;
        }
        if (EditorToolbarRect().mouseOver())
        {
            consumed = true;
        }

        if (!editor.enabled)
        {
            return consumed;
        }

        if (editor.showUnitList && EditorUnitListPanelRect().mouseOver())
        {
            consumed = true;
        }

        const RectF palettePanel = EditorPalettePanelRect();
        const RectF paletteViewport = EditorPaletteViewportRect();
        if (palettePanel.mouseOver())
        {
            const double maxScroll = Max(0.0, MapEditorPaletteContentHeight(editor) - paletteViewport.h);
            editor.paletteScroll = Clamp(editor.paletteScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
            consumed = true;

            double y = paletteViewport.y - editor.paletteScroll;
            Optional<MapEditorAssetKind> previousKind;
            for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
            {
                const MapEditorAsset& asset = editor.assets[i];
                if (!previousKind || *previousKind != asset.kind)
                {
                    y += 30.0;
                    previousKind = asset.kind;
                }

                const RectF itemRect{ paletteViewport.x, y, paletteViewport.w, 46.0 };
                if (paletteViewport.intersects(itemRect) && itemRect.leftClicked())
                {
                    editor.selectedAsset = i;
                    editor.statusText = U"Selected: {}"_fmt(asset.fileName);
                }

                y += 54.0;
            }
        }

        if (consumed)
        {
            return true;
        }

        const Optional<Point> cell = PickMapEditorCell(editor, screenMouse);
        if (!cell)
        {
            return false;
        }

        if (MouseL.down() && 0 <= editor.selectedAsset && editor.selectedAsset < static_cast<int32>(editor.assets.size()))
        {
            MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
            if (editor.assets[editor.selectedAsset].kind == MapEditorAssetKind::Terrain)
            {
                target.terrainAsset = editor.selectedAsset;
            }
            else
            {
                target.objectAsset = editor.selectedAsset;
            }
            consumed = true;
        }
        if (MouseR.down())
        {
            MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
            target.objectAsset = InvalidMapEditorAsset;
            consumed = true;
        }

        return consumed;
    }

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
        DrawEditorButton(EditorToolbarButtonRect(0), editor.enabled ? U"Map Editor: ON" : U"Map Editor: OFF", editor.enabled, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(1), U"Save TOML", false, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(2), U"W -", false, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(3), U"W +", false, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(4), U"H -", false, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(5), U"H +", false, uiFont);
        DrawEditorButton(EditorToolbarButtonRect(6), U"Unit List", editor.showUnitList, uiFont);
        uiFont(U"Map: {} x {}"_fmt(editor.mapWidth, editor.mapHeight)).draw(790, 23, Palette::White);
        uiFont(editor.statusText).draw(980, 23, Palette::Lightgray);
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
        uiFont(U"TOML data driven sample").draw(160, 86, Palette::Lightgray);
        uiFont(U"{} entries"_fmt(catalog.entries.size())).draw(560, 86, Palette::Gold);
        uiFont(catalog.statusText).draw(11, 44, 106, Palette::Lightgray);

        const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
        if (panel.mouseOver())
        {
            editor.unitListScroll = Clamp(editor.unitListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
        }

        viewport.draw(ColorF{ 0, 0, 0, 0.10 });
        const double viewportBottom = viewport.y + viewport.h;
        double y = viewport.y - editor.unitListScroll;
        for (const auto& entry : catalog.entries)
        {
            const RectF row{ viewport.x, y, viewport.w, 78.0 };
            y += 86.0;
            if (!((viewport.y <= row.y) && ((row.y + row.h) <= viewportBottom)))
            {
                continue;
            }

            const bool isBuilding = entry.kind == U"building";
            row.draw(isBuilding ? ColorF{ 0.14, 0.10, 0.08, 0.92 } : ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
            RectF{ row.x + 10, row.y + 11, 48, 48 }.draw(isBuilding ? ColorF{ 0.72, 0.45, 0.18 } : ColorF{ 0.18, 0.42, 0.72 });
            uiFont(entry.name).draw(15, row.x + 70, row.y + 8, Palette::White);
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
        DrawUnitCatalogList(editor, unitCatalog, uiFont);
        if (!editor.enabled)
        {
            return;
        }

        if (const Optional<Point> cell = PickMapEditorCell(editor, screenMouse))
        {
            ToQuarterTile(MapEditorCellCenter(cell->x, cell->y)).drawFrame(4.0, ColorF{ 1.0, 0.84, 0.0, 0.9 });
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
