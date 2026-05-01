# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline void ApplyProjectileHit(BattleWorld& world, const DefinitionStores& defs, size_t projectileIndex, UnitId target)
    {
        const UnitDef& targetDef = defs.units[world.units.defId[target]];
        const SkillDef& skill = defs.skills[world.projectiles.skill[projectileIndex]];
        world.units.hp[target] -= Max(1, skill.damage - targetDef.defense);
        if (world.units.hp[target] <= 0)
        {
            world.units.alive[target] = false;
            if (world.selection.selected == target)
            {
                world.selection.selected = InvalidUnitId;
            }
        }
    }
}
