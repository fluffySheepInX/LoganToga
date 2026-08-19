# pragma once
# include <Siv3D.hpp>
# include "DefinitionStores.h"
# include "UnitCatalog.h"
# include "Loaders/SkillDefLoader.h"
# include "Loaders/UnitDefLoader.h"
# include "Loaders/ResourceDefLoader.h"
# include "Loaders/BuildActionDefLoader.h"
# include "Loaders/AiProfileDefLoader.h"
# include "ModDefinitionPaths.h"

namespace LT3
{
    // ロード済みの定義間参照を検証し、未解決参照を警告へ追加する。
    inline void ValidateDefinitionReferences(DefinitionStores& defs)
    {
        for (const auto& skill : defs.skills)
        {
            for (const auto& cost : skill.resourceCosts)
            {
                if (!defs.resourceByTag.contains(NormalizeDefinitionTag(cost.resourceTag)))
                {
                    defs.addLoadWarning(U"Unresolved resource tag '{}' for skill '{}'"_fmt(cost.resourceTag, skill.tag));
                }
            }
        }

        for (const auto& profile : defs.aiProfiles)
        {
            for (const auto& unitTag : profile.initialUnits)
            {
                if (!defs.unitByTag.contains(NormalizeDefinitionTag(unitTag)))
                {
                    defs.addLoadWarning(U"Unresolved initial unit tag '{}' for AI profile '{}'"_fmt(unitTag, profile.tag));
                }
            }

            for (const auto& unitWeight : profile.unitWeights)
            {
                if (!defs.unitByTag.contains(NormalizeDefinitionTag(unitWeight.unitTag)))
                {
                    defs.addLoadWarning(U"Unresolved unit tag '{}' for AI profile '{}'"_fmt(unitWeight.unitTag, profile.tag));
                }
            }

            for (const auto& buildPriority : profile.buildPriorities)
            {
                if (!defs.buildActionByTag.contains(NormalizeDefinitionTag(buildPriority.actionTag)))
                {
                    defs.addLoadWarning(U"Unresolved build action tag '{}' for AI profile '{}'"_fmt(buildPriority.actionTag, profile.tag));
                }
            }
        }
    }

    inline DefinitionStores CreateDefaultDefinitions(const UnitCatalog& unitCatalog)
    {
        DefinitionStores defs;
        defs.loadWarnings.clear();
        LoadSkillDefinitions(defs);
        LoadUnitDefinitions(defs, unitCatalog);
        LoadResourceDefinitions(defs);
        LoadBuildActionDefinitions(defs);
        LoadAiProfileDefinitions(defs);
        ValidateDefinitionReferences(defs);
        return defs;
    }

    inline DefinitionStores CreateDefaultDefinitions()
    {
        return CreateDefaultDefinitions(LoadUnitCatalog());
    }

    inline DefinitionStores CreateDefaultDefinitions(const UnitCatalog& unitCatalog, const ModContext& mod)
    {
        DefinitionStores defs;
        defs.loadWarnings.clear();
        LoadSkillDefinitions(defs, ResolveModDefinitionPath(mod, U"070_Scenario/InfoSkill/Skills.toml"));
        LoadUnitDefinitions(defs, unitCatalog);
        LoadResourceDefinitions(defs, ResolveModDefinitionPath(mod, U"070_Scenario/InfoResource/Resources.toml"));
        LoadBuildActionDefinitions(defs, ResolveModDefinitionPath(mod, U"070_Scenario/InfoBuildMenu/BuildMenu.toml"));
        LoadAiProfileDefinitions(defs, ResolveModDefinitionPath(mod, U"070_Scenario/InfoAI/AiProfiles.toml"));
        ValidateDefinitionReferences(defs);
        return defs;
    }
}
