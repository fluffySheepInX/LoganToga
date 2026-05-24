# pragma once
# include <Siv3D.hpp>
# include "../Data/TomlTextUtils.h"
# include "BattleQueries.h"

namespace LT3
{
    inline FilePath ResolveMapEditorTomlPath();

    inline Size LoadBattleMapSizeFromMapEditorToml()
    {
        const TOMLReader toml{ ResolveMapEditorTomlPath() };
        if (!toml)
        {
            return Size{ DefaultBattleMapWidth, DefaultBattleMapHeight };
        }

        return Size{
            Clamp(toml[U"map.width"].getOr<int32>(DefaultBattleMapWidth), 1, 40),
            Clamp(toml[U"map.height"].getOr<int32>(DefaultBattleMapHeight), 1, 40)
        };
    }

    inline bool IsDecalAssetFileNameForBattle(StringView fileName)
    {
        return String{ fileName }.lowercased().starts_with(U"decal_");
    }

    inline void ApplyDecalPassabilityFromMapEditorToml(BattleWorld& world)
    {
        const TOMLReader toml{ ResolveMapEditorTomlPath() };
        if (!toml)
        {
            return;
        }

        const TOMLValue tilesValue = toml[U"tiles"];
        if (!tilesValue.isTableArray())
        {
            return;
        }

        for (const auto tileValue : tilesValue.tableArrayView())
        {
            const int32 x = tileValue[U"x"].getOr<int32>(-1);
            const int32 y = tileValue[U"y"].getOr<int32>(-1);
            if (!world.map.inBounds(y, x))
            {
                continue;
            }

            bool blocksPassage = false;

            const String object = tileValue[U"object"].getOr<String>(U"");
            if (!object.isEmpty() && IsDecalAssetFileNameForBattle(object))
            {
                blocksPassage = tileValue[U"decal_blocks_passage"].getOr<bool>(false);
            }

            const TOMLValue decalsValue = tileValue[U"decals"];
            if (!blocksPassage && decalsValue.isTableArray())
            {
                for (const auto decalValue : decalsValue.tableArrayView())
                {
                    if (decalValue[U"decal_blocks_passage"].getOr<bool>(false))
                    {
                        blocksPassage = true;
                        break;
                    }
                }
            }

            if (!blocksPassage)
            {
                continue;
            }

            const TileIndex idx = world.map.index(y, x);
            if ((world.map.flags[idx] & 1u) != 0)
            {
                world.map.flags[idx] &= ~1u;
                ++world.map.revision;
            }
        }
    }

    inline FilePath ResolveMapEditorTomlPath()
    {
        return ResolveFirstExistingPath({
            U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/map_editor_map.toml",
            U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/map_editor_map.toml",
        });
    }

    inline std::pair<Vec2, Vec2> LoadHomePositionsFromMapEditorToml()
    {
        Vec2 playerHome{ 210.0, 450.0 };
        Vec2 enemyHome{ 1390.0, 450.0 };

        const TOMLReader toml{ ResolveMapEditorTomlPath() };
        if (!toml)
        {
            return { playerHome, enemyHome };
        }

        playerHome.x = toml[U"home.player_x"].getOr<double>(playerHome.x);
        playerHome.y = toml[U"home.player_y"].getOr<double>(playerHome.y);
        enemyHome.x = toml[U"home.enemy_x"].getOr<double>(enemyHome.x);
        enemyHome.y = toml[U"home.enemy_y"].getOr<double>(enemyHome.y);
        return { playerHome, enemyHome };
    }

    inline FilePath ResolveResourceNodeTomlPath()
    {
        return ResolveFirstExistingPath({
            U"000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Map001.ResourceNodes.toml",
            U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Map001.ResourceNodes.toml",
        });
    }

    inline AiProfileDefId ResolveDefaultAiProfileDefId(const DefinitionStores& defs)
    {
        if (defs.aiProfileByTag.contains(U"balanced"))
        {
            return defs.aiProfileByTag.at(U"balanced");
        }

        if (!defs.aiProfiles.isEmpty())
        {
            return 0;
        }

        return InvalidAiProfileDefId;
    }

    inline String LoadSelectedAiProfileTagFromMapEditorToml()
    {
        const TOMLReader toml{ ResolveMapEditorTomlPath() };
        if (!toml)
        {
            return U"";
        }

        return toml[U"ai.selected_profile"].getOr<String>(U"").lowercased();
    }

    inline AiProfileDefId ResolveSelectedAiProfileDefId(const DefinitionStores& defs)
    {
        const String selectedTag = LoadSelectedAiProfileTagFromMapEditorToml();
        if (!selectedTag.isEmpty() && defs.aiProfileByTag.contains(selectedTag))
        {
            return defs.aiProfileByTag.at(selectedTag);
        }

        return ResolveDefaultAiProfileDefId(defs);
    }

    inline void InitializeAiRuntime(BattleWorld& world, const DefinitionStores& defs)
    {
        const AiProfileDefId profileId = ResolveSelectedAiProfileDefId(defs);
        const String profileTag = (profileId != InvalidAiProfileDefId && profileId < defs.aiProfiles.size())
            ? defs.aiProfiles[profileId].tag
            : U"";
        world.aiRuntime.resetForProfile(profileId, profileTag);
        world.enemySpawnTimerSec = world.aiRuntime.spawnTimerSec;
    }

    inline void LoadDefaultBattleResourceNodes(BattleWorld& world, const DefinitionStores& defs)
    {
        const TOMLReader toml{ ResolveResourceNodeTomlPath() };
        if (toml)
        {
            const TOMLValue resourceNodes = toml[U"resource_nodes"];
            if (resourceNodes.isEmpty())
            {
                return;
            }

            try
            {
                for (const auto& nodeValue : resourceNodes.tableArrayView())
                {
                    const String resourceKind = nodeValue[U"resource_kind"].getOr<String>(U"").lowercased();
                    if (!defs.resourceByTag.contains(resourceKind))
                    {
                        continue;
                    }

                    Array<double> positionValues;
                    const TOMLValue position = nodeValue[U"position"];
                    if (position.isEmpty() || !position.isArray())
                    {
                        continue;
                    }

                    for (const auto& positionValue : position.arrayView())
                    {
                        if (const Optional<double> coordinate = positionValue.getOpt<double>())
                        {
                            positionValues << *coordinate;
                        }
                    }
                    if (positionValues.size() < 2)
                    {
                        continue;
                    }

                    world.resourceNodes.add(
                        defs.resourceByTag.at(resourceKind),
                        Vec2{ positionValues[0], positionValues[1] },
                        Max(0, nodeValue[U"amount"].getOr<int32>(0)),
                        Max(0, nodeValue[U"income_per_sec"].getOr<int32>(0)));
                }
            }
            catch (const std::exception&)
            {
            }
        }
    }

    inline void SpawnDefaultBattle(BattleWorld& world, const DefinitionStores& defs)
    {
        const Size mapSize = LoadBattleMapSizeFromMapEditorToml();
        world.mapWidth  = mapSize.x;
        world.mapHeight = mapSize.y;

        world.map.init(world.mapWidth, world.mapHeight);
        ApplyDecalPassabilityFromMapEditorToml(world);

        world.resources = MakeResourceRuntimeStore(defs);
        InitializeAiRuntime(world, defs);

        const UnitDefId home = ResolveCommandBaseUnitDefId(defs);
        if (home != InvalidUnitDefId)
        {
            const auto homePositions = LoadHomePositionsFromMapEditorToml();
            AddUnitToBattleWorld(world, home, Faction::Player, homePositions.first, defs);
            AddUnitToBattleWorld(world, home, Faction::Enemy, homePositions.second, defs);
        }

        LoadDefaultBattleResourceNodes(world, defs);
    }
}
