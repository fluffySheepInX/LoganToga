#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionLoaders.h"
# include "../Data/UnitCatalog.h"
# include "../UI/BattleRenderer.h"

namespace LT3
{
    struct AppDefinitionState
    {
        UnitCatalog unitCatalog;
        DefinitionStores defs;
        BattleRenderAssets renderAssets;
    };

    inline AppDefinitionState CreateAppDefinitionState()
    {
        AppDefinitionState state;
        state.unitCatalog = LoadUnitCatalog();
        state.defs = CreateDefaultDefinitions(state.unitCatalog);
        state.renderAssets = BuildBattleRenderAssets(state.unitCatalog);
        return state;
    }
}
