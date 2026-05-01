# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "CameraInputSystem.h"

namespace LT3
{
    inline void StartVisibleBuildCommand(BattleWorld& world, const DefinitionStores& defs, int32 commandIndex)
    {
        int32 visibleIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            if (!CanUseBuildAction(world, defs, world.selection.selected, defs.buildActions[i]))
            {
                continue;
            }

            if (visibleIndex == commandIndex)
            {
                TryStartBuild(world, defs, static_cast<BuildActionDefId>(i));
                return;
            }
            ++visibleIndex;
        }
    }

    inline void HandleBattleInput(BattleWorld& world, const DefinitionStores& defs, const Vec2& worldMouse)
    {
        int32 visibleBuildCommandIndex = 0;
        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            if (!CanUseBuildAction(world, defs, world.selection.selected, defs.buildActions[i]))
            {
                continue;
            }

            if (BuildCommandIconRect(visibleBuildCommandIndex).leftClicked())
            {
                TryStartBuild(world, defs, static_cast<BuildActionDefId>(i));
                return;
            }
            ++visibleBuildCommandIndex;
        }

        if (MouseL.down() && !IsCursorOnBuildCommandUi(world, defs))
        {
            if (const Optional<UnitId> unit = PickUnitAt(world, defs, worldMouse, Faction::Player))
            {
                world.selection.selected = *unit;
            }
            else
            {
                world.selection.selected = InvalidUnitId;
            }
        }

        if (MouseR.down() && !IsCursorOnBuildCommandUi(world, defs) && IsValidUnit(world, world.selection.selected))
        {
            IssueMove(world, world.selection.selected, worldMouse);
        }

        if (Key1.down()) StartVisibleBuildCommand(world, defs, 0);
        if (Key2.down()) StartVisibleBuildCommand(world, defs, 1);
        if (Key3.down()) StartVisibleBuildCommand(world, defs, 2);
    }
}
