# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void UpdateWinLose(BattleWorld& world, const DefinitionStores& defs)
    {
        if (world.units.size() == 0)
        {
            world.victory = false;
            world.defeat = false;
            return;
        }

        if (world.aiRuntime.battleTimeLimitSec > 0.0 && world.elapsedSec >= world.aiRuntime.battleTimeLimitSec)
        {
            world.victory = false;
            world.defeat = true;
            return;
        }

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
