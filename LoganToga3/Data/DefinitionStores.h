#pragma once
# include <Siv3D.hpp>
# include "IdTypes.h"

namespace LT3
{
    struct SkillDef
    {
        String tag;
        String name;
        double range = 120.0;
        double cooldownSec = 0.8;
        int32 damage = 8;
        ColorF color = Palette::White;
    };

    struct UnitDef
    {
        String tag;
        String name;
        UnitRole role = UnitRole::Soldier;
        int32 hp = 50;
        int32 attack = 6;
        int32 defense = 0;
        double speed = 80.0;
        double radius = 14.0;
        int32 priceGold = 0;
        int32 gatherPower = 0;
        SkillDefId skill = InvalidSkillDefId;
        ColorF color = Palette::White;
        double visualScale = 1.0;
    };

    struct BuildActionDef
    {
        String tag;
        String ownerTag;
        String name;
        String description;
        String icon;
        String category;
        Array<String> requirements;
        String spawnTag;
        BuildActionResultType resultType = BuildActionResultType::None;
        UnitDefId spawnUnit = InvalidUnitDefId;
        int32 costGold = 0;
        int32 createCount = 1;
        double buildTimeSec = 0.0;
        bool isMove = false;
    };

    struct ResourceDef
    {
        String tag;
        String name;
        ResourceKind kind = ResourceKind::Gold;
        ColorF color = Palette::Gold;
        int32 initialAmount = 100;
    };

    struct DefinitionStores
    {
        Array<SkillDef> skills;
        Array<UnitDef> units;
        Array<BuildActionDef> buildActions;
        Array<ResourceDef> resources;
        HashTable<String, SkillDefId> skillByTag;
        HashTable<String, UnitDefId> unitByTag;
        HashTable<String, BuildActionDefId> buildActionByTag;
        HashTable<String, ResourceDefId> resourceByTag;

        SkillDefId addSkill(const SkillDef& def)
        {
            const SkillDefId id = static_cast<SkillDefId>(skills.size());
            skills << def;
            skillByTag[def.tag] = id;
            return id;
        }

        UnitDefId addUnit(const UnitDef& def)
        {
            const UnitDefId id = static_cast<UnitDefId>(units.size());
            units << def;
            unitByTag[def.tag] = id;
            return id;
        }

        BuildActionDefId addBuildAction(const BuildActionDef& def)
        {
            const BuildActionDefId id = static_cast<BuildActionDefId>(buildActions.size());
            buildActions << def;
            buildActionByTag[def.tag] = id;
            return id;
        }

        ResourceDefId addResource(const ResourceDef& def)
        {
            const ResourceDefId id = static_cast<ResourceDefId>(resources.size());
            resources << def;
            resourceByTag[def.tag] = id;
            return id;
        }
    };
}
