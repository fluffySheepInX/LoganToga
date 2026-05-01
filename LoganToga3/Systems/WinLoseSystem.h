# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateWinLose(BattleWorld& world, const DefinitionStores& defs)
    {
        bool playerBaseAlive = false;
        bool enemyBaseAlive = false;

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (defs.units[world.units.defId[unit]].role != UnitRole::Base) continue;
            playerBaseAlive |= world.units.faction[unit] == Faction::Player;
            enemyBaseAlive |= world.units.faction[unit] == Faction::Enemy;
        }

        world.victory = !enemyBaseAlive;
        world.defeat = !playerBaseAlive;
    }
}
