# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline FilePath ResolveMapEditorTomlPath()
    {
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/map_editor_map.toml";
        if (FileSystem::Exists(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/map_editor_map.toml";
        if (FileSystem::Exists(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
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
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Map001.ResourceNodes.toml";
        if (FileSystem::Exists(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Map001.ResourceNodes.toml";
        if (FileSystem::Exists(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
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
        world.mapWidth  = DefaultBattleMapWidth;
        world.mapHeight = DefaultBattleMapHeight;

        world.map.init(world.mapWidth, world.mapHeight);

        world.resources = MakeResourceRuntimeStore(defs);

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
