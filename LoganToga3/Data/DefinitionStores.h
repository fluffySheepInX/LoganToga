#pragma once
# include <Siv3D.hpp>
# include "IdTypes.h"

namespace LT3
{
    struct SkillResourceCostDef
    {
        String resourceTag;
        int32 amount = 1;
    };

    struct SkillDef
    {
        String tag;
        String name;
        String description;
        String icon;
        Array<String> iconLayers;
        SkillKind kind = SkillKind::Missile;
        double range = 120.0;
        double rangeMin = 0.0;
        double cooldownSec = 0.8;
        int32 mpCost = 0;
        double damage = 1.0;
        double selfDamageOnHit = 0.0;
        double projectileSpeed = 380.0;
        SkillProjectileMotion projectileMotion = SkillProjectileMotion::Direct;
        int32 burstCount = 1;
        double burstIntervalSec = 0.0;
        SkillBurstFireMode burstFireMode = SkillBurstFireMode::Simultaneous;
        SkillBurstOrderMode burstOrderMode = SkillBurstOrderMode::Sequential;
        SkillRayMode rayMode = SkillRayMode::None;
        double rayLength = 1.0;
        bool rayLockToCaster = false;
        double spreadDeg = 0.0;
        double arcHeight = 72.0;
        double orbitRadius = 54.0;
        double orbitAngularSpeedDeg = 220.0;
        double orbitDurationSec = 1.2;
        double projectileWidth = 54.0;
        double projectileHeight = 72.0;
        double swingRadius = 0.0;
        double swingAngleDeg = 90.0;
        SkillSwingHitMode swingHitMode = SkillSwingHitMode::Stop;
        ColorF color = Palette::White;
        bool bom = false;
        double bomRadius = 0.0;
        SkillBomVisual bomVisual = SkillBomVisual::Circle;
        String bomImage;
        double bomVisualScale = 1.0;
        double bomVisualDurationSec = 0.22;
        bool bomFriendlyFire = false;
        double bomSelfDamageScale = 0.0;
        bool allfunc = false;
        SkillProjectileCenter projectileCenter = SkillProjectileCenter::Off;
        bool projectileHoming = false;
        bool projectileD360 = false;
        double projectileStartDegree = 0.0;
        int32 projectileStartDegreeType = 0;
        String nextSkillTag;
        SkillDefId nextSkill = InvalidSkillDefId;
        bool nextLast = false;
        bool jointSkill = false;
        bool sendTarget = false;
        bool sendImageDegree = false;
        String soundEffect;
        double soundEffectVolume = 1.0;
        String projectileImage;
        String projectileDiagonalImage;
        Array<SkillResourceCostDef> resourceCosts;
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
        Array<SkillDefId> skills;
        ColorF color = Palette::White;
        double visualScale = 1.0;
        bool unique = false;
        bool uniqueRespawnAllowed = false;
        String spawnVoice;
        double spawnVoiceVolume = 1.0;
        double spawnVoiceCooldownSec = 0.0;
        bool spawnVoiceForEnemy = true;
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
        bool enemyCanProduce = true;
        CarrierActionKind carrierAction = CarrierActionKind::Store;
        double carrierRadiusPx = 84.0;
        int32 carrierMaxUnits = 0;
    };

    struct ResourceDef
    {
        // 内部参照用の一意タグ
        String tag;
        // シナリオや表示順で使う外部ID
        String id;
        // UI表示用の資源名
        String name;
        // UI表示に使うアイコンファイル名
        String icon;
        // 資源種別（Gold / Trust / Food など）
        ResourceKind kind = ResourceKind::Gold;
        // UI上の基本表示色
        ColorF color = Palette::Gold;
        // 戦闘開始時の初期保有量
        int32 initialAmount = 0;
        // 時間経過で毎秒増加する基本収入量
        int32 passiveIncomePerSec = 0;
    };

    struct AiUnitWeightDef
    {
        String unitTag;
        double weight = 1.0;
        int32 desiredCount = 0;
    };

    struct AiBuildPriorityDef
    {
        String actionTag;
        double weight = 1.0;
        int32 desiredCount = 1;
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
        double battleTimeLimitSec = 25.0 * 60.0;
        double retreatHpRatio = 0.0;
        bool freeSpawnEnabled = true;
        double resourceMultiplier = 1.0;
        String contactBehavior = U"ignore";
        Array<String> initialUnits;
        Array<AiUnitWeightDef> unitWeights;
        Array<AiBuildPriorityDef> buildPriorities;
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
