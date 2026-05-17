# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"

namespace LT3
{
    inline Vec2 ResolveEnemySpawnOrigin(const BattleWorld& world, const DefinitionStores& defs)
    {
        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
            {
                continue;
            }
            if (world.units.defId[unit] >= defs.units.size())
            {
                continue;
            }

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (def.role == UnitRole::Base && def.tag.lowercased() == U"home")
            {
                return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
            }
        }

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
            {
                continue;
            }
            if (world.units.defId[unit] >= defs.units.size())
            {
                continue;
            }

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (def.role == UnitRole::Base)
            {
                return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
            }
        }

        return Vec2{ 1325, 450.0 };
    }

    inline void UpdateEnemyDirector(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
     const UnitId unitCount = static_cast<UnitId>(world.units.size());

        world.enemySpawnTimerSec += dt;
        if (world.enemySpawnTimerSec >= 8.0 && defs.units.size() > 0)
        {
            world.enemySpawnTimerSec = 0.0;
            Array<UnitDefId> enemySpawnCandidates;
            for (UnitDefId unitDefId = 0; unitDefId < defs.units.size(); ++unitDefId)
            {
                const UnitDef& def = defs.units[unitDefId];
                if (def.role == UnitRole::Base || def.role == UnitRole::Barrier)
                {
                    continue;
                }
                if (def.speed <= 0.0)
                {
                    continue;
                }
                enemySpawnCandidates << unitDefId;
            }

            if (!enemySpawnCandidates.isEmpty())
            {
                const UnitDefId spawn = enemySpawnCandidates.choice();
                const Vec2 enemySpawnOrigin = ResolveEnemySpawnOrigin(world, defs);
                AddUnitToBattleWorld(world, spawn, Faction::Enemy, enemySpawnOrigin + Vec2{ 0.0, Random(-100.0, 100.0) }, defs);
            }
        }

        for (UnitId unit = 0; unit < unitCount; ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (world.units.faction[unit] != Faction::Enemy) continue;
            const UnitDef& unitDef = defs.units[world.units.defId[unit]];
            if (unitDef.role == UnitRole::Base) continue;
            if (unitDef.skill == InvalidSkillDefId || unitDef.skill >= defs.skills.size()) continue;

            const UnitId target = FindNearestEnemy(world, unit, 620.0);
            if (IsValidUnit(world, target))
            {
                const SkillDef& skill = defs.skills[unitDef.skill];
                if (world.units.position[unit].distanceFrom(world.units.position[target]) > skill.range * 0.85)
                {
                    IssueMove(world, unit, world.units.position[target]);
                }
                world.units.attackTarget[unit] = target;
            }
        }
    }
}
