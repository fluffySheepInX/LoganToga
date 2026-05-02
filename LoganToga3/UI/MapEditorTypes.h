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
        bool showDebugInfo = true;
        Array<MapEditorAsset> assets;
        Array<MapEditorCell> cells;
        int32 mapWidth = DefaultMapEditorWidth;
        int32 mapHeight = DefaultMapEditorHeight;
        int32 selectedAsset = InvalidMapEditorAsset;
        double paletteScroll = 0.0;
        bool showUnitList = false;
        double unitListScroll = 0.0;
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
}
