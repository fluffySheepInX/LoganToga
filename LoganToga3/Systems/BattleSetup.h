# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
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

        if (defs.unitByTag.contains(U"Home"))
        {
            const UnitDefId home = defs.unitByTag.at(U"Home");
            AddUnitToBattleWorld(world, home, Faction::Player, Vec2{ 210, 450 }, defs);
            AddUnitToBattleWorld(world, home, Faction::Enemy, Vec2{ 1390, 450 }, defs);
        }

        LoadDefaultBattleResourceNodes(world, defs);
    }
}
