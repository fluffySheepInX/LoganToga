# pragma once
# include <Siv3D.hpp>
# include "../BattleWorld/BattleWorld.h"
# include "../UI/QuarterView.h"

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

    inline bool EqualsIgnoreCase(const String& a, const String& b)
    {
        return !a.isEmpty() && !b.isEmpty() && a.lowercased() == b.lowercased();
    }

    inline bool DoesUnitMatchOwnerTag(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, const String& ownerTag)
    {
        if (!IsValidUnit(world, unit) || ownerTag.isEmpty())
        {
            return false;
        }

        const UnitDef& def = defs.units[world.units.defId[unit]];
        return EqualsIgnoreCase(def.unit_id, ownerTag)
            || EqualsIgnoreCase(def.building_category, ownerTag)
            || EqualsIgnoreCase(def.unit_family, ownerTag);
    }

    inline bool DoesUnitMatchAnyOwnerTag(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, const BuildActionDef& action)
    {
        if (!action.ownerTags.isEmpty())
        {
            for (const auto& ownerTag : action.ownerTags)
            {
                if (DoesUnitMatchOwnerTag(world, defs, unit, ownerTag))
                {
                    return true;
                }
            }
            return false;
        }

        return DoesUnitMatchOwnerTag(world, defs, unit, action.ownerTag);
    }

    inline bool IsBuildActionSupported(const BuildActionDef& action)
    {
        switch (action.resultType)
        {
        case BuildActionResultType::Unit:
            return action.spawnUnit != InvalidUnitDefId;
        case BuildActionResultType::Object:
            return !action.resultTag.isEmpty();
        case BuildActionResultType::Carrier:
            return true;
        default:
            return false;
        }
    }

    inline bool DoesBuildActionRequireTargetPosition(const BuildActionDef& action)
    {
        return action.isMove || action.resultType == BuildActionResultType::Object;
    }

    inline bool CanUseBuildAction(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, const BuildActionDef& action)
    {
        return IsValidUnit(world, unit)
            && world.units.faction[unit] == Faction::Player
            && IsBuildActionSupported(action)
            && DoesUnitMatchAnyOwnerTag(world, defs, unit, action);
    }

    inline bool CanAffordBuildAction(const BattleWorld& world, const BuildActionDef& action)
    {
        return !world.resources.playerAmounts.isEmpty() && world.resources.playerAmounts[0] >= action.costGold;
    }

    inline ResourceDefId FindResourceDefByKind(const DefinitionStores& defs, ResourceKind kind)
    {
        for (ResourceDefId id = 0; id < defs.resources.size(); ++id)
        {
            if (defs.resources[id].kind == kind)
            {
                return id;
            }
        }

        return InvalidResourceDefId;
    }

    inline int32 GetFactionResourceAmount(const BattleWorld& world, Faction faction, ResourceDefId resourceId)
    {
        if (resourceId == InvalidResourceDefId)
        {
            return 0;
        }

        const Array<int32>& values = (faction == Faction::Enemy)
            ? world.resources.enemyAmounts
            : world.resources.playerAmounts;
        if (resourceId >= values.size())
        {
            return 0;
        }

        return values[resourceId];
    }

    inline int32 GetPlayerResourceAmount(const BattleWorld& world, ResourceDefId resourceId)
    {
        return GetFactionResourceAmount(world, Faction::Player, resourceId);
    }

    inline Optional<size_t> FindHoveredResourceNode(const BattleWorld& world, const Vec2& screenPos, double radius = 42.0)
    {
        size_t bestIndex = 0;
        double bestDistanceSq = Square(radius);
        bool found = false;
        const double scale = QuarterViewCamera2D.getScale();
        const Vec2 cameraCenter = QuarterViewCamera2D.getCenter();

        for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
        {
            if (world.resourceNodes.amount[node] <= 0)
            {
                continue;
            }

            const Vec2 faceCenter = QuarterTileFaceCenterScreen(world.resourceNodes.position[node]);
            const Vec2 nodeScreen = QuarterViewOrigin + ((faceCenter - QuarterViewOrigin) * scale) - (cameraCenter * scale);
            const double distanceSq = nodeScreen.distanceFromSq(screenPos);
            if (distanceSq <= bestDistanceSq)
            {
                bestDistanceSq = distanceSq;
                bestIndex = node;
                found = true;
            }
        }

        if (!found)
        {
            return none;
        }

        return bestIndex;
    }

    inline bool CanStartBuildAction(const BattleWorld& world, const DefinitionStores& defs, UnitId unit, BuildActionDefId actionId)
    {
        if (!IsValidUnit(world, unit)) return false;
        if (actionId >= defs.buildActions.size()) return false;

        const BuildActionDef& action = defs.buildActions[actionId];
        return CanUseBuildAction(world, defs, unit, action)
            && CanAffordBuildAction(world, action);
    }

    inline const Array<QueuedBuildAction>& GetQueuedBuildActionEntries(const BattleWorld& world, UnitId unit)
    {
        static const Array<QueuedBuildAction> empty;
        if (!IsValidUnit(world, unit) || unit >= world.buildQueues.entries.size())
        {
            return empty;
        }

        return world.buildQueues.entries[unit];
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
            if (def.role == UnitRole::Base && def.unit_id.lowercased() == U"home")
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
