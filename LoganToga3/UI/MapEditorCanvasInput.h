#pragma once
# include <Siv3D.hpp>
# include "MapEditorResourceInput.h"

namespace LT3
{
    inline bool ProcessMapEditorPaletteInput(MapEditorState& editor)
    {
        const RectF palettePanel = EditorPalettePanelRect();
        const RectF paletteViewport = EditorPaletteViewportRect();
        if (!palettePanel.mouseOver())
        {
            return false;
        }

        const double maxScroll = Max(0.0, MapEditorPaletteContentHeight(editor) - paletteViewport.h);
        editor.paletteScroll = Clamp(editor.paletteScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);

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

        return true;
    }

    inline bool ProcessMapEditorCanvasInput(MapEditorState& editor, const Vec2& screenMouse)
    {
        if (KeySpace.pressed())
        {
            return false;
        }

        const Optional<Point> cell = PickMapEditorCell(editor, screenMouse);
        if (!cell)
        {
            if (editor.resourcePlacementDragKind && MouseL.up())
            {
                editor.resourcePlacementDragKind.reset();
                return true;
            }
            return false;
        }

        bool consumed = false;

        if (editor.resourcePlacementDragKind && MouseL.up())
        {
            CommitDraggedResourcePlacement(editor, *cell);
            return true;
        }

        if (editor.resourcePlacementDragKind)
        {
            return true;
        }

        if (KeyR.down())
        {
            if (const Optional<size_t> existing = FindResourceNodeAtCell(editor, *cell))
            {
                editor.resourceNodes[*existing].kind = NextResourceKind(editor.resourceNodes[*existing].kind);
                editor.selectedResourceNodeIndex = static_cast<int32>(*existing);
                editor.statusText = U"Resource kind: {}"_fmt(ResourceKindToTag(editor.resourceNodes[*existing].kind));
            }
            else
            {
                editor.resourceNodes << ResourceNodeEditData{ ResourceKind::Gold, *cell, 700, 5 };
                SortMapEditorResourceNodes(editor);
                editor.selectedResourceNodeIndex = static_cast<int32>(editor.resourceNodes.size() - 1);
                if (const Optional<size_t> index = FindResourceNodeAtCell(editor, *cell))
                {
                    editor.selectedResourceNodeIndex = static_cast<int32>(*index);
                }
                editor.statusText = U"Resource node added";
            }
            consumed = true;
        }
        if (KeyDelete.down() || KeyBackspace.down())
        {
            SelectResourceNodeAtCell(editor, *cell);
            RemoveSelectedResourceNode(editor);
            consumed = true;
        }
        if (const Optional<size_t> existing = FindResourceNodeAtCell(editor, *cell))
        {
            editor.selectedResourceNodeIndex = static_cast<int32>(*existing);
            if (KeyPageUp.down())
            {
                editor.resourceNodes[*existing].incomePerSec += 1;
                editor.statusText = U"Resource income: {}"_fmt(editor.resourceNodes[*existing].incomePerSec);
                consumed = true;
            }
            if (KeyPageDown.down())
            {
                editor.resourceNodes[*existing].incomePerSec = Max(0, editor.resourceNodes[*existing].incomePerSec - 1);
                editor.statusText = U"Resource income: {}"_fmt(editor.resourceNodes[*existing].incomePerSec);
                consumed = true;
            }
        }

        if (MouseM.down())
        {
            SelectResourceNodeAtCell(editor, *cell);
            if (IsValidSelectedResourceNodeIndex(editor))
            {
                editor.statusText = U"Selected resource node: {}"_fmt(ResourceKindLabel(editor.resourceNodes[editor.selectedResourceNodeIndex].kind));
                consumed = true;
            }
        }

        if (IsValidSelectedResourceNodeIndex(editor) && MouseL.down() && KeyShift.pressed())
        {
            editor.resourceNodeDragging = true;
            editor.statusText = U"Dragging resource node";
            consumed = true;
        }
        if (editor.resourceNodeDragging && MouseL.pressed() && IsValidSelectedResourceNodeIndex(editor))
        {
            editor.resourceNodes[editor.selectedResourceNodeIndex].cell = *cell;
            SortMapEditorResourceNodes(editor);
            if (const Optional<size_t> index = FindResourceNodeAtCell(editor, *cell))
            {
                editor.selectedResourceNodeIndex = static_cast<int32>(*index);
            }
            consumed = true;
        }
        if (editor.resourceNodeDragging && MouseL.up())
        {
            editor.resourceNodeDragging = false;
            if (IsValidSelectedResourceNodeIndex(editor))
            {
                const auto& node = editor.resourceNodes[editor.selectedResourceNodeIndex];
                editor.statusText = U"Moved resource node to ({}, {})"_fmt(node.cell.x, node.cell.y);
            }
            consumed = true;
        }

        if (MouseL.pressed() && 0 <= editor.selectedAsset && editor.selectedAsset < static_cast<int32>(editor.assets.size()))
        {
            if (const Optional<size_t> existing = FindResourceNodeAtCell(editor, *cell))
            {
                if (PassesResourceNodeFilter(editor, editor.resourceNodes[*existing].kind))
                {
                    SelectResourceNodeIndex(editor, static_cast<int32>(*existing));
                    editor.statusText = U"Selected resource node: {}"_fmt(ResourceKindLabel(editor.resourceNodes[*existing].kind));
                    return true;
                }
            }
            if (IsValidSelectedResourceNodeIndex(editor) && KeyShift.pressed())
            {
                return true;
            }
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
        if (MouseR.pressed())
        {
            MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
            target.objectAsset = InvalidMapEditorAsset;
            consumed = true;
        }

        return consumed;
    }
}
