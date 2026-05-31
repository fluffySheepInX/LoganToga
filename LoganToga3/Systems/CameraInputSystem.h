# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "../UI/BattleRenderer.h"
# include "../UI/MapEditor.h"
# include "../UI/QuarterView.h"
# include "../UI/AiEditor.h"
# include "../UI/SkillEditorCommon.h"

namespace LT3
{
    inline bool IsCursorOnBuildCommandUi(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor)
    {
        const UnitId selectedUnit = GetSelectedUnit(world);
        int32 visibleBuildCommandCount = 0;
        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            if (CanUseBuildAction(world, defs, selectedUnit, defs.buildActions[i]))
            {
                ++visibleBuildCommandCount;
            }
        }

        const int32 visibleBuildCommandRows = (visibleBuildCommandCount + 2) / 3;
        int32 visibleBuildCommandIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            if (!CanUseBuildAction(world, defs, selectedUnit, defs.buildActions[i]))
            {
                continue;
            }

            if (BattleCommandIconRect(mapEditor, visibleBuildCommandIndex, visibleBuildCommandRows).mouseOver())
            {
                return true;
            }
            ++visibleBuildCommandIndex;
        }

        return false;
    }

    inline bool IsCursorOnMapArea(const MapEditorState& mapEditor, const BattleWorld& world, const DefinitionStores& defs, const Optional<Vec2>& logicalScreenMouse = none)
    {
        const auto mouseOver = [&](const RectF& rect)
        {
            return logicalScreenMouse ? rect.intersects(*logicalScreenMouse) : rect.mouseOver();
        };

        if (mouseOver(EditorToolbarRect()))
        {
            return false;
        }
        if (mapEditor.enabled && mouseOver(EditorPalettePanelRect()))
        {
            return false;
        }
        if (mapEditor.showUnitList && mouseOver(EditorUnitListPanelRect()))
        {
            return false;
        }
        if (mapEditor.showBuildingEditor && mouseOver(BuildingEditorPanelWithPosRect(mapEditor)))
        {
            return false;
        }
        if (mapEditor.showCommandEditor && mouseOver(EditorCommandPanelRect()))
        {
            return false;
        }
        if (mapEditor.showDecalEditor && mouseOver(EditorDecalEditorPanelRect(mapEditor)))
        {
            return false;
        }
        if (mapEditor.showSkillEditor && mouseOver(SkillEditorPanelRect()))
        {
            return false;
        }
        if (mapEditor.showUniqueEditor && mouseOver(EditorUniquePanelRect(mapEditor)))
        {
            return false;
        }
        if (mapEditor.showAiEditor && mouseOver(AiEditorPanelRect()))
        {
            return false;
        }
        if (mapEditor.enabled && mouseOver(EditorResourceNodeListPanelRect()))
        {
            return false;
        }
        if (mapEditor.enabled && mouseOver(EditorResourceValidationPanelRect()))
        {
            return false;
        }
        if (mapEditor.enabled && mouseOver(EditorResourcePalettePanelRect()))
        {
            return false;
        }
        if (IsValidSelectedResourceNodeIndex(mapEditor) && mouseOver(EditorResourceNodePanelRect(mapEditor)))
        {
            return false;
        }
        if ((!mapEditor.enabled) && mouseOver(RectF{ 1240, 90, 330, 245 }))
        {
            return false;
        }
        if (IsCursorOnBuildCommandUi(world, defs, mapEditor))
        {
            return false;
        }

        return true;
    }

    inline void UpdateQuarterViewCamera(const MapEditorState& mapEditor, const BattleWorld& world, const DefinitionStores& defs)
    {
        const bool mapHovered = IsCursorOnMapArea(mapEditor, world, defs);
        const bool enableControl = mapHovered;
        UpdateQuarterViewCamera2D(enableControl, mapHovered);
    }
}
