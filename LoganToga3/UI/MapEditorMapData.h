#pragma once
# include <Siv3D.hpp>
# include "../Data/BattleAssetPaths.h"
# include "../Data/Loaders/ResourceDefLoader.h"
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

                MapEditorAsset asset;
                asset.path = path;
                asset.fileName = FileSystem::FileName(path);
                if (!LoadMapEditorAssetVisual(asset))
                {
                    continue;
                }
                asset.kind = (baseSize != Size{ 0, 0 } && asset.imageSize != baseSize) ? MapEditorAssetKind::Object : MapEditorAssetKind::Terrain;
                NormalizeDecalSettings(asset);
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
        const int32 assetCount = static_cast<int32>(editor.assets.size());
        if ((editor.selectedAsset < 0) || (assetCount <= editor.selectedAsset))
        {
            editor.selectedAsset = InvalidMapEditorAsset;
        }
        if ((editor.decalEditorAssetIndex < 0) || (assetCount <= editor.decalEditorAssetIndex))
        {
            editor.decalEditorAssetIndex = InvalidMapEditorAsset;
            editor.showDecalEditor = false;
        }

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
            editor.showCommandEditor = toml[U"toolbar.command_editor"].getOr<bool>(editor.showCommandEditor);
            editor.showSkillEditor = toml[U"toolbar.skill_editor"].getOr<bool>(editor.showSkillEditor);
            editor.showAiEditor = toml[U"toolbar.ai_editor"].getOr<bool>(editor.showAiEditor);
            editor.showDebugInfo = toml[U"toolbar.debug_info"].getOr<bool>(editor.showDebugInfo);
            editor.showEnemyMoveMarkers = toml[U"toolbar.enemy_move_markers"].getOr<bool>(editor.showEnemyMoveMarkers);
            editor.showBattleGrid = toml[U"toolbar.battle_grid"].getOr<bool>(editor.showBattleGrid);
            editor.uiLayoutEditEnabled = toml[U"toolbar.ui_layout_edit"].getOr<bool>(editor.uiLayoutEditEnabled);
            editor.paletteTabIndex = Clamp(toml[U"toolbar.map_assets_tab"].getOr<int32>(editor.paletteTabIndex), 0, 1);
            editor.showResourcePanels = toml[U"toolbar.resource_panels"].getOr<bool>(editor.showResourcePanels);
            editor.showPerlinNoisePanel = false;
            editor.showStarToolMenu = false;
            editor.showFogPanel = false;
            editor.perlinMapWidth = editor.mapWidth;
            editor.perlinMapHeight = editor.mapHeight;
            editor.fogEnabled = toml[U"fog.enabled"].getOr<bool>(editor.fogEnabled);
            editor.fogColor.r = Clamp(toml[U"fog.color_r"].getOr<double>(editor.fogColor.r), 0.0, 1.0);
            editor.fogColor.g = Clamp(toml[U"fog.color_g"].getOr<double>(editor.fogColor.g), 0.0, 1.0);
            editor.fogColor.b = Clamp(toml[U"fog.color_b"].getOr<double>(editor.fogColor.b), 0.0, 1.0);
            editor.fogOpacity = Clamp(toml[U"fog.opacity"].getOr<double>(editor.fogOpacity), 0.0, 1.0);
            editor.fogPreviewVision = toml[U"fog.preview_vision"].getOr<bool>(editor.fogPreviewVision);
            editor.playerHomePosition.x = toml[U"home.player_x"].getOr<double>(editor.playerHomePosition.x);
            editor.playerHomePosition.y = toml[U"home.player_y"].getOr<double>(editor.playerHomePosition.y);
            editor.enemyHomePosition.x = toml[U"home.enemy_x"].getOr<double>(editor.enemyHomePosition.x);
            editor.enemyHomePosition.y = toml[U"home.enemy_y"].getOr<double>(editor.enemyHomePosition.y);
            editor.selectedAiProfileTag = toml[U"ai.selected_profile"].getOr<String>(editor.selectedAiProfileTag).lowercased();

            const TOMLValue tilesValue = toml[U"tiles"];
            if (tilesValue.isTableArray())
            {
                for (const auto tileValue : tilesValue.tableArrayView())
                {
                    const int32 x = tileValue[U"x"].getOr<int32>(-1);
                    const int32 y = tileValue[U"y"].getOr<int32>(-1);
                    if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
                    {
                        continue;
                    }

                    MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
                    cell.decals.clear();

                    const String terrain = tileValue[U"terrain"].getOr<String>(U"");
                    cell.terrainAsset = terrain.isEmpty()
                        ? InvalidMapEditorAsset
                        : FindMapEditorAssetIndexByFileName(editor, terrain);

                    const String object = tileValue[U"object"].getOr<String>(U"");
                    cell.objectAsset = object.isEmpty()
                        ? InvalidMapEditorAsset
                        : FindMapEditorAssetIndexByFileName(editor, object);
                    if (IsMapEditorDecalAsset(editor, cell.objectAsset))
                    {
                        cell.objectAsset = InvalidMapEditorAsset;
                    }

                    bool hasDecals = false;
                    const TOMLValue decalsValue = tileValue[U"decals"];
                    if (decalsValue.isTableArray())
                    {
                        for (const auto decalValue : decalsValue.tableArrayView())
                        {
                            const String decalFileName = decalValue[U"asset"].getOr<String>(U"");
                            const int32 decalAssetIndex = decalFileName.isEmpty()
                                ? InvalidMapEditorAsset
                                : FindMapEditorAssetIndexByFileName(editor, decalFileName);
                            if (!IsMapEditorDecalAsset(editor, decalAssetIndex))
                            {
                                continue;
                            }

                            MapEditorAsset& asset = editor.assets[decalAssetIndex];
                            asset.decalOpacity = decalValue[U"decal_opacity"].getOr<double>(asset.decalOpacity);
                            asset.decalScale = decalValue[U"decal_scale"].getOr<double>(asset.decalScale);
                            asset.decalBlocksPassage = decalValue[U"decal_blocks_passage"].getOr<bool>(asset.decalBlocksPassage);
                            asset.useRandomDecalOpacity = decalValue[U"decal_opacity_random"].getOr<bool>(asset.useRandomDecalOpacity);
                            asset.decalOpacityMin = decalValue[U"decal_opacity_min"].getOr<double>(asset.decalOpacityMin);
                            asset.decalOpacityMax = decalValue[U"decal_opacity_max"].getOr<double>(asset.decalOpacityMax);
                            asset.useRandomDecalScale = decalValue[U"decal_scale_random"].getOr<bool>(asset.useRandomDecalScale);
                            asset.decalScaleMin = decalValue[U"decal_scale_min"].getOr<double>(asset.decalScaleMin);
                            asset.decalScaleMax = decalValue[U"decal_scale_max"].getOr<double>(asset.decalScaleMax);
                            NormalizeDecalSettings(asset);
                            cell.decals << MapEditorDecalPlacement{
                                decalAssetIndex,
                                Clamp(decalValue[U"decal_applied_opacity"].getOr<double>(asset.decalOpacity), 0.0, 1.0),
                                Clamp(decalValue[U"decal_applied_scale"].getOr<double>(asset.decalScale), 0.1, 4.0)
                            };
                            hasDecals = true;
                        }
                    }

                    if (!hasDecals)
                    {
                        cell.decalOpacity = 1.0;
                        cell.decalScale = 1.0;
                    }
                    SyncLegacyDecalFieldsFromStack(cell);
                }
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
        writer << U"command_editor = " << (editor.showCommandEditor ? U"true" : U"false") << U"\n";
        writer << U"skill_editor = " << (editor.showSkillEditor ? U"true" : U"false") << U"\n";
        writer << U"ai_editor = " << (editor.showAiEditor ? U"true" : U"false") << U"\n";
        writer << U"debug_info = " << (editor.showDebugInfo ? U"true" : U"false") << U"\n";
        writer << U"enemy_move_markers = " << (editor.showEnemyMoveMarkers ? U"true" : U"false") << U"\n";
        writer << U"battle_grid = " << (editor.showBattleGrid ? U"true" : U"false") << U"\n";
        writer << U"ui_layout_edit = " << (editor.uiLayoutEditEnabled ? U"true" : U"false") << U"\n";
        writer << U"map_assets_tab = " << Clamp(editor.paletteTabIndex, 0, 1) << U"\n";
        writer << U"resource_panels = " << (editor.showResourcePanels ? U"true" : U"false") << U"\n\n";

        writer << U"[fog]\n";
        writer << U"enabled = " << (editor.fogEnabled ? U"true" : U"false") << U"\n";
        writer << U"color_r = " << Clamp(editor.fogColor.r, 0.0, 1.0) << U"\n";
        writer << U"color_g = " << Clamp(editor.fogColor.g, 0.0, 1.0) << U"\n";
        writer << U"color_b = " << Clamp(editor.fogColor.b, 0.0, 1.0) << U"\n";
        writer << U"opacity = " << Clamp(editor.fogOpacity, 0.0, 1.0) << U"\n";
        writer << U"preview_vision = " << (editor.fogPreviewVision ? U"true" : U"false") << U"\n\n";

        writer << U"[home]\n";
        writer << U"player_x = " << editor.playerHomePosition.x << U"\n";
        writer << U"player_y = " << editor.playerHomePosition.y << U"\n";
        writer << U"enemy_x = " << editor.enemyHomePosition.x << U"\n";
        writer << U"enemy_y = " << editor.enemyHomePosition.y << U"\n\n";

        writer << U"[ai]\n";
        writer << U"selected_profile = \"" << TomlEscape(editor.selectedAiProfileTag.lowercased()) << U"\"\n\n";

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
                const bool legacyObjectIsDecal = IsMapEditorDecalAsset(editor, cell.objectAsset);
                const String object = (!legacyObjectIsDecal && 0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(editor.assets.size())) ? editor.assets[cell.objectAsset].fileName : U"";

                writer << U"[[tiles]]\n";
                writer << U"x = " << x << U"\n";
                writer << U"y = " << y << U"\n";
                writer << U"terrain = \"" << TomlEscape(terrain) << U"\"\n";
                writer << U"object = \"" << TomlEscape(object) << U"\"\n";
                for (const MapEditorDecalPlacement& decal : cell.decals)
                {
                    if (!IsMapEditorDecalAsset(editor, decal.assetIndex))
                    {
                        continue;
                    }

                    const MapEditorAsset& asset = editor.assets[decal.assetIndex];
                    writer << U"[[tiles.decals]]\n";
                    writer << U"asset = \"" << TomlEscape(asset.fileName) << U"\"\n";
                    writer << U"decal_opacity = " << asset.decalOpacity << U"\n";
                    writer << U"decal_scale = " << asset.decalScale << U"\n";
                    writer << U"decal_blocks_passage = " << (asset.decalBlocksPassage ? U"true" : U"false") << U"\n";
                    writer << U"decal_opacity_random = " << (asset.useRandomDecalOpacity ? U"true" : U"false") << U"\n";
                    writer << U"decal_opacity_min = " << asset.decalOpacityMin << U"\n";
                    writer << U"decal_opacity_max = " << asset.decalOpacityMax << U"\n";
                    writer << U"decal_scale_random = " << (asset.useRandomDecalScale ? U"true" : U"false") << U"\n";
                    writer << U"decal_scale_min = " << asset.decalScaleMin << U"\n";
                    writer << U"decal_scale_max = " << asset.decalScaleMax << U"\n";
                    writer << U"decal_applied_opacity = " << decal.opacity << U"\n";
                    writer << U"decal_applied_scale = " << decal.scale << U"\n";
                }
                writer << U"\n";
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
        editor.showCommandEditor = false;
        editor.buildingEditorTab = 0;
        editor.buildingEditorLineActionTag.clear();
        editor.buildingEditorIconHorizontal.clear();
        editor.buildingEditorIconDiagUpRight.clear();
        editor.buildingEditorIconDiagUpLeft.clear();
        editor.buildLineIconsDirty = false;
        editor.commandListScroll = 0.0;
        editor.commandUnitListScroll = 0.0;
        editor.selectedCommandActionIndex = -1;
        editor.commandEditorMode = 0;
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
        editor.showStarToolMenu = false;
        editor.showPerlinNoisePanel = false;
        editor.showFogPanel = false;
        editor.fogEnabled = true;
        editor.fogColor = ColorF{ 0.02, 0.025, 0.035 };
        editor.fogOpacity = 0.52;
        editor.fogPreviewVision = true;
        editor.perlinStack.clear();
        editor.perlinMapWidth = editor.mapWidth;
        editor.perlinMapHeight = editor.mapHeight;
        editor.playerHomePosition = { 210.0, 450.0 };
        editor.enemyHomePosition = { 1390.0, 450.0 };
        editor.draggingPlayerHome = false;
        editor.draggingEnemyHome = false;
        editor.lastPaintCell.reset();
        editor.lastPaintAsset = InvalidMapEditorAsset;
        editor.lastEraseCell.reset();
        editor.showDecalEditor = false;
        editor.decalEditorAssetIndex = InvalidMapEditorAsset;
        editor.zOrderMode = false;
        editor.zOrderDragStartCell.reset();
        editor.zOrderSelectionRect.reset();
        editor.zOrderSelectedStackIndex = 0;
        editor.statusText = U"Map editor ready";
    }
}
