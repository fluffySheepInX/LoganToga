# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateGathering(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.faction[unit] != Faction::Player) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (def.gatherPower <= 0) continue;

            for (size_t node = 0; node < world.resourceNodes.amount.size(); ++node)
            {
                if (world.resourceNodes.amount[node] <= 0) continue;
                if (world.units.position[unit].distanceFrom(world.resourceNodes.position[node]) > 34.0) continue;

                const int32 gathered = Min(world.resourceNodes.amount[node], Max(1, static_cast<int32>(def.gatherPower * dt)));
                world.resourceNodes.amount[node] -= gathered;
                world.resources.playerGold += gathered;
            }
        }
    }
}
