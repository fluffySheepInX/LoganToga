#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
    inline bool ProcessMapEditorInput(MapEditorState& editor, const Vec2& screenMouse)
    {
        bool consumed = false;
        if (EditorToolbarButtonRect(editor, 0).leftClicked())
        {
            editor.enabled = !editor.enabled;
            editor.statusText = editor.enabled ? U"Map editor ON" : U"Map editor OFF";
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 1).leftClicked())
        {
            SaveMapEditorToml(editor);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 2).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth - 1, editor.mapHeight);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 3).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth + 1, editor.mapHeight);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 4).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight - 1);
            consumed = true;
        }
        if (editor.enabled && EditorToolbarButtonRect(editor, 5).leftClicked())
        {
            ResizeMapEditorCells(editor, editor.mapWidth, editor.mapHeight + 1);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 6).leftClicked())
        {
            editor.showUnitList = !editor.showUnitList;
            editor.statusText = editor.showUnitList ? U"Unit List ON" : U"Unit List OFF";
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 7).leftClicked())
        {
            editor.showDebugInfo = !editor.showDebugInfo;
            editor.statusText = editor.showDebugInfo ? U"Debug Info ON" : U"Debug Info OFF";
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarButtonRect(editor, 8).leftClicked())
        {
            editor.uiLayoutEditEnabled = !editor.uiLayoutEditEnabled;
            editor.uiLayoutDraggingSelectedInfo = false;
            editor.uiLayoutDraggingCommandPanel = false;
            if (!editor.uiLayoutEditEnabled)
            {
                SaveBattleUiLayoutToml(editor);
            }
            else
            {
                editor.statusText = U"UI Layout Edit ON";
            }
            SaveMapEditorToml(editor, false);
            consumed = true;
        }
        if (EditorToolbarRect().mouseOver())
        {
            consumed = true;
        }

        if (editor.uiLayoutEditEnabled)
        {
            if (EditorUiLayoutGridDecrementRect().leftClicked())
            {
                editor.uiLayoutGridSize = Clamp(editor.uiLayoutGridSize - 8, 8, 160);
                SaveBattleUiLayoutToml(editor, false);
                editor.statusText = U"UI Grid: {}"_fmt(editor.uiLayoutGridSize);
                consumed = true;
            }
            if (EditorUiLayoutGridIncrementRect().leftClicked())
            {
                editor.uiLayoutGridSize = Clamp(editor.uiLayoutGridSize + 8, 8, 160);
                SaveBattleUiLayoutToml(editor, false);
                editor.statusText = U"UI Grid: {}"_fmt(editor.uiLayoutGridSize);
                consumed = true;
            }

            const bool showDetail = KeyControl.pressed();
            const RectF infoRect = showDetail ? BattleInfoPanelDetailRect(editor) : BattleInfoPanelCompactRect(editor);
            const RectF commandRect = BattleCommandPanelRect(editor, 3);

            const RectF infoHandle = UiLayoutDragHandleRect(infoRect);
            const RectF commandHandle = UiLayoutDragHandleRect(commandRect);

            if (!editor.uiLayoutDraggingCommandPanel && MouseL.down() && infoHandle.mouseOver())
            {
                editor.uiLayoutDraggingSelectedInfo = true;
                editor.uiLayoutDragOffset = screenMouse - editor.uiSelectedInfoAnchor;
                consumed = true;
            }
            if (!editor.uiLayoutDraggingSelectedInfo && MouseL.down() && commandHandle.mouseOver())
            {
                editor.uiLayoutDraggingCommandPanel = true;
                editor.uiLayoutDragOffset = screenMouse - editor.uiCommandPanelPos;
                consumed = true;
            }

            if (editor.uiLayoutDraggingSelectedInfo && MouseL.pressed())
            {
                const Vec2 snapped = SnapUiLayoutPosition(screenMouse - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
                editor.uiSelectedInfoAnchor.x = Clamp(snapped.x, 0.0, 1600.0 - infoRect.w);
                editor.uiSelectedInfoAnchor.y = Clamp(snapped.y, infoRect.h, 900.0);
                consumed = true;
            }
            if (editor.uiLayoutDraggingCommandPanel && MouseL.pressed())
            {
                const Vec2 snapped = SnapUiLayoutPosition(screenMouse - editor.uiLayoutDragOffset, editor.uiLayoutGridSize);
                editor.uiCommandPanelPos.x = Clamp(snapped.x, 0.0, 1600.0 - commandRect.w);
                editor.uiCommandPanelPos.y = Clamp(snapped.y, 70.0, 900.0 - commandRect.h);
                consumed = true;
            }

            if (MouseL.up())
            {
                const bool wasDragging = editor.uiLayoutDraggingSelectedInfo || editor.uiLayoutDraggingCommandPanel;
                editor.uiLayoutDraggingSelectedInfo = false;
                editor.uiLayoutDraggingCommandPanel = false;
                if (wasDragging)
                {
                    SaveBattleUiLayoutToml(editor, false);
                    editor.statusText = U"UI Layout updated";
                }
            }

            if (infoRect.mouseOver() || commandRect.mouseOver())
            {
                consumed = true;
            }
            if (EditorUiLayoutGridPanelRect().mouseOver())
            {
                consumed = true;
            }
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

        if (KeySpace.pressed())
        {
            return false;
        }

        const Optional<Point> cell = PickMapEditorCell(editor, screenMouse);
        if (!cell)
        {
            return false;
        }

        if (MouseL.pressed() && 0 <= editor.selectedAsset && editor.selectedAsset < static_cast<int32>(editor.assets.size()))
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
        if (MouseR.pressed())
        {
            MapEditorCell& target = editor.cells[MapEditorCellIndex(editor, cell->x, cell->y)];
            target.objectAsset = InvalidMapEditorAsset;
            consumed = true;
        }

        return consumed;
    }
}
