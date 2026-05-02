#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"

namespace LT3
{
    inline FilePath ResolveBattleUiLayoutTomlPath()
    {
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoUI/BattleUiLayout.toml";
        if (FileSystem::Exists(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoUI/BattleUiLayout.toml";
        if (FileSystem::Exists(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
    }

    inline RectF BattleInfoPanelCompactRect(const MapEditorState& editor)
    {
        return RectF{ editor.uiSelectedInfoAnchor.x, editor.uiSelectedInfoAnchor.y - 96.0, 520.0, 96.0 };
    }

    inline RectF BattleInfoPanelDetailRect(const MapEditorState& editor)
    {
        return RectF{ editor.uiSelectedInfoAnchor.x, editor.uiSelectedInfoAnchor.y - 220.0, 520.0, 220.0 };
    }

    inline RectF BattleCommandPanelRect(const MapEditorState& editor, int32 rows)
    {
        return RectF{ editor.uiCommandPanelPos.x, editor.uiCommandPanelPos.y, 286.0, Max(112.0, rows * 88.0 + 24.0) };
    }

    inline RectF BattleCommandIconRect(const MapEditorState& editor, int32 index)
    {
        constexpr int32 columns = 3;
        const int32 col = index % columns;
        const int32 row = index / columns;
        const Vec2 origin = editor.uiCommandPanelPos + Vec2{ 22.0, 22.0 };
        const Vec2 step{ 88.0, 88.0 };
        return RectF{ origin + Vec2{ col * step.x, row * step.y }, 78.0, 78.0 };
    }

    inline RectF UiLayoutDragHandleRect(const RectF& panelRect)
    {
        return RectF{ panelRect.x + panelRect.w - 24.0, panelRect.y + 6.0, 18.0, 18.0 };
    }

    inline Vec2 SnapUiLayoutPosition(const Vec2& pos, int32 gridSize)
    {
        const int32 safeGrid = Max(8, gridSize);
        return Vec2{
            Math::Round(pos.x / safeGrid) * safeGrid,
            Math::Round(pos.y / safeGrid) * safeGrid
        };
    }

    inline bool SaveBattleUiLayoutToml(MapEditorState& editor, bool updateStatus = true)
    {
        if (editor.uiLayoutPath.isEmpty())
        {
            editor.uiLayoutPath = ResolveBattleUiLayoutTomlPath();
        }

        FileSystem::CreateDirectories(FileSystem::ParentPath(editor.uiLayoutPath));
        TextWriter writer{ editor.uiLayoutPath };
        if (!writer)
        {
            if (updateStatus)
            {
                editor.statusText = U"UI layout save failed: {}"_fmt(editor.uiLayoutPath);
            }
            return false;
        }

        writer << U"[layout]\n";
        writer << U"grid = " << editor.uiLayoutGridSize << U"\n";
        writer << U"info_x = " << editor.uiSelectedInfoAnchor.x << U"\n";
        writer << U"info_bottom = " << editor.uiSelectedInfoAnchor.y << U"\n";
        writer << U"command_x = " << editor.uiCommandPanelPos.x << U"\n";
        writer << U"command_y = " << editor.uiCommandPanelPos.y << U"\n";

        if (updateStatus)
        {
            editor.statusText = U"Saved UI layout: {}"_fmt(editor.uiLayoutPath);
        }
        return true;
    }

    inline void LoadBattleUiLayoutToml(MapEditorState& editor)
    {
        editor.uiLayoutPath = ResolveBattleUiLayoutTomlPath();

        const TOMLReader toml{ editor.uiLayoutPath };
        if (!toml)
        {
            return;
        }

        editor.uiLayoutGridSize = Clamp(toml[U"layout.grid"].getOr<int32>(editor.uiLayoutGridSize), 8, 160);
        editor.uiSelectedInfoAnchor.x = toml[U"layout.info_x"].getOr<double>(editor.uiSelectedInfoAnchor.x);
        editor.uiSelectedInfoAnchor.y = toml[U"layout.info_bottom"].getOr<double>(editor.uiSelectedInfoAnchor.y);
        editor.uiCommandPanelPos.x = toml[U"layout.command_x"].getOr<double>(editor.uiCommandPanelPos.x);
        editor.uiCommandPanelPos.y = toml[U"layout.command_y"].getOr<double>(editor.uiCommandPanelPos.y);
    }
}
