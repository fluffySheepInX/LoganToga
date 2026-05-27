#pragma once
# include <Siv3D.hpp>
# include "MapEditorInput.h"
# include "RectUiHelpers.h"

namespace LT3
{
    inline void DrawResourceKindIcon(const MapEditorState& editor, ResourceKind kind, const Vec2& center, double size)
    {
        const String iconName = (kind == ResourceKind::Trust)
            ? U"resource-trust.png"
            : (kind == ResourceKind::Food) ? U"resource-food.png" : U"resource-gold.png";
        if (editor.resourceIconTextures.contains(iconName))
        {
            editor.resourceIconTextures.at(iconName).resized(size, size).drawAt(center);
        }
        else
        {
            Circle{ center, size * 0.35 }.draw(ResourceKindColor(kind));
        }
    }

    inline void DrawMapEditorResourcePalette(const MapEditorState& editor, const Font& uiFont)
    {
        if (!editor.enabled)
        {
            return;
        }

        const RectF panel = EditorResourcePalettePanelRect();
        const RectF clearAllRect = EditorResourceClearAllRect();
        DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.92 }, ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Resource Place").draw(panel.x + 10.0, panel.y + 6.0, Palette::White);
        clearAllRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, clearAllRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Clear All").drawAt(11, clearAllRect.center(), Palette::White);

        for (int32 i = 0; i < 3; ++i)
        {
            const ResourceKind kind = static_cast<ResourceKind>(i);
            const RectF iconRect = EditorResourcePaletteIconRect(i);
            const bool active = editor.resourcePlacementDragKind && *editor.resourcePlacementDragKind == kind;
            iconRect.draw(active ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
                .drawFrame(2, iconRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
            DrawResourceKindIcon(editor, kind, iconRect.center().movedBy(0, -4), 26.0);
            uiFont(ResourceKindLabel(kind)).drawAt(10, iconRect.center().movedBy(0, 16), ResourceKindColor(kind));
        }
    }

    inline void DrawResourcePlacementGhost(const MapEditorState& editor, const Font& uiFont)
    {
        if (!editor.enabled || !editor.resourcePlacementDragKind)
        {
            return;
        }

        const Vec2 cursorPos = Cursor::PosF();
        Circle{ cursorPos, 24.0 }.draw(ColorF{ 0.08, 0.09, 0.11, 0.72 }).drawFrame(2.0, ColorF{ 1.0, 0.84, 0.0, 0.85 });
        DrawResourceKindIcon(editor, *editor.resourcePlacementDragKind, cursorPos, 28.0);
        uiFont(ResourceKindLabel(*editor.resourcePlacementDragKind)).drawAt(10, cursorPos.movedBy(0, 28), ResourceKindColor(*editor.resourcePlacementDragKind));
    }

    inline void DrawMapEditorResourceNodes(const MapEditorState& editor, const Font& uiFont)
    {
        if (!editor.enabled)
        {
            return;
        }

        const bool hideMarkersByUiHover = EditorResourceNodeListPanelRect().mouseOver()
            || EditorResourceNodeFilterRect(0).mouseOver()
            || EditorResourceNodeFilterRect(1).mouseOver()
            || EditorResourceNodeFilterRect(2).mouseOver()
            || EditorResourceNodeFilterRect(3).mouseOver()
            || EditorResourcePalettePanelRect().mouseOver()
            || EditorResourceValidationPanelRect().mouseOver()
            || (IsValidSelectedResourceNodeIndex(editor) && EditorResourceNodePanelRect().mouseOver());
        if (hideMarkersByUiHover)
        {
            return;
        }

        for (int32 i = 0; i < static_cast<int32>(editor.resourceNodes.size()); ++i)
        {
            const auto& node = editor.resourceNodes[i];
            if (!PassesResourceNodeFilter(editor, node.kind))
            {
                continue;
            }
            const Vec2 center = QuarterTileFaceCenterScreen(MapEditorCellCenter(node.cell.x, node.cell.y));
            const ColorF color = ResourceKindColor(node.kind);
            const bool selected = editor.selectedResourceNodeIndex == i;
            Circle{ center, 18.0 }.draw(ColorF{ color, 0.85 }).drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1, 1, 1, 0.25 });
            DrawResourceKindIcon(editor, node.kind, center, 22.0);
            uiFont(U"{}"_fmt(node.incomePerSec)).drawAt(10, center.movedBy(0, 26), Palette::Black);
            uiFont(ResourceKindLabel(node.kind)).drawAt(10, center.movedBy(0, 8), color);
        }
    }

    inline void DrawMapEditorResourceNodeList(const MapEditorState& editor, const Font& uiFont)
    {
        if (!editor.enabled)
        {
            return;
        }

        const RectF panel = EditorResourceNodeListPanelRect();
        const RectF viewport = EditorResourceNodeListViewportRect();
        DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.92 }, ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"Resource Nodes").draw(panel.x + 12.0, panel.y + 8.0, Palette::White);
        const String nodeCountText = U"{}"_fmt(editor.resourceNodes.size());
        const RectF nodeCountRegion{ panel.x + 120.0, panel.y + 8.0, panel.w - 132.0, 20.0 };
        uiFont(nodeCountText).draw(Arg::rightCenter = nodeCountRegion.rightCenter(), Palette::Gold);

        viewport.draw(ColorF{ 0, 0, 0, 0.08 });
        const double viewportBottom = viewport.y + viewport.h;
        int32 visibleIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(editor.resourceNodes.size()); ++i)
        {
            const ResourceNodeEditData& node = editor.resourceNodes[i];
            if (!PassesResourceNodeFilter(editor, node.kind))
            {
                continue;
            }
            const RectF row = EditorResourceNodeListRowRect(viewport, visibleIndex, editor.resourceNodeListScroll);
            ++visibleIndex;
            if (!((viewport.y <= row.y) && ((row.y + row.h) <= viewportBottom)))
            {
                continue;
            }

            const bool selected = editor.selectedResourceNodeIndex == i;
            DrawRectListRow(row, selected);
            DrawResourceKindIcon(editor, node.kind, Vec2{ row.x + 18.0, row.y + 23.0 }, 18.0);
            uiFont(U"{} ({}, {})"_fmt(ResourceKindLabel(node.kind), node.cell.x, node.cell.y)).draw(12, row.x + 30.0, row.y + 6.0, Palette::White);
            const String nodeMode = node.oneShot ? U"one-shot" : U"income";
            uiFont(U"amount:{}  {}:{}  cap:{:.1f}s"_fmt(node.amount, nodeMode, node.incomePerSec, node.captureTimeSec)).draw(11, row.x + 30.0, row.y + 24.0, Palette::Lightgray);
        }

        const double contentHeight = EditorResourceNodeListContentHeight(editor);
        DrawRectVerticalScrollbar(viewport, contentHeight, editor.resourceNodeListScroll, ColorF{ 1, 1, 1, 0.08 }, ColorF{ 1.0, 0.84, 0.0, 0.70 }, 6.0, 4.0, 32.0);

        const Array<String> filterLabels = { U"All", U"Gold", U"Trust", U"Food" };
        for (int32 i = 0; i < 4; ++i)
        {
            const RectF filterRect = EditorResourceNodeFilterRect(i);
            const bool active = (i == 0) ? (editor.resourceNodeFilterKind < 0) : (editor.resourceNodeFilterKind == (i - 1));
            DrawRectListRow(filterRect, active);
            uiFont(filterLabels[i]).drawAt(11, filterRect.center(), active ? Palette::White : Palette::Lightgray);
        }
    }

    inline void DrawMapEditorResourceNodeEditor(const MapEditorState& editor, const Font& uiFont)
    {
        if (!IsValidSelectedResourceNodeIndex(editor))
        {
            return;
        }

        const ResourceNodeEditData& node = editor.resourceNodes[editor.selectedResourceNodeIndex];
        const RectF panel = EditorResourceNodePanelRect(editor);
        const RectF closeRect = EditorResourceNodeCloseRect(editor);
        const RectF removeRect = EditorResourceNodeRemoveRect(editor);
        DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.94 }, ColorF{ 1, 1, 1, 0.18 });
        DrawRectPanelTitle(panel, U"Resource Node Editor", uiFont);
        DrawRectPanelCloseButton(closeRect, uiFont, 18);
        if (editor.uiLayoutEditEnabled)
        {
            DrawUiLayoutDragHandleOnly(EditorResourceNodeDragHandleRect(editor), editor.uiLayoutDraggingResourceNodeEditor, uiFont, 11);
        }

        uiFont(U"Cell: ({}, {})"_fmt(node.cell.x, node.cell.y)).draw(13, panel.x + 24.0, panel.y + 48.0, Palette::Lightgray);
        uiFont(U"Type").draw(13, panel.x + 24.0, panel.y + 74.0, Palette::Gold);

        for (int32 i = 0; i < 3; ++i)
        {
            const ResourceKind kind = static_cast<ResourceKind>(i);
            const RectF kindRect = EditorResourceNodeKindRect(editor, i);
            const bool active = node.kind == kind;
            DrawRectTabButton(kindRect, ResourceKindLabel(kind), active, uiFont, 13);
            const ColorF textColor = active ? ColorF{ Palette::White } : ResourceKindColor(kind);
            uiFont(ResourceKindLabel(kind)).drawAt(13, kindRect.center(), textColor);
        }

        const RectF amountDecRect = EditorResourceNodeAmountDecRect(editor);
        const RectF amountIncRect = EditorResourceNodeAmountIncRect(editor);
        const RectF amountRowRect{ amountDecRect.x, amountDecRect.y - 2.0, amountIncRect.x + amountIncRect.w - amountDecRect.x, amountDecRect.h + 4.0 };
        DrawRectValueAdjustRow(amountRowRect, U"Amount", U"{}"_fmt(node.amount), amountDecRect, amountIncRect, uiFont);

        const RectF incomeDecRect = EditorResourceNodeIncomeDecRect(editor);
        const RectF incomeIncRect = EditorResourceNodeIncomeIncRect(editor);
        const RectF incomeRowRect{ incomeDecRect.x, incomeDecRect.y - 2.0, incomeIncRect.x + incomeIncRect.w - incomeDecRect.x, incomeDecRect.h + 4.0 };
        DrawRectValueAdjustRow(incomeRowRect, U"Income / sec", U"{}"_fmt(node.incomePerSec), incomeDecRect, incomeIncRect, uiFont);

        DrawRectCheckRow(EditorResourceNodeOneShotRect(editor), U"一回だけ", node.oneShot, uiFont, 12);

        const RectNumberStepperRects captureTimeStepper = EditorResourceNodeCaptureTimeStepperRects(editor);
        uiFont(U"占領時間").draw(13, panel.x + 24.0, captureTimeStepper.value.y - 18.0, Palette::Gold);
        DrawRectNumberStepper(captureTimeStepper,
            (editor.resourceCaptureTimeEditingIndex == editor.selectedResourceNodeIndex)
                ? editor.resourceCaptureTimeEditingText
                : U"{:.1f}s"_fmt(node.captureTimeSec),
            U"{}"_fmt(ResourceCaptureTimeStep(editor, editor.selectedResourceNodeIndex)),
            editor.resourceCaptureTimeEditingIndex == editor.selectedResourceNodeIndex,
            editor.resourceCaptureTimeStepMenuIndex && *editor.resourceCaptureTimeStepMenuIndex == editor.selectedResourceNodeIndex,
            true,
            uiFont);

        DrawRectIconButton(removeRect, U"Remove", uiFont, 13, ColorF{ 0.12, 0.05, 0.05, 0.95 }, 1.0, Palette::White);

        if (editor.resourceCaptureTimeStepMenuIndex && *editor.resourceCaptureTimeStepMenuIndex == editor.selectedResourceNodeIndex)
        {
            const Array<double>& steps = ResourceCaptureTimeStepOptions();
            const Vec2 menuPos = editor.resourceCaptureTimeStepMenuPos;
            const RectF menuRect{ menuPos.x, menuPos.y, 88.0, 8.0 + static_cast<double>(steps.size()) * 22.0 };
            menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
            for (int32 i = 0; i < static_cast<int32>(steps.size()); ++i)
            {
                const RectF item{ menuRect.x + 4.0, menuRect.y + 4.0 + i * 22.0, menuRect.w - 8.0, 20.0 };
                item.draw(item.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
                uiFont(U"step {}"_fmt(steps[i])).draw(11, item.x + 6.0, item.y + 2.0, Palette::White);
            }
        }
    }

    inline void DrawMapEditorResourceValidation(const MapEditorState& editor, const Font& uiFont)
    {
        if (!editor.enabled)
        {
            return;
        }

        const Array<String> issues = ValidateMapEditorResourceNodes(editor);
        const RectF panel = EditorResourceValidationPanelRect();
        DrawRectPanelFrame(panel, ColorF{ 0.02, 0.03, 0.045, 0.92 }, ColorF{ 1, 1, 1, 0.16 });
        DrawRectPanelTitle(panel, U"Resource Validation", uiFont, 12.0, 8.0);
        if (issues.isEmpty())
        {
            uiFont(U"No issues").draw(panel.x + 12.0, panel.y + 36.0, Palette::Lime);
            return;
        }

        uiFont(U"{} issues"_fmt(issues.size())).draw(panel.x + 132.0, panel.y + 8.0, Palette::Orange);
        const size_t displayCount = Min<size_t>(6, issues.size());
        for (size_t i = 0; i < displayCount; ++i)
        {
            uiFont(issues[i]).draw(11, panel.x + 12.0, panel.y + 36.0 + static_cast<double>(i) * 22.0, Palette::Lightgray);
        }
    }

}
