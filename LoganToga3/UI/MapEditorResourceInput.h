#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
    inline Optional<size_t> FindResourceNodeAtCell(const MapEditorState& editor, const Point& cell)
    {
        for (size_t i = 0; i < editor.resourceNodes.size(); ++i)
        {
            if (editor.resourceNodes[i].cell == cell)
            {
                return i;
            }
        }

        return none;
    }

    inline void SelectResourceNodeAtCell(MapEditorState& editor, const Point& cell)
    {
        if (const Optional<size_t> index = FindResourceNodeAtCell(editor, cell))
        {
            editor.selectedResourceNodeIndex = static_cast<int32>(*index);
        }
        else
        {
            editor.selectedResourceNodeIndex = -1;
        }
    }

    inline void RemoveSelectedResourceNode(MapEditorState& editor)
    {
        if (!IsValidSelectedResourceNodeIndex(editor))
        {
            return;
        }

        editor.resourceNodes.remove_at(editor.selectedResourceNodeIndex);
        editor.selectedResourceNodeIndex = -1;
        editor.statusText = U"Resource node removed";
    }

    inline void SelectResourceNodeIndex(MapEditorState& editor, int32 index)
    {
        editor.selectedResourceNodeIndex = (0 <= index && index < static_cast<int32>(editor.resourceNodes.size())) ? index : -1;
    }

    inline void NudgeSelectedResourceNode(MapEditorState& editor, int32 dx, int32 dy)
    {
        if (!IsValidSelectedResourceNodeIndex(editor))
        {
            return;
        }

        ResourceNodeEditData& node = editor.resourceNodes[editor.selectedResourceNodeIndex];
        node.cell.x = Clamp(node.cell.x + dx, 0, editor.mapWidth - 1);
        node.cell.y = Clamp(node.cell.y + dy, 0, editor.mapHeight - 1);
        editor.statusText = U"Moved resource node to ({}, {})"_fmt(node.cell.x, node.cell.y);
    }

    inline void DuplicateSelectedResourceNode(MapEditorState& editor)
    {
        if (!IsValidSelectedResourceNodeIndex(editor))
        {
            return;
        }

        ResourceNodeEditData copy = editor.resourceNodes[editor.selectedResourceNodeIndex];
        for (int32 y = 0; y < editor.mapHeight; ++y)
        {
            for (int32 x = 0; x < editor.mapWidth; ++x)
            {
                const Point candidate{ x, y };
                if (!FindResourceNodeAtCell(editor, candidate))
                {
                    copy.cell = candidate;
                    editor.resourceNodes << copy;
                    SortMapEditorResourceNodes(editor);
                    if (const Optional<size_t> index = FindResourceNodeAtCell(editor, candidate))
                    {
                        editor.selectedResourceNodeIndex = static_cast<int32>(*index);
                    }
                    editor.statusText = U"Duplicated resource node";
                    return;
                }
            }
        }

        editor.statusText = U"Duplicate failed: no empty cell";
    }

    inline void ClearAllResourceNodes(MapEditorState& editor)
    {
        editor.resourceNodes.clear();
        editor.selectedResourceNodeIndex = -1;
        editor.resourceNodeDragging = false;
        editor.statusText = U"All resource nodes cleared";
    }

    inline void BeginResourcePlacementDrag(MapEditorState& editor, ResourceKind kind)
    {
        editor.resourcePlacementDragKind = kind;
        editor.statusText = U"Placing resource: {}"_fmt(ResourceKindLabel(kind));
    }

    inline void CommitDraggedResourcePlacement(MapEditorState& editor, const Point& cell)
    {
        if (!editor.resourcePlacementDragKind)
        {
            return;
        }

        if (const Optional<size_t> existing = FindResourceNodeAtCell(editor, cell))
        {
            editor.resourceNodes[*existing].kind = *editor.resourcePlacementDragKind;
            editor.selectedResourceNodeIndex = static_cast<int32>(*existing);
            editor.statusText = U"Resource updated: {}"_fmt(ResourceKindLabel(*editor.resourcePlacementDragKind));
        }
        else
        {
            editor.resourceNodes << ResourceNodeEditData{ *editor.resourcePlacementDragKind, cell, 700, 5 };
            SortMapEditorResourceNodes(editor);
            if (const Optional<size_t> index = FindResourceNodeAtCell(editor, cell))
            {
                editor.selectedResourceNodeIndex = static_cast<int32>(*index);
            }
            editor.statusText = U"Resource placed: {}"_fmt(ResourceKindLabel(*editor.resourcePlacementDragKind));
        }

        editor.resourcePlacementDragKind.reset();
    }

    inline bool ProcessResourcePaletteInput(MapEditorState& editor)
    {
        if (!editor.enabled)
        {
            return false;
        }

        if (EditorResourceClearAllRect().leftClicked())
        {
            ClearAllResourceNodes(editor);
            return true;
        }

        for (int32 i = 0; i < 3; ++i)
        {
            if (EditorResourcePaletteIconRect(i).leftPressed())
            {
                BeginResourcePlacementDrag(editor, static_cast<ResourceKind>(i));
                return true;
            }
        }

        if (editor.resourcePlacementDragKind && MouseL.up())
        {
            return true;
        }

        return EditorResourcePalettePanelRect().mouseOver();
    }

    inline bool ProcessResourceNodeEditorPanelInput(MapEditorState& editor)
    {
        if (!IsValidSelectedResourceNodeIndex(editor))
        {
            return false;
        }

        bool consumed = false;
        ResourceNodeEditData& node = editor.resourceNodes[editor.selectedResourceNodeIndex];
        if (EditorResourceNodeCloseRect().leftClicked())
        {
            editor.selectedResourceNodeIndex = -1;
            return true;
        }

        for (int32 i = 0; i < 3; ++i)
        {
            if (EditorResourceNodeKindRect(i).leftClicked())
            {
                node.kind = static_cast<ResourceKind>(i);
                editor.statusText = U"Resource kind: {}"_fmt(ResourceKindLabel(node.kind));
                consumed = true;
            }
        }

        if (EditorResourceNodeAmountDecRect().leftClicked())
        {
            node.amount = Max(0, node.amount - 100);
            editor.statusText = U"Resource amount: {}"_fmt(node.amount);
            consumed = true;
        }
        if (EditorResourceNodeAmountIncRect().leftClicked())
        {
            node.amount += 100;
            editor.statusText = U"Resource amount: {}"_fmt(node.amount);
            consumed = true;
        }
        if (EditorResourceNodeIncomeDecRect().leftClicked())
        {
            node.incomePerSec = Max(0, node.incomePerSec - 1);
            editor.statusText = U"Resource income: {}"_fmt(node.incomePerSec);
            consumed = true;
        }
        if (EditorResourceNodeIncomeIncRect().leftClicked())
        {
            node.incomePerSec += 1;
            editor.statusText = U"Resource income: {}"_fmt(node.incomePerSec);
            consumed = true;
        }
        if (EditorResourceNodeRemoveRect().leftClicked())
        {
            RemoveSelectedResourceNode(editor);
            return true;
        }

        if (EditorResourceNodePanelRect().mouseOver())
        {
            consumed = true;
        }

        return consumed;
    }

    inline bool ProcessResourceNodeListInput(MapEditorState& editor)
    {
        if (!editor.enabled)
        {
            return false;
        }

        const RectF panel = EditorResourceNodeListPanelRect();
        const RectF viewport = EditorResourceNodeListViewportRect();
        for (int32 i = 0; i < 4; ++i)
        {
            if (EditorResourceNodeFilterRect(i).leftClicked())
            {
                editor.resourceNodeFilterKind = (i == 0) ? -1 : (i - 1);
                editor.resourceNodeListScroll = 0.0;
                editor.statusText = U"Resource filter: {}"_fmt((i == 0) ? U"All" : ResourceKindLabel(static_cast<ResourceKind>(i - 1)));
                return true;
            }
        }
        if (!panel.mouseOver())
        {
            return false;
        }

        const double maxScroll = Max(0.0, EditorResourceNodeListContentHeight(editor) - viewport.h);
        editor.resourceNodeListScroll = Clamp(editor.resourceNodeListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);

        for (int32 i = 0; i < static_cast<int32>(editor.resourceNodes.size()); ++i)
        {
            if (!PassesResourceNodeFilter(editor, editor.resourceNodes[i].kind))
            {
                continue;
            }
            const RectF row = EditorResourceNodeListRowRect(viewport, i, editor.resourceNodeListScroll);
            if (viewport.intersects(row) && row.leftClicked())
            {
                editor.selectedResourceNodeIndex = i;
                editor.statusText = U"Selected resource node: {}"_fmt(ResourceKindLabel(editor.resourceNodes[i].kind));
            }
        }

        return true;
    }
}
