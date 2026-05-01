# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline Optional<UnitId> PickUnitAt(const BattleWorld& world, const DefinitionStores& defs, const Vec2& pos, Faction faction)
    {
        for (int32 i = static_cast<int32>(world.units.size()) - 1; i >= 0; --i)
        {
            const UnitId unit = static_cast<UnitId>(i);
            if (!IsValidUnit(world, unit) || world.units.faction[unit] != faction) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (Circle{ world.units.position[unit], def.radius + 6.0 }.intersects(pos))
            {
                return unit;
            }
        }

        return none;
    }

    inline void IssueMove(BattleWorld& world, UnitId unit, const Vec2& destination)
    {
        if (!IsValidUnit(world, unit)) return;
        world.units.targetPosition[unit] = destination;
        world.units.attackTarget[unit] = InvalidUnitId;
        world.units.task[unit] = UnitTask::Moving;
    }

    inline bool TryStartBuild(BattleWorld& world, const DefinitionStores& defs, BuildActionDefId actionId)
    {
        const UnitId builder = world.selection.selected;
        if (!CanStartBuildAction(world, defs, builder, actionId)) return false;

        const BuildActionDef& action = defs.buildActions[actionId];
        world.resources.playerGold -= action.costGold;
        world.units.task[builder] = UnitTask::Building;
        world.units.buildAction[builder] = actionId;
        world.units.buildProgressSec[builder] = 0.0;
        return true;
    }
}
