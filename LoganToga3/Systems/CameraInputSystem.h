# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "../UI/BattleRenderer.h"
# include "../UI/MapEditor.h"
# include "../UI/QuarterView.h"

namespace LT3
{
    inline bool IsCursorOnBuildCommandUi(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor)
    {
        const UnitId selectedUnit = GetSelectedUnit(world);
        int32 visibleBuildCommandIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            if (!CanUseBuildAction(world, defs, selectedUnit, defs.buildActions[i]))
            {
                continue;
            }

            if (BattleCommandIconRect(mapEditor, visibleBuildCommandIndex).mouseOver())
            {
                return true;
            }
            ++visibleBuildCommandIndex;
        }

        return false;
    }

    inline bool IsCursorOnMapArea(const MapEditorState& mapEditor, const BattleWorld& world, const DefinitionStores& defs)
    {
        if (EditorToolbarRect().mouseOver())
        {
            return false;
        }
        if (mapEditor.enabled && EditorPalettePanelRect().mouseOver())
        {
            return false;
        }
        if (mapEditor.showUnitList && EditorUnitListPanelRect().mouseOver())
        {
            return false;
        }
        if ((!mapEditor.enabled) && RectF{ 1240, 90, 330, 245 }.mouseOver())
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
        const bool enableControl = mapHovered && (!mapEditor.enabled || KeySpace.pressed());
        UpdateQuarterViewCamera2D(enableControl, mapHovered);
    }
}
