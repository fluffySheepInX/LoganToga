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
        int32 visionRadiusCells = 6;
        SkillDefId skill = InvalidSkillDefId;
        ColorF color = Palette::White;
        double visualScale = 1.0;
        String classBuild;
        String classTag;
        bool blocksTileMovement = false;
    };

    struct BuildActionDef
    {
        String tag;
        String id;
        String ownerTag;
        Array<String> ownerTags;
        String name;
        String description;
        String icon;
        String lineIconHorizontal;
        String lineIconDiagUpRight;
        String lineIconDiagUpLeft;
        String category;
        Array<String> requirements;
        String spawnTag;
        Array<String> spawnTags;
        String resultTag;
        BuildActionResultType resultType = BuildActionResultType::None;
        UnitDefId spawnUnit = InvalidUnitDefId;
        Array<UnitDefId> spawnUnits;
        int32 costGold = 0;
        int32 createCount = 1;
        double buildTimeSec = 0.0;
        bool isMove = false;
        BuildPlacementMode placementMode = BuildPlacementMode::Point;
        BuildLineAxisMode lineAxisMode = BuildLineAxisMode::Auto;
        int32 lineThicknessCells = 1;
        int32 maxLineCells = 12;
        bool useRightDragPlacement = false;
    };

    struct ResourceDef
    {
        String tag;
        String id;
        String name;
        String icon;
        ResourceKind kind = ResourceKind::Gold;
        ColorF color = Palette::Gold;
        int32 initialAmount = 0;
        int32 passiveIncomePerSec = 0;
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
