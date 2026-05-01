# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void SpawnDefaultBattle(BattleWorld& world, const DefinitionStores& defs)
    {
        world.mapWidth = DefaultBattleMapWidth;
        world.mapHeight = DefaultBattleMapHeight;

        const UnitDefId worker = defs.unitByTag.at(U"worker");
        const UnitDefId soldier = defs.unitByTag.at(U"soldier");
        const UnitDefId archer = defs.unitByTag.at(U"archer");
        const UnitDefId base = ResolveCommandBaseUnitDefId(defs);
        const ResourceDefId gold = defs.resourceByTag.at(U"gold");

        if (base == InvalidUnitDefId)
        {
            return;
        }

        world.units.add(base, Faction::Player, Vec2{ 210, 450 }, defs);
        world.units.add(worker, Faction::Player, Vec2{ 285, 405 }, defs);
        world.units.add(worker, Faction::Player, Vec2{ 285, 495 }, defs);
        world.units.add(soldier, Faction::Player, Vec2{ 365, 450 }, defs);
        world.units.add(archer, Faction::Player, Vec2{ 335, 515 }, defs);

        world.units.add(base, Faction::Enemy, Vec2{ 1390, 450 }, defs);
        world.units.add(soldier, Faction::Enemy, Vec2{ 1280, 410 }, defs);
        world.units.add(soldier, Faction::Enemy, Vec2{ 1280, 490 }, defs);
        world.units.add(archer, Faction::Enemy, Vec2{ 1240, 450 }, defs);

        world.resourceNodes.add(gold, Vec2{ 610, 310 }, 700);
        world.resourceNodes.add(gold, Vec2{ 610, 590 }, 700);
        world.resourceNodes.add(gold, Vec2{ 800, 450 }, 900);
        world.resourceNodes.add(gold, Vec2{ 990, 310 }, 700);
        world.resourceNodes.add(gold, Vec2{ 990, 590 }, 700);
    }
}
