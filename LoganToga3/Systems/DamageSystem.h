# pragma once
# include <Siv3D.hpp>
# include "SelectionSystem.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline void ApplyProjectileHit(BattleWorld& world, const DefinitionStores& defs, size_t projectileIndex, UnitId target)
    {
        const UnitId attacker = world.projectiles.owner[projectileIndex];
        if (!IsValidUnit(world, attacker))
        {
            return;
        }

        const UnitDef& attackerDef = defs.units[world.units.defId[attacker]];
        const UnitDef& targetDef = defs.units[world.units.defId[target]];
        const SkillDef& skill = defs.skills[world.projectiles.skill[projectileIndex]];
        const double rawDamage = (static_cast<double>(attackerDef.attack) * skill.damage) - static_cast<double>(targetDef.defense);
        const int32 finalDamage = Max(1, static_cast<int32>(Math::Round(rawDamage)));
        world.units.hp[target] -= finalDamage;
        if (world.units.hp[target] <= 0)
        {
            SetUnitAlive(world, target, false);
         ClearSelectionIfUnitSelected(world, target);
        }
    }
}
