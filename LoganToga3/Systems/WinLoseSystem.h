# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    // 拠点の生存状況から排他的な勝敗状態を確定する。
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
        world.defeat = !world.victory && !playerBaseAlive;
    }
}
