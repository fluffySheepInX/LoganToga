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

    inline void InitializeAppRuntimeState(AppRuntimeState& runtime, const AppDefinitionState& definitions)
    {
        runtime.world = BattleWorld{};
        SpawnDefaultBattle(runtime.world, definitions.defs);
    }
}
