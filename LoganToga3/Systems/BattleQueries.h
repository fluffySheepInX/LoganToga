# pragma once
# include <Siv3D.hpp>
# include "../BattleWorld/BattleWorld.h"

namespace LT3
{
    struct BuildActionUiState
    {
        BuildActionDefId actionId = InvalidBuildActionDefId;
        bool affordable = false;
    };

    inline bool IsValidUnit(const BattleWorld& world, UnitId unit)
    {
        return unit != InvalidUnitId && unit < world.units.size() && world.units.alive[unit];
    }

    inline double UnitSelectionRadius(const UnitDef& def)
    {
        return Max(def.radius, def.radius * def.visualScale);
    }

    inline bool IsEnemy(Faction a, Faction b)
    {
        return (a == Faction::Player && b == Faction::Enemy) || (a == Faction::Enemy && b == Faction::Player);
    }

    inline bool CanUseBuildAction(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, const BuildActionDef& action)
    {
        return IsValidUnit(world, unit)
            && world.units.faction[unit] == Faction::Player
            && defs.units[world.units.defId[unit]].tag.lowercased() == action.ownerTag.lowercased();
    }

    inline bool CanAffordBuildAction(const BattleWorld& world, const BuildActionDef& action)
    {
        return world.resources.playerGold >= action.costGold;
    }

    inline bool CanStartBuildAction(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, BuildActionDefId actionId)
    {
        if (!IsValidUnit(world, unit)) return false;
        if (actionId >= defs.buildActions.size()) return false;
        if (defs.units[world.units.defId[unit]].role != UnitRole::Base) return false;

        const BuildActionDef& action = defs.buildActions[actionId];
        return CanUseBuildAction(world, defs, unit, action)
            && CanAffordBuildAction(world, action);
    }

    inline const Array<BuildActionDefId>& GetQueuedBuildActions(const BattleWorld& world, UnitId unit)
    {
        static const Array<BuildActionDefId> empty;
        if (!IsValidUnit(world, unit) || unit >= world.buildQueues.actionIds.size())
        {
            return empty;
        }

        return world.buildQueues.actionIds[unit];
    }

    inline Array<BuildActionUiState> CollectVisibleBuildActions(const BattleWorld& world, const DefinitionStores& defs, UnitId unit)
    {
        Array<BuildActionUiState> visibleActions;
        visibleActions.reserve(defs.buildActions.size());

        for (int32 i = 0; i < static_cast<int32>(defs.buildActions.size()); ++i)
        {
            const BuildActionDef& action = defs.buildActions[i];
            if (CanUseBuildAction(world, defs, unit, action))
            {
                visibleActions << BuildActionUiState{ static_cast<BuildActionDefId>(i), CanAffordBuildAction(world, action) };
            }
        }

        return visibleActions;
    }

    inline UnitId FindNearestEnemy(const BattleWorld& world, UnitId unit, double range)
    {
        if (!IsValidUnit(world, unit)) return InvalidUnitId;

        const Vec2 pos = world.units.position[unit];
        const Faction faction = world.units.faction[unit];
        UnitId best = InvalidUnitId;
        double bestDistanceSq = range * range;

        for (UnitId other = 0; other < world.units.size(); ++other)
        {
            if (!IsValidUnit(world, other)) continue;
            if (!IsEnemy(faction, world.units.faction[other])) continue;

            const double distanceSq = pos.distanceFromSq(world.units.position[other]);
            if (distanceSq < bestDistanceSq)
            {
                bestDistanceSq = distanceSq;
                best = other;
            }
        }

        return best;
    }

    inline UnitDefId ResolveCommandBaseUnitDefId(const DefinitionStores& defs)
    {
        for (UnitDefId id = 0; id < defs.units.size(); ++id)
        {
            const UnitDef& def = defs.units[id];
            if (def.role == UnitRole::Base && def.tag.lowercased() == U"home")
            {
                return id;
            }
        }

        for (UnitDefId id = 0; id < defs.units.size(); ++id)
        {
            if (defs.units[id].role == UnitRole::Base)
            {
                return id;
            }
        }

        return InvalidUnitDefId;
    }
}
