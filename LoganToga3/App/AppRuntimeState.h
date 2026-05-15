#pragma once
# include <Siv3D.hpp>
# include "AppDefinitionState.h"
# include "../Systems/BattleSystems.h"

namespace LT3
{
    struct AppRuntimeState
    {
        BattleWorld world;
    };

    inline void ResetBattleRuntimeState(AppRuntimeState& runtime, const DefinitionStores& defs, bool enemyDirectorPaused)
    {
        runtime.world = BattleWorld{};
        SpawnDefaultBattle(runtime.world, defs);
        runtime.world.enemyDirectorPaused = enemyDirectorPaused;
    }

    inline void InitializeAppRuntimeState(AppRuntimeState& runtime, const AppDefinitionState& definitions)
    {
        ResetBattleRuntimeState(runtime, definitions.defs, false);
    }
}
