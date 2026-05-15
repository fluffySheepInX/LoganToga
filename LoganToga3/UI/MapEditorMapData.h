#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "MapEditorResourceData.h"
# include "MapEditorUiLayout.h"
# include "MapEditorAssetUtils.h"
# include "MapEditorCellOps.h"

namespace LT3
{
    inline void LoadMapEditorAssets(MapEditorState& editor)
    {
        editor.assetDirectory = ResolveMapEditorAssetDirectory();
        editor.savePath = editor.assetDirectory + U"map_editor_map.toml";
        editor.uiLayoutPath = ResolveBattleUiLayoutTomlPath();
        editor.resourceNodeSavePath = ResolveResourceNodeTomlPath();
        editor.assets.clear();
        editor.resourceIconTextures.clear();

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
            editor.showBuildingEditor = toml[U"toolbar.building_editor"].getOr<bool>(editor.showBuildingEditor);
            editor.showDebugInfo = toml[U"toolbar.debug_info"].getOr<bool>(editor.showDebugInfo);
            editor.showBattleGrid = toml[U"toolbar.battle_grid"].getOr<bool>(editor.showBattleGrid);
            editor.uiLayoutEditEnabled = toml[U"toolbar.ui_layout_edit"].getOr<bool>(editor.uiLayoutEditEnabled);
            editor.paletteTabIndex = Clamp(toml[U"toolbar.map_assets_tab"].getOr<int32>(editor.paletteTabIndex), 0, 1);
            editor.showResourcePanels = toml[U"toolbar.resource_panels"].getOr<bool>(editor.showResourcePanels);
            editor.playerHomePosition.x = toml[U"home.player_x"].getOr<double>(editor.playerHomePosition.x);
            editor.playerHomePosition.y = toml[U"home.player_y"].getOr<double>(editor.playerHomePosition.y);
            editor.enemyHomePosition.x = toml[U"home.enemy_x"].getOr<double>(editor.enemyHomePosition.x);
            editor.enemyHomePosition.y = toml[U"home.enemy_y"].getOr<double>(editor.enemyHomePosition.y);

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
        LoadMapEditorResourceNodes(editor);

        const FilePath resourcePath = ResolveResourceTomlPath();
        const TOMLReader resourceToml{ resourcePath };
        if (resourceToml)
        {
            for (const auto resourceValue : resourceToml[U"Map001"].tableArrayView())
            {
                const String icon = resourceValue[U"icon"].getOr<String>(U"");
                if (icon.isEmpty() || editor.resourceIconTextures.contains(icon))
                {
                    continue;
                }

                const FilePath iconPath = ResolveSystemImagePath(icon);
                if (FileSystem::Exists(iconPath))
                {
                    editor.resourceIconTextures.emplace(icon, Texture{ iconPath });
                }
            }
        }
        editor.statusText = U"Loaded {} map images from {}"_fmt(editor.assets.size(), editor.assetDirectory);
        if (editor.assets.isEmpty())
        {
            editor.statusText = U"No map images found: {}"_fmt(editor.assetDirectory);
        }
    }

    inline bool SaveMapEditorToml(MapEditorState& editor, bool updateStatus = true)
    {
        SortMapEditorResourceNodes(editor);
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
        writer << U"building_editor = " << (editor.showBuildingEditor ? U"true" : U"false") << U"\n";
        writer << U"debug_info = " << (editor.showDebugInfo ? U"true" : U"false") << U"\n";
        writer << U"battle_grid = " << (editor.showBattleGrid ? U"true" : U"false") << U"\n";
        writer << U"ui_layout_edit = " << (editor.uiLayoutEditEnabled ? U"true" : U"false") << U"\n";
        writer << U"map_assets_tab = " << Clamp(editor.paletteTabIndex, 0, 1) << U"\n";
        writer << U"resource_panels = " << (editor.showResourcePanels ? U"true" : U"false") << U"\n\n";

        writer << U"[home]\n";
        writer << U"player_x = " << editor.playerHomePosition.x << U"\n";
        writer << U"player_y = " << editor.playerHomePosition.y << U"\n";
        writer << U"enemy_x = " << editor.enemyHomePosition.x << U"\n";
        writer << U"enemy_y = " << editor.enemyHomePosition.y << U"\n\n";

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

        SaveMapEditorResourceNodes(editor);

        const Array<String> resourceIssues = ValidateMapEditorResourceNodes(editor);

        if (updateStatus)
        {
            editor.statusText = U"Saved TOML: {}, {}{}"_fmt(
                editor.savePath,
                editor.resourceNodeSavePath,
                resourceIssues.isEmpty() ? U"" : U"  validation:{}"_fmt(resourceIssues.size()));
        }
        return true;
    }

    inline void InitializeMapEditorState(MapEditorState& editor)
    {
        editor.enabled = false;
        editor.showDebugInfo = true;
        editor.showBattleGrid = true;
        editor.assets.clear();
        editor.cells.clear();
        editor.mapWidth = DefaultMapEditorWidth;
        editor.mapHeight = DefaultMapEditorHeight;
        editor.selectedAsset = InvalidMapEditorAsset;
        editor.paletteScroll = 0.0;
        editor.paletteTabIndex = 0;
        editor.showResourcePanels = true;
        editor.showUnitList = false;
        editor.showBuildingEditor = false;
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
        editor.resourceNodeSavePath = ResolveResourceNodeTomlPath();
        editor.resourceNodes.clear();
        editor.selectedResourceNodeIndex = -1;
        editor.resourceNodeListScroll = 0.0;
        editor.resourceNodeDragging = false;
        editor.resourceNodeFilterKind = -1;
        editor.resourcePlacementDragKind.reset();
        editor.resourceIconTextures.clear();
        editor.playerHomePosition = { 210.0, 450.0 };
        editor.enemyHomePosition = { 1390.0, 450.0 };
        editor.draggingPlayerHome = false;
        editor.draggingEnemyHome = false;
        editor.statusText = U"Map editor ready";
    }
}
