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

    struct ResourceNodeEditData
    {
        ResourceKind kind = ResourceKind::Gold;
        Point cell{ 0, 0 };
        int32 amount = 700;
        int32 incomePerSec = 5;
    };

    struct MapEditorState
    {
        bool enabled = false;
        bool showDebugInfo = true;
        Array<MapEditorAsset> assets;
        Array<MapEditorCell> cells;
        int32 mapWidth = DefaultMapEditorWidth;
        int32 mapHeight = DefaultMapEditorHeight;
        int32 selectedAsset = InvalidMapEditorAsset;
        double paletteScroll = 0.0;
        bool showUnitList = false;
        bool showBuildingEditor = false;
        double unitListScroll = 0.0;
        int32 selectedUnitCatalogIndex = -1;
        bool showUnitParameterEditor = false;
        bool unitCatalogDirty = false;
        bool uiLayoutEditEnabled = false;
        int32 uiLayoutGridSize = 40;
        Vec2 uiSelectedInfoAnchor{ 24.0, 826.0 };
        Vec2 uiCommandPanelPos{ 1088.0, 668.0 };
        bool uiLayoutDraggingSelectedInfo = false;
        bool uiLayoutDraggingCommandPanel = false;
        Vec2 uiLayoutDragOffset{ 0.0, 0.0 };
        FilePath assetDirectory;
        FilePath savePath;
        FilePath uiLayoutPath;
        FilePath resourceNodeSavePath;
        Array<ResourceNodeEditData> resourceNodes;
        int32 selectedResourceNodeIndex = -1;
        double resourceNodeListScroll = 0.0;
        bool resourceNodeDragging = false;
        int32 resourceNodeFilterKind = -1;
        Optional<ResourceKind> resourcePlacementDragKind;
        HashTable<String, Texture> resourceIconTextures;
        String statusText = U"Map editor ready";
    };

    inline RectF EditorToolbarRect()
    {
        return RectF{ 0, 8, 1600, 52 };
    }

    inline RectF EditorToolbarButtonRect(int32 index)
    {
        return RectF{ 24.0 + index * 112.0, 18.0, 104.0, 32.0 };
    }

    inline Optional<int32> EditorToolbarVisualIndex(const MapEditorState& editor, int32 buttonIndex)
    {
        if (editor.enabled)
        {
            return buttonIndex;
        }

        switch (buttonIndex)
        {
        case 0:
            return 0;
        case 6:
            return 1;
        case 7:
            return 2;
        case 8:
            return 3;
        case 9:
            return 4;
        default:
            return none;
        }
    }

    inline RectF EditorToolbarButtonRect(const MapEditorState& editor, int32 buttonIndex)
    {
        const Optional<int32> visualIndex = EditorToolbarVisualIndex(editor, buttonIndex);
        if (!visualIndex)
        {
            return RectF{ 0, 0, 0, 0 };
        }

        return EditorToolbarButtonRect(*visualIndex);
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

    inline RectF EditorUnitListRowRect(const RectF& viewport, int32 index, double scroll)
    {
        return RectF{ viewport.x, viewport.y + index * 86.0 - scroll, viewport.w, 78.0 };
    }

    inline RectF EditorUnitParameterPanelRect()
    {
        return RectF{ 700, 72, 360, 316 };
    }

    inline RectF EditorUnitScaleDecrementRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 24.0, panel.y + 128.0, 48.0, 40.0 };
    }

    inline RectF EditorUnitScaleIncrementRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 288.0, panel.y + 128.0, 48.0, 40.0 };
    }

    inline RectF EditorUnitScaleResetRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 132.0, panel.y + 176.0, 96.0, 34.0 };
    }

    inline RectF EditorUnitMoveDecrementRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 24.0, panel.y + 228.0, 48.0, 40.0 };
    }

    inline RectF EditorUnitMoveIncrementRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 288.0, panel.y + 228.0, 48.0, 40.0 };
    }

    inline RectF EditorUnitMoveUseSpeedRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + 116.0, panel.y + 276.0, 128.0, 28.0 };
    }

    inline RectF EditorUnitParameterCloseRect()
    {
        const RectF panel = EditorUnitParameterPanelRect();
        return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
    }

    inline RectF EditorUiLayoutGridPanelRect()
    {
        return RectF{ 1040, 72, 236, 52 };
    }

    inline RectF EditorUiLayoutGridDecrementRect()
    {
        const RectF panel = EditorUiLayoutGridPanelRect();
        return RectF{ panel.x + 10.0, panel.y + 10.0, 32.0, 32.0 };
    }

    inline RectF EditorUiLayoutGridIncrementRect()
    {
        const RectF panel = EditorUiLayoutGridPanelRect();
        return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 32.0, 32.0 };
    }

    inline RectF EditorUiLayoutGridValueRect()
    {
        const RectF panel = EditorUiLayoutGridPanelRect();
        return RectF{ panel.x + 48.0, panel.y + 10.0, panel.w - 96.0, 32.0 };
    }

    inline RectF EditorResourceNodePanelRect()
    {
        return RectF{ 700, 404, 360, 248 };
    }

    inline RectF EditorResourceNodeCloseRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
    }

    inline RectF EditorResourceNodeRemoveRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + panel.w - 128.0, panel.y + panel.h - 42.0, 104.0, 28.0 };
    }

    inline RectF EditorResourceNodeListPanelRect()
    {
        return RectF{ 1040, 132, 236, 320 };
    }

    inline RectF EditorResourceNodeListViewportRect()
    {
        const RectF panel = EditorResourceNodeListPanelRect();
        return RectF{ panel.x + 10.0, panel.y + 34.0, panel.w - 20.0, panel.h - 44.0 };
    }

    inline RectF EditorResourceNodeListRowRect(const RectF& viewport, int32 index, double scroll)
    {
        return RectF{ viewport.x, viewport.y + index * 54.0 - scroll, viewport.w, 46.0 };
    }

    inline RectF EditorResourceNodeFilterRect(int32 index)
    {
        const RectF panel = EditorResourceNodeListPanelRect();
        return RectF{ panel.x + 10.0 + index * 54.0, panel.y + panel.h + 8.0, 48.0, 28.0 };
    }

    inline RectF EditorResourcePalettePanelRect()
    {
        return RectF{ 1040, 662, 236, 86 };
    }

    inline RectF EditorResourcePaletteIconRect(int32 index)
    {
        const RectF panel = EditorResourcePalettePanelRect();
        return RectF{ panel.x + 14.0 + index * 72.0, panel.y + 30.0, 56.0, 44.0 };
    }

    inline RectF EditorResourceClearAllRect()
    {
        const RectF panel = EditorResourcePalettePanelRect();
        return RectF{ panel.x + panel.w - 98.0, panel.y + 4.0, 88.0, 22.0 };
    }

    inline RectF EditorResourceNodeKindRect(int32 index)
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + 24.0 + index * 104.0, panel.y + 96.0, 92.0, 32.0 };
    }

    inline RectF EditorResourceNodeAmountDecRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + 24.0, panel.y + 158.0, 48.0, 40.0 };
    }

    inline RectF EditorResourceNodeAmountIncRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + 288.0, panel.y + 158.0, 48.0, 40.0 };
    }

    inline RectF EditorResourceNodeIncomeDecRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + 24.0, panel.y + 206.0, 48.0, 40.0 };
    }

    inline RectF EditorResourceNodeIncomeIncRect()
    {
        const RectF panel = EditorResourceNodePanelRect();
        return RectF{ panel.x + 288.0, panel.y + 206.0, 48.0, 40.0 };
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

    inline bool IsValidSelectedResourceNodeIndex(const MapEditorState& editor)
    {
        return 0 <= editor.selectedResourceNodeIndex
            && editor.selectedResourceNodeIndex < static_cast<int32>(editor.resourceNodes.size());
    }

    inline double EditorResourceNodeListContentHeight(const MapEditorState& editor)
    {
        return editor.resourceNodes.size() * 54.0;
    }
}
