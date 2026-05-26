# pragma once
# include <Siv3D.hpp>
# include "../App/BattleNotificationState.h"
# include "BuildSystem.h"
# include "CombatSystem.h"
# include "EnemyDirectorSystem.h"
# include "MovementSystem.h"
# include "PathfindingSystem.h"
# include "ProjectileSystem.h"
# include "ResourceSystem.h"
# include "WinLoseSystem.h"

namespace LT3
{
  inline bool IsBattleFinished(const BattleWorld& world)
    {
        return world.victory || world.defeat;
    }

    inline void AdvanceBattleClock(BattleWorld& world, double dt)
    {
        world.elapsedSec += dt;
    }

    inline void UpdateBattleSimulation(BattleWorld& world, const DefinitionStores& defs, double dt, BattleNotificationRuntimeState* notifications = nullptr)
    {
        if (!world.enemyDirectorPaused)
        {
            UpdateEnemyDirector(world, defs, dt);
        }
        UpdatePathfinding(world, defs, dt);
        UpdateMovement(world, defs, dt);
        UpdateGathering(world, defs, dt, notifications);
        UpdateCombat(world, defs, dt);
        UpdateProjectiles(world, defs, dt);
        UpdateBuildQueues(world, defs, dt, notifications);
        UpdateWinLose(world, defs);
    }

    inline void UpdateBattleWorld(BattleWorld& world, const DefinitionStores& defs, double dt, BattleNotificationRuntimeState* notifications = nullptr)
    {
      if (IsBattleFinished(world)) return;

     AdvanceBattleClock(world, dt);
        UpdateBattleSimulation(world, defs, dt, notifications);
    }
}
