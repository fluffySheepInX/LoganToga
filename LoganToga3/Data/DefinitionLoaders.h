# pragma once
# include <Siv3D.hpp>
# include "DefinitionStores.h"
# include "UnitCatalog.h"
# include "Loaders/SkillDefLoader.h"
# include "Loaders/UnitDefLoader.h"
# include "Loaders/ResourceDefLoader.h"
# include "Loaders/BuildActionDefLoader.h"

namespace LT3
{
    inline DefinitionStores CreateDefaultDefinitions(const UnitCatalog& unitCatalog)
    {
        DefinitionStores defs;
        LoadSkillDefinitions(defs);
        LoadUnitDefinitions(defs, unitCatalog);
        LoadResourceDefinitions(defs);
        LoadBuildActionDefinitions(defs);
        return defs;
    }

    inline DefinitionStores CreateDefaultDefinitions()
    {
        return CreateDefaultDefinitions(LoadUnitCatalog());
    }
}
