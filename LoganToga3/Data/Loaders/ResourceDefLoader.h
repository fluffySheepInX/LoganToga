#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"

namespace LT3
{
    inline void LoadResourceDefinitions(DefinitionStores& defs)
    {
        defs.resources.clear();
        defs.resourceByTag.clear();

        defs.addResource({ U"gold", U"Gold", ResourceKind::Gold, Palette::Gold, 110 });
    }
}
