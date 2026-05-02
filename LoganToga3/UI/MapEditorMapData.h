#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "MapEditorUiLayout.h"

namespace LT3
{
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

    inline int32 FindMapEditorAssetIndexByFileName(const MapEditorState& editor, StringView fileName)
    {
        for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
        {
            if (editor.assets[i].fileName == fileName)
            {
                return i;
            }
        }

        return InvalidMapEditorAsset;
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
        editor.uiLayoutPath = ResolveBattleUiLayoutTomlPath();
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

        const TOMLReader toml{ editor.savePath };
        if (toml)
        {
            editor.mapWidth = Clamp(toml[U"map.width"].getOr<int32>(editor.mapWidth), 4, 40);
            editor.mapHeight = Clamp(toml[U"map.height"].getOr<int32>(editor.mapHeight), 4, 40);
        }

        InitializeMapEditorCells(editor);
        if (toml)
        {
            editor.enabled = toml[U"toolbar.map_editor_enabled"].getOr<bool>(editor.enabled);
            editor.showUnitList = toml[U"toolbar.unit_list"].getOr<bool>(editor.showUnitList);
            editor.showDebugInfo = toml[U"toolbar.debug_info"].getOr<bool>(editor.showDebugInfo);
            editor.uiLayoutEditEnabled = toml[U"toolbar.ui_layout_edit"].getOr<bool>(editor.uiLayoutEditEnabled);

            for (const auto tileValue : toml[U"tiles"].tableArrayView())
            {
                const int32 x = tileValue[U"x"].getOr<int32>(-1);
                const int32 y = tileValue[U"y"].getOr<int32>(-1);
                if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
                {
                    continue;
                }

                MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];

                const String terrain = tileValue[U"terrain"].getOr<String>(U"");
                cell.terrainAsset = terrain.isEmpty()
                    ? InvalidMapEditorAsset
                    : FindMapEditorAssetIndexByFileName(editor, terrain);

                const String object = tileValue[U"object"].getOr<String>(U"");
                cell.objectAsset = object.isEmpty()
                    ? InvalidMapEditorAsset
                    : FindMapEditorAssetIndexByFileName(editor, object);
            }
        }
        LoadBattleUiLayoutToml(editor);
        editor.statusText = U"Loaded {} map images from {}"_fmt(editor.assets.size(), editor.assetDirectory);
        if (editor.assets.isEmpty())
        {
            editor.statusText = U"No map images found: {}"_fmt(editor.assetDirectory);
        }
    }

    inline bool SaveMapEditorToml(MapEditorState& editor, bool updateStatus = true)
    {
        FileSystem::CreateDirectories(FileSystem::ParentPath(editor.savePath));
        TextWriter writer{ editor.savePath };
        if (!writer)
        {
            if (updateStatus)
            {
                editor.statusText = U"Save failed: {}"_fmt(editor.savePath);
            }
            return false;
        }

        writer << U"[toolbar]\n";
        writer << U"map_editor_enabled = " << (editor.enabled ? U"true" : U"false") << U"\n";
        writer << U"unit_list = " << (editor.showUnitList ? U"true" : U"false") << U"\n";
        writer << U"debug_info = " << (editor.showDebugInfo ? U"true" : U"false") << U"\n";
        writer << U"ui_layout_edit = " << (editor.uiLayoutEditEnabled ? U"true" : U"false") << U"\n\n";

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

        if (updateStatus)
        {
            editor.statusText = U"Saved TOML: {}"_fmt(editor.savePath);
        }
        return true;
    }

    inline void InitializeMapEditorState(MapEditorState& editor)
    {
        editor.enabled = false;
        editor.showDebugInfo = true;
        editor.assets.clear();
        editor.cells.clear();
        editor.mapWidth = DefaultMapEditorWidth;
        editor.mapHeight = DefaultMapEditorHeight;
        editor.selectedAsset = InvalidMapEditorAsset;
        editor.paletteScroll = 0.0;
        editor.showUnitList = false;
        editor.unitListScroll = 0.0;
        editor.selectedUnitCatalogIndex = -1;
        editor.showUnitParameterEditor = false;
        editor.unitCatalogDirty = false;
        editor.uiLayoutEditEnabled = false;
        editor.uiLayoutGridSize = 40;
        editor.uiSelectedInfoAnchor = { 24.0, 826.0 };
        editor.uiCommandPanelPos = { 1088.0, 668.0 };
        editor.uiLayoutDraggingSelectedInfo = false;
        editor.uiLayoutDraggingCommandPanel = false;
        editor.uiLayoutDragOffset = { 0.0, 0.0 };
        editor.assetDirectory = ResolveMapEditorAssetDirectory();
        editor.savePath = editor.assetDirectory + U"map_editor_map.toml";
        editor.uiLayoutPath = ResolveBattleUiLayoutTomlPath();
        editor.statusText = U"Map editor ready";
    }
}
