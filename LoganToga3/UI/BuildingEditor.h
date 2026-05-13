#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/UnitCatalog.h"

namespace LT3
{
    inline RectF BuildingEditorPanelRect()
    {
        return RectF{ 700, 404, 360, 332 };
    }

    inline RectF BuildingEditorCloseRect()
    {
        const RectF panel = BuildingEditorPanelRect();
        return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
    }

    inline RectF BuildingEditorPreviewRect()
    {
        const RectF panel = BuildingEditorPanelRect();
        return RectF{ panel.x + 24.0, panel.y + 50.0, panel.w - 48.0, 118.0 };
    }

    inline RectF BuildingEditorAxisButtonRect(int32 axisIndex, int32 buttonIndex)
    {
        const RectF panel = BuildingEditorPanelRect();
        return RectF{ panel.x + 92.0 + buttonIndex * 54.0, panel.y + 190.0 + axisIndex * 56.0, 48.0, 40.0 };
    }

    inline RectF BuildingEditorResetRect()
    {
        const RectF panel = BuildingEditorPanelRect();
        return RectF{ panel.x + panel.w - 122.0, panel.y + panel.h - 42.0, 98.0, 28.0 };
    }

    inline HashTable<FilePath, Texture>& BuildingEditorTextureCache()
    {
        static HashTable<FilePath, Texture> cache;
        return cache;
    }

    inline bool HasSelectedCatalogEntry(const MapEditorState& editor, const UnitCatalog& catalog)
    {
        return 0 <= editor.selectedUnitCatalogIndex
            && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size());
    }

    inline void ChangeSelectedUnitVisualOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
    {
        if (!HasSelectedCatalogEntry(editor, catalog))
        {
            return;
        }

        UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        entry.visualOffset.x = Clamp(entry.visualOffset.x + delta.x, -128, 128);
        entry.visualOffset.y = Clamp(entry.visualOffset.y + delta.y, -128, 128);
        SaveUnitCatalogToml(catalog, editor.statusText);
        editor.unitCatalogDirty = true;
    }

    inline void ResetSelectedUnitVisualOffset(MapEditorState& editor, UnitCatalog& catalog)
    {
        if (!HasSelectedCatalogEntry(editor, catalog))
        {
            return;
        }

        UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        entry.visualOffset = Point{ 0, 0 };
        SaveUnitCatalogToml(catalog, editor.statusText);
        editor.unitCatalogDirty = true;
    }

    inline bool ProcessBuildingEditorInput(MapEditorState& editor, UnitCatalog& catalog)
    {
        if (!editor.showBuildingEditor)
        {
            return false;
        }

        const RectF panel = BuildingEditorPanelRect();
        if (!panel.mouseOver())
        {
            return false;
        }

        if (BuildingEditorCloseRect().leftClicked())
        {
            editor.showBuildingEditor = false;
            editor.statusText = U"BuildingEditor OFF";
            return true;
        }

        if (!HasSelectedCatalogEntry(editor, catalog))
        {
            return true;
        }

        if (BuildingEditorAxisButtonRect(0, 0).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ -4, 0 });
        }
        if (BuildingEditorAxisButtonRect(0, 1).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ -1, 0 });
        }
        if (BuildingEditorAxisButtonRect(0, 2).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 1, 0 });
        }
        if (BuildingEditorAxisButtonRect(0, 3).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 4, 0 });
        }

        if (BuildingEditorAxisButtonRect(1, 0).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, -4 });
        }
        if (BuildingEditorAxisButtonRect(1, 1).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, -1 });
        }
        if (BuildingEditorAxisButtonRect(1, 2).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, 1 });
        }
        if (BuildingEditorAxisButtonRect(1, 3).leftClicked())
        {
            ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, 4 });
        }

        if (BuildingEditorResetRect().leftClicked())
        {
            ResetSelectedUnitVisualOffset(editor, catalog);
        }

        return true;
    }

    inline void DrawBuildingEditorButton(const RectF& rect, StringView text, const Font& uiFont)
    {
        rect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        uiFont(text).drawAt(16, rect.center(), Palette::White);
    }

    inline void DrawBuildingEditor(MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
    {
        if (!editor.showBuildingEditor)
        {
            return;
        }

        const RectF panel = BuildingEditorPanelRect();
        const RectF closeRect = BuildingEditorCloseRect();
        const RectF previewRect = BuildingEditorPreviewRect();

        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
        uiFont(U"BuildingEditor").draw(720, 420, Palette::White);
        closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 }).drawFrame(1, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
        uiFont(U"×").drawAt(18, closeRect.center(), Palette::White);
        previewRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });

        if (!HasSelectedCatalogEntry(editor, catalog))
        {
            uiFont(U"Unit List から対象を選択してください").draw(12, panel.x + 24.0, panel.y + 182.0, Palette::Lightgray);
            return;
        }

        const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
        uiFont(entry.name).draw(14, panel.x + 24.0, panel.y + 178.0, Palette::White);
        uiFont(U"tag:{}  image:{}"_fmt(entry.tag, entry.image)).draw(11, panel.x + 24.0, panel.y + 200.0, Palette::Lightgray);
        uiFont(U"Offset X:{}  Y:{}"_fmt(entry.visualOffset.x, entry.visualOffset.y)).draw(13, panel.x + 24.0, panel.y + 224.0, Palette::Gold);

        const Vec2 anchor{ previewRect.center().x, previewRect.y + previewRect.h * 0.72 };
        Line{ previewRect.x + 14.0, anchor.y, previewRect.x + previewRect.w - 14.0, anchor.y }.draw(1.0, ColorF{ 0.20, 0.65, 1.0, 0.35 });
        Line{ anchor.x, previewRect.y + 12.0, anchor.x, previewRect.y + previewRect.h - 12.0 }.draw(1.0, ColorF{ 0.20, 0.65, 1.0, 0.35 });
        Circle{ anchor, 3.0 }.draw(ColorF{ 1.0, 0.84, 0.0, 0.95 });

        const FilePath imagePath = ResolveCatalogVisualPath(entry.kind, entry.image);
        if (!imagePath.isEmpty() && FileSystem::Exists(imagePath))
        {
            auto& textureCache = BuildingEditorTextureCache();
            if (!textureCache.contains(imagePath))
            {
                textureCache.emplace(imagePath, Texture{ imagePath });
            }

            const Texture& texture = textureCache.at(imagePath);
            const double fitScale = Min((previewRect.w - 48.0) / Max(1.0, static_cast<double>(texture.width())), (previewRect.h - 28.0) / Max(1.0, static_cast<double>(texture.height())));
            const double drawScale = Min(fitScale, 2.0 * entry.visualScale);
            const Vec2 previewOffset = Vec2{ static_cast<double>(entry.visualOffset.x), static_cast<double>(entry.visualOffset.y) } * drawScale;
            const TextureRegion previewTexture = texture.scaled(drawScale);
            if (entry.kind.lowercased() == U"building")
            {
                previewTexture.draw(Arg::bottomCenter = anchor + previewOffset);
            }
            else
            {
                previewTexture.drawAt(anchor + previewOffset);
            }
        }
        else
        {
            RectF{ Arg::center = anchor, 92.0, 72.0 }.draw(ColorF{ 0.18, 0.18, 0.20 }).drawFrame(1, ColorF{ 1, 1, 1, 0.12 });
            uiFont(U"image n/a").drawAt(12, anchor, Palette::Lightgray);
        }

        uiFont(U"X").draw(13, panel.x + 24.0, panel.y + 258.0, Palette::White);
        uiFont(U"Y").draw(13, panel.x + 24.0, panel.y + 314.0, Palette::White);
        uiFont(U"step: -4 -1 +1 +4").draw(11, panel.x + 24.0, panel.y + panel.h - 36.0, Palette::Lightgray);

        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(0, 0), U"-4", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(0, 1), U"-1", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(0, 2), U"+1", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(0, 3), U"+4", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(1, 0), U"-4", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(1, 1), U"-1", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(1, 2), U"+1", uiFont);
        DrawBuildingEditorButton(BuildingEditorAxisButtonRect(1, 3), U"+4", uiFont);
        DrawBuildingEditorButton(BuildingEditorResetRect(), U"Reset", uiFont);
    }
}
