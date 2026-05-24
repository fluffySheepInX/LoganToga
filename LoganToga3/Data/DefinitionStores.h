#pragma once
# include <Siv3D.hpp>
# include "IdTypes.h"

namespace LT3
{
    struct SkillDef
    {
        String tag;
        String name;
        String description;
        String icon;
        Array<String> iconLayers;
        SkillKind kind = SkillKind::Missile;
        double range = 120.0;
        double cooldownSec = 0.8;
        int32 mpCost = 0;
        double damage = 1.0;
        double projectileSpeed = 380.0;
        SkillProjectileMotion projectileMotion = SkillProjectileMotion::Direct;
        int32 burstCount = 1;
        double burstIntervalSec = 0.0;
        double spreadDeg = 0.0;
        double arcHeight = 72.0;
        double orbitRadius = 54.0;
        double orbitAngularSpeedDeg = 220.0;
        double orbitDurationSec = 1.2;
        double projectileWidth = 54.0;
        double projectileHeight = 72.0;
        double swingRadius = 0.0;
        double swingAngleDeg = 90.0;
        ColorF color = Palette::White;
        SkillProjectileCenter projectileCenter = SkillProjectileCenter::Off;
        bool projectileHoming = false;
        bool projectileD360 = false;
        double projectileStartDegree = 0.0;
        int32 projectileStartDegreeType = 0;
        String projectileImage;
        String projectileDiagonalImage;
    };

    struct UnitDef
    {
        String unit_id;
        String name;
        UnitRole role = UnitRole::Soldier;
        int32 hp = 50;
        int32 attack = 6;
        int32 defense = 0;
        double speed = 80.0;
        double radius = 14.0;
        int32 gatherPower = 0;
        int32 visionRadiusCells = 6;
        SkillDefId skill = InvalidSkillDefId;
        ColorF color = Palette::White;
        double visualScale = 1.0;
        String building_category;
        String unit_family;
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
        Array<String> iconLayers;
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
        int32 costTrust = 0;
        int32 costFood = 0;
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

    struct AiUnitWeightDef
    {
        String unitTag;
        double weight = 1.0;
    };

    struct AiProfileDef
    {
        String tag;
        String name;
        String description;
        String presetType;
        double openingDelaySec = 10.0;
        double spawnIntervalSec = 8.0;
        double attackWaveIntervalSec = 35.0;
        double aggression = 0.5;
        double economyFocus = 0.5;
        double defenseFocus = 0.5;
        double techFocus = 0.3;
        int32 attackGroupSize = 4;
        int32 maxArmySize = 24;
        double retreatHpRatio = 0.0;
        bool freeSpawnEnabled = true;
        double resourceMultiplier = 1.0;
        String contactBehavior = U"ignore";
        Array<AiUnitWeightDef> unitWeights;
        Array<String> targetPriority;
    };

    struct DefinitionStores
    {
        Array<SkillDef> skills;
        Array<UnitDef> units;
        Array<BuildActionDef> buildActions;
        Array<ResourceDef> resources;
        Array<AiProfileDef> aiProfiles;
        HashTable<String, Array<String>> skillIconWarningsByTag;
        HashTable<String, SkillDefId> skillByTag;
        HashTable<String, UnitDefId> unitByTag;
        HashTable<String, BuildActionDefId> buildActionByTag;
        HashTable<String, ResourceDefId> resourceByTag;
        HashTable<String, AiProfileDefId> aiProfileByTag;

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
            unitByTag[def.unit_id] = id;
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

        AiProfileDefId addAiProfile(const AiProfileDef& def)
        {
            const AiProfileDefId id = static_cast<AiProfileDefId>(aiProfiles.size());
            aiProfiles << def;
            aiProfileByTag[def.tag] = id;
            return id;
        }
    };
}
