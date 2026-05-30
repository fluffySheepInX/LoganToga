#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "RectUiHelpers.h"

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
        if (0 <= editor.selectedResourceNodeIndex && editor.selectedResourceNodeIndex < static_cast<int32>(editor.resourceCaptureTimeSteps.size()))
        {
            editor.resourceCaptureTimeSteps.remove_at(editor.selectedResourceNodeIndex);
        }
        editor.resourceCaptureTimeEditingIndex = -1;
        editor.resourceCaptureTimeEditingText.clear();
        editor.resourceCaptureTimeStepMenuIndex = none;
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
                    EnsureResourceCaptureTimeSteps(editor);
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
        editor.resourceCaptureTimeEditingIndex = -1;
        editor.resourceCaptureTimeEditingText.clear();
        editor.resourceCaptureTimeStepMenuIndex = none;
        editor.resourceCaptureTimeSteps.clear();
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
            editor.resourceNodes << ResourceNodeEditData{ *editor.resourcePlacementDragKind, cell, 700, 5, false, 1.5 };
            EnsureResourceCaptureTimeSteps(editor);
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
        EnsureResourceCaptureTimeSteps(editor);

        if (editor.resourceCaptureTimeEditingIndex == editor.selectedResourceNodeIndex)
        {
            TextInput::UpdateText(editor.resourceCaptureTimeEditingText);
            if (KeyEscape.down())
            {
                editor.resourceCaptureTimeEditingIndex = -1;
                editor.resourceCaptureTimeEditingText.clear();
                return true;
            }
            if (KeyEnter.down())
            {
                if (TryCommitResourceCaptureTimeText(node, editor.resourceCaptureTimeEditingText))
                {
                    editor.statusText = U"Capture time: {:.1f}s"_fmt(node.captureTimeSec);
                }
                else
                {
                    editor.statusText = U"Invalid capture time: {}"_fmt(editor.resourceCaptureTimeEditingText);
                }
                editor.resourceCaptureTimeEditingIndex = -1;
                editor.resourceCaptureTimeEditingText.clear();
                return true;
            }
        }

        if (editor.resourceCaptureTimeStepMenuIndex && *editor.resourceCaptureTimeStepMenuIndex == editor.selectedResourceNodeIndex)
        {
            const Array<double>& steps = ResourceCaptureTimeStepOptions();
            const Vec2 menuPos = editor.resourceCaptureTimeStepMenuPos;
            const RectF menuRect{ menuPos.x, menuPos.y, 88.0, 8.0 + static_cast<double>(steps.size()) * 22.0 };
            for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
            {
                const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
                if (item.leftClicked())
                {
                    SetResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex, steps[i]);
                    editor.resourceCaptureTimeStepMenuIndex = none;
                    editor.statusText = U"Capture step set to {}"_fmt(steps[i]);
                    return true;
                }
            }

            if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
            {
                editor.resourceCaptureTimeStepMenuIndex = none;
                return true;
            }

            return true;
        }

        if (EditorResourceNodeCloseRect(editor).leftClicked())
        {
            editor.resourceCaptureTimeEditingIndex = -1;
            editor.resourceCaptureTimeEditingText.clear();
            editor.resourceCaptureTimeStepMenuIndex = none;
            editor.selectedResourceNodeIndex = -1;
            return true;
        }

        for (int32 i = 0; i < 3; ++i)
        {
            if (EditorResourceNodeKindRect(editor, i).leftClicked())
            {
                node.kind = static_cast<ResourceKind>(i);
                editor.statusText = U"Resource kind: {}"_fmt(ResourceKindLabel(node.kind));
                consumed = true;
            }
        }

        if (EditorResourceNodeAmountDecRect(editor).leftClicked())
        {
            node.amount = Max(0, node.amount - 100);
            editor.statusText = U"Resource amount: {}"_fmt(node.amount);
            consumed = true;
        }
        if (EditorResourceNodeAmountIncRect(editor).leftClicked())
        {
            node.amount += 100;
            editor.statusText = U"Resource amount: {}"_fmt(node.amount);
            consumed = true;
        }
        if (EditorResourceNodeIncomeDecRect(editor).leftClicked())
        {
            node.incomePerSec = Max(0, node.incomePerSec - 1);
            editor.statusText = U"Resource income: {}"_fmt(node.incomePerSec);
            consumed = true;
        }
        if (EditorResourceNodeIncomeIncRect(editor).leftClicked())
        {
            node.incomePerSec += 1;
            editor.statusText = U"Resource income: {}"_fmt(node.incomePerSec);
            consumed = true;
        }

        if (EditorResourceNodeOneShotRect(editor).leftClicked())
        {
            node.oneShot = !node.oneShot;
            editor.statusText = node.oneShot ? U"Resource mode: one-shot" : U"Resource mode: income";
            consumed = true;
        }

        const RectNumberStepperRects captureTimeStepper = EditorResourceNodeCaptureTimeStepperRects(editor);
        switch (DetectRectNumberStepperInput(captureTimeStepper))
        {
        case RectNumberStepperInputAction::StartValueEdit:
            editor.resourceCaptureTimeEditingIndex = editor.selectedResourceNodeIndex;
            editor.resourceCaptureTimeEditingText = U"{:.1f}"_fmt(node.captureTimeSec);
            editor.resourceCaptureTimeStepMenuIndex = none;
            return true;
        case RectNumberStepperInputAction::CycleStep:
            CycleResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex);
            editor.statusText = U"Capture step set to {}"_fmt(ResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex));
            return true;
        case RectNumberStepperInputAction::OpenStepMenu:
            editor.resourceCaptureTimeStepMenuIndex = editor.selectedResourceNodeIndex;
            editor.resourceCaptureTimeStepMenuPos = Cursor::PosF();
            return true;
        case RectNumberStepperInputAction::Decrement:
        {
            const double step = ApplyTemporaryStepModifier(ResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex));
            node.captureTimeSec = ClampResourceCaptureTimeSec(node.captureTimeSec - step);
            editor.statusText = U"Capture time: {:.1f}s"_fmt(node.captureTimeSec);
            return true;
        }
        case RectNumberStepperInputAction::Increment:
        {
            const double step = ApplyTemporaryStepModifier(ResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex));
            node.captureTimeSec = ClampResourceCaptureTimeSec(node.captureTimeSec + step);
            editor.statusText = U"Capture time: {:.1f}s"_fmt(node.captureTimeSec);
            return true;
        }
        default:
            break;
        }

        if (editor.resourceCaptureTimeEditingIndex == editor.selectedResourceNodeIndex && MouseL.down())
        {
            if (!captureTimeStepper.value.mouseOver())
            {
                if (TryCommitResourceCaptureTimeText(node, editor.resourceCaptureTimeEditingText))
                {
                    editor.statusText = U"Capture time: {:.1f}s"_fmt(node.captureTimeSec);
                }
                else
                {
                    editor.statusText = U"Invalid capture time: {}"_fmt(editor.resourceCaptureTimeEditingText);
                }
                editor.resourceCaptureTimeEditingIndex = -1;
                editor.resourceCaptureTimeEditingText.clear();
                return true;
            }
        }

        if (EditorResourceNodeRemoveRect(editor).leftClicked())
        {
            RemoveSelectedResourceNode(editor);
            return true;
        }

        if (EditorResourceNodePanelRect(editor).mouseOver())
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
        const RectF validationPanel = EditorResourceValidationPanelRect();
        const RectF palettePanel = EditorResourcePalettePanelRect();
        for (int32 i = 0; i < 4; ++i)
        {
            if (EditorResourceNodeFilterRect(i).leftClicked())
            {
                editor.resourceNodeFilterKind = (i == 0) ? -1 : (i - 1);
                const Optional<Point> selectedCell = IsValidSelectedResourceNodeIndex(editor)
                    ? Optional<Point>{ editor.resourceNodes[editor.selectedResourceNodeIndex].cell }
                    : none;
                if (editor.resourceNodeFilterKind >= 0)
                {
                    const ResourceKind targetKind = static_cast<ResourceKind>(editor.resourceNodeFilterKind);
                    editor.resourceNodes.sort_by([targetKind](const ResourceNodeEditData& a, const ResourceNodeEditData& b)
                    {
                        const bool aTarget = (a.kind == targetKind);
                        const bool bTarget = (b.kind == targetKind);
                        if (aTarget != bTarget)
                        {
                            return aTarget;
                        }
                        if (a.cell.y != b.cell.y)
                        {
                            return a.cell.y < b.cell.y;
                        }
                        if (a.cell.x != b.cell.x)
                        {
                            return a.cell.x < b.cell.x;
                        }
                        return static_cast<int32>(a.kind) < static_cast<int32>(b.kind);
                    });
                }
                else
                {
                    SortMapEditorResourceNodes(editor);
                }
                if (selectedCell)
                {
                    if (const Optional<size_t> newSelected = FindResourceNodeAtCell(editor, *selectedCell))
                    {
                        editor.selectedResourceNodeIndex = static_cast<int32>(*newSelected);
                    }
                    else
                    {
                        editor.selectedResourceNodeIndex = -1;
                    }
                }
                editor.resourceNodeListScroll = 0.0;
                editor.statusText = U"Resource filter + sort: {}"_fmt((i == 0) ? U"All" : ResourceKindLabel(static_cast<ResourceKind>(i - 1)));
                return true;
            }
        }

        if (viewport.mouseOver())
        {
            const double wheel = Mouse::Wheel();
            if (wheel != 0.0)
            {
                const double maxScroll = Max(0.0, EditorResourceNodeListContentHeight(editor) - viewport.h);
                editor.resourceNodeListScroll = Clamp(editor.resourceNodeListScroll - wheel * 42.0, 0.0, maxScroll);
                return true;
            }
        }

        if (validationPanel.mouseOver() && Mouse::Wheel() != 0.0)
        {
            return true;
        }

        if (palettePanel.mouseOver() && Mouse::Wheel() != 0.0)
        {
            return true;
        }

        if (!panel.mouseOver())
        {
            return false;
        }

        int32 visibleIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(editor.resourceNodes.size()); ++i)
        {
            if (!PassesResourceNodeFilter(editor, editor.resourceNodes[i].kind))
            {
                continue;
            }
            const RectF row = EditorResourceNodeListRowRect(viewport, visibleIndex, editor.resourceNodeListScroll);
            ++visibleIndex;
            if (viewport.intersects(row) && row.leftClicked())
            {
                editor.selectedResourceNodeIndex = i;
                editor.statusText = U"Selected resource node: {}"_fmt(ResourceKindLabel(editor.resourceNodes[i].kind));
            }
        }

        return true;
    }
}
