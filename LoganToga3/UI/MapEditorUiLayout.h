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
        return RectF{ editor.uiSelectedInfoAnchor.x, editor.uiSelectedInfoAnchor.y - 124.0 + EditorBarTopAnchorOffset(editor, editor.uiSelectedInfoTopAnchor), 282.0, 124.0 };
    }

    inline RectF BattleInfoPanelDetailRect(const MapEditorState& editor)
    {
        return RectF{ editor.uiSelectedInfoAnchor.x, editor.uiSelectedInfoAnchor.y - 340.0 + EditorBarTopAnchorOffset(editor, editor.uiSelectedInfoTopAnchor), 282.0, 340.0 };
    }

    inline RectF BattleInfoPanelMultiRect(const MapEditorState& editor, size_t selectedCount)
    {
        constexpr double rowHeight = 28.0;
        const size_t visibleCount = Min<size_t>(12, selectedCount);
        const bool hasOmitted = (selectedCount > visibleCount);
        const double panelHeight = 42.0 + static_cast<double>(visibleCount) * rowHeight + (hasOmitted ? 26.0 : 12.0);
        const RectF compact = BattleInfoPanelCompactRect(editor);
        return RectF{ compact.x, compact.y, compact.w, panelHeight };
    }

    inline double BattleUiBottomSafeInset()
    {
        return 40.0;
    }

    inline double BattleUiBottomSafeY()
    {
        return Max(0.0, Scene::Height() - BattleUiBottomSafeInset());
    }

    inline RectF BattleCommandPanelRect(const MapEditorState& editor, int32 rows)
    {
        const double panelHeight = Max(112.0, rows * 88.0 + 24.0);
        const double panelY = Min(editor.uiCommandPanelPos.y, Max(0.0, BattleUiBottomSafeY() - panelHeight));
        return RectF{ editor.uiCommandPanelPos.x, panelY + EditorBarTopAnchorOffset(editor, editor.uiCommandPanelTopAnchor), 286.0, panelHeight };
    }

    inline RectF BattleCommandIconRect(const MapEditorState& editor, int32 index, int32 rows)
    {
        constexpr int32 columns = 3;
        const int32 col = index % columns;
        const int32 row = index / columns;
        const Vec2 origin = BattleCommandPanelRect(editor, rows).pos + Vec2{ 22.0, 22.0 };
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
        writer << U"resource_x = " << editor.uiResourcePanelPos.x << U"\n";
        writer << U"resource_y = " << editor.uiResourcePanelPos.y << U"\n";
        writer << U"param_editor_x = " << editor.uiParamEditorPos.x << U"\n";
        writer << U"param_editor_y = " << editor.uiParamEditorPos.y << U"\n";
        writer << U"building_editor_x = " << editor.uiBuildingEditorPos.x << U"\n";
        writer << U"building_editor_y = " << editor.uiBuildingEditorPos.y << U"\n";
        writer << U"info_top_anchor = " << (editor.uiSelectedInfoTopAnchor ? U"true" : U"false") << U"\n";
        writer << U"command_top_anchor = " << (editor.uiCommandPanelTopAnchor ? U"true" : U"false") << U"\n";
        writer << U"resource_top_anchor = " << (editor.uiResourcePanelTopAnchor ? U"true" : U"false") << U"\n";
        writer << U"param_editor_top_anchor = " << (editor.uiParamEditorTopAnchor ? U"true" : U"false") << U"\n";
        writer << U"building_editor_top_anchor = " << (editor.uiBuildingEditorTopAnchor ? U"true" : U"false") << U"\n";

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
        editor.uiResourcePanelPos.x = toml[U"layout.resource_x"].getOr<double>(editor.uiResourcePanelPos.x);
        editor.uiResourcePanelPos.y = toml[U"layout.resource_y"].getOr<double>(editor.uiResourcePanelPos.y);
        editor.uiParamEditorPos.x = toml[U"layout.param_editor_x"].getOr<double>(editor.uiParamEditorPos.x);
        editor.uiParamEditorPos.y = toml[U"layout.param_editor_y"].getOr<double>(editor.uiParamEditorPos.y);
        editor.uiBuildingEditorPos.x = toml[U"layout.building_editor_x"].getOr<double>(editor.uiBuildingEditorPos.x);
        editor.uiBuildingEditorPos.y = toml[U"layout.building_editor_y"].getOr<double>(editor.uiBuildingEditorPos.y);
        editor.uiSelectedInfoTopAnchor = toml[U"layout.info_top_anchor"].getOr<bool>(editor.uiSelectedInfoTopAnchor);
        editor.uiCommandPanelTopAnchor = toml[U"layout.command_top_anchor"].getOr<bool>(editor.uiCommandPanelTopAnchor);
        editor.uiResourcePanelTopAnchor = toml[U"layout.resource_top_anchor"].getOr<bool>(editor.uiResourcePanelTopAnchor);
        editor.uiParamEditorTopAnchor = toml[U"layout.param_editor_top_anchor"].getOr<bool>(editor.uiParamEditorTopAnchor);
        editor.uiBuildingEditorTopAnchor = toml[U"layout.building_editor_top_anchor"].getOr<bool>(editor.uiBuildingEditorTopAnchor);
    }
}
