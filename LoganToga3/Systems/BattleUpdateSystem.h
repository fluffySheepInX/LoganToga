# pragma once
# include <Siv3D.hpp>
# include "BuildSystem.h"
# include "CombatSystem.h"
# include "EnemyDirectorSystem.h"
# include "MovementSystem.h"
# include "ProjectileSystem.h"
# include "ResourceSystem.h"
# include "WinLoseSystem.h"

namespace LT3
{
    inline void UpdateBattleWorld(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        if (world.victory || world.defeat) return;

        world.elapsedSec += dt;
        UpdateEnemyDirector(world, defs, dt);
        UpdateMovement(world, defs, dt);
        UpdateGathering(world, defs, dt);
        UpdateCombat(world, defs, dt);
        UpdateProjectiles(world, defs, dt);
        UpdateBuildQueues(world, defs, dt);
        UpdateWinLose(world, defs);
    }
}
