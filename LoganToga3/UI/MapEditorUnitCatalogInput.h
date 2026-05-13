#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"

namespace LT3
{
    inline void ChangeSelectedUnitVisualScale(MapEditorState& editor, UnitCatalog& catalog, double delta)
    {
        if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
        {
            return;
        }

        UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        entry.visualScale = Math::Round(Clamp(entry.visualScale + delta, 0.25, 3.0) * 100.0) / 100.0;
        SaveUnitCatalogToml(catalog, editor.statusText);
        editor.unitCatalogDirty = true;
    }

    inline void ChangeSelectedUnitMove(MapEditorState& editor, UnitCatalog& catalog, int32 delta)
    {
        if (editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
        {
            return;
        }

        UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        entry.move = Clamp(entry.move + delta, 0, 2000);
        SaveUnitCatalogToml(catalog, editor.statusText);
        editor.unitCatalogDirty = true;
    }

    inline bool ProcessUnitCatalogEditorInput(MapEditorState& editor, UnitCatalog& catalog)
    {
        bool consumed = false;
        if (editor.showUnitList && EditorUnitListPanelRect().mouseOver())
        {
            const RectF viewport = EditorUnitListViewportRect();
            const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
            editor.unitListScroll = Clamp(editor.unitListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
            consumed = true;

            for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
            {
                const RectF row = EditorUnitListRowRect(viewport, i, editor.unitListScroll);
                if (viewport.intersects(row) && row.leftClicked())
                {
                    editor.selectedUnitCatalogIndex = i;
                    editor.showUnitParameterEditor = true;
                    editor.statusText = U"Editing unit: {}"_fmt(catalog.entries[i].tag);
                }
            }
        }

        if (editor.showUnitParameterEditor && EditorUnitParameterPanelRect().mouseOver())
        {
            consumed = true;
            if (EditorUnitParameterCloseRect().leftClicked())
            {
                editor.showUnitParameterEditor = false;
            }
            if (EditorUnitScaleDecrementRect().leftClicked())
            {
                ChangeSelectedUnitVisualScale(editor, catalog, -0.05);
            }
            if (EditorUnitScaleIncrementRect().leftClicked())
            {
                ChangeSelectedUnitVisualScale(editor, catalog, 0.05);
            }
            if (EditorUnitScaleResetRect().leftClicked())
            {
                if (0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size()))
                {
                    catalog.entries[editor.selectedUnitCatalogIndex].visualScale = 1.0;
                    SaveUnitCatalogToml(catalog, editor.statusText);
                    editor.unitCatalogDirty = true;
                }
            }
            if (EditorUnitMoveDecrementRect().leftClicked())
            {
                ChangeSelectedUnitMove(editor, catalog, -25);
            }
            if (EditorUnitMoveIncrementRect().leftClicked())
            {
                ChangeSelectedUnitMove(editor, catalog, 25);
            }
            if (EditorUnitMoveUseSpeedRect().leftClicked())
            {
                if (0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size()))
                {
                    catalog.entries[editor.selectedUnitCatalogIndex].move = 0;
                    SaveUnitCatalogToml(catalog, editor.statusText);
                    editor.unitCatalogDirty = true;
                }
            }
        }

        return consumed;
    }
}
