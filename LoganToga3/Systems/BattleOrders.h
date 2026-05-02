# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    inline Optional<UnitId> PickUnitAt(const BattleWorld& world, const DefinitionStores& defs, const Vec2& pos, Faction faction)
    {
        for (int32 i = static_cast<int32>(world.units.size()) - 1; i >= 0; --i)
        {
            const UnitId unit = static_cast<UnitId>(i);
            if (!IsValidUnit(world, unit) || world.units.faction[unit] != faction) continue;

            const UnitDef& def = defs.units[world.units.defId[unit]];
            if (Circle{ world.units.position[unit], UnitSelectionRadius(def) + 6.0 }.intersects(pos))
            {
                return unit;
            }
        }

        return none;
    }

    inline void IssueMove(BattleWorld& world, UnitId unit, const Vec2& destination)
    {
        if (!IsValidUnit(world, unit)) return;
        world.units.targetPosition[unit] = destination;
        world.units.attackTarget[unit] = InvalidUnitId;
        world.units.task[unit] = UnitTask::Moving;
    }

    inline Vec2 ComputeUnitGroupCenter(const BattleWorld& world, const Array<UnitId>& units)
    {
        Vec2 center{ 0, 0 };
        int32 count = 0;
        for (const UnitId unit : units)
        {
            if (!IsValidUnit(world, unit))
            {
                continue;
            }

            center += world.units.position[unit];
            ++count;
        }

        return (count > 0) ? (center / static_cast<double>(count)) : Vec2{ 0, 0 };
    }

    inline Vec2 ResolveFormationFacingDirection(const BattleWorld& world, const Array<UnitId>& units, const Vec2& destination, const Vec2& current)
    {
        Vec2 facing = current - destination;
        if (facing.lengthSq() < 16.0)
        {
            facing = destination - ComputeUnitGroupCenter(world, units);
        }
        if (facing.lengthSq() < 0.0001)
        {
            facing = Vec2{ 1.0, 0.0 };
        }
        return facing.normalized();
    }

    inline Array<Vec2> BuildFormationMoveTargets(const BattleWorld& world, const DefinitionStores& defs, const Array<UnitId>& units, const Vec2& destination, const Vec2& facingDirection)
    {
        Array<UnitId> validUnits;
        double maxRadius = 0.0;
        for (const UnitId unit : units)
        {
            if (!IsValidUnit(world, unit))
            {
                continue;
            }

            validUnits << unit;
            maxRadius = Max(maxRadius, defs.units[world.units.defId[unit]].radius);
        }

        Array<Vec2> targets;
        if (validUnits.isEmpty())
        {
            return targets;
        }

        const Vec2 forward = (facingDirection.lengthSq() < 0.0001)
            ? Vec2{ 1.0, 0.0 }
            : facingDirection.normalized();
        const Vec2 lateral{ -forward.y, forward.x };
        const double spacing = Max(32.0, maxRadius * 2.8);
        const int32 columns = Max(1, static_cast<int32>(Math::Ceil(Sqrt(static_cast<double>(validUnits.size())))));

        targets.reserve(validUnits.size());
        size_t startIndex = 0;
        int32 row = 0;
        while (startIndex < validUnits.size())
        {
            const int32 rowCount = Min(columns, static_cast<int32>(validUnits.size() - startIndex));
            const double rowCenter = (static_cast<double>(rowCount) - 1.0) * 0.5;
            for (int32 column = 0; column < rowCount; ++column)
            {
                const double sideOffset = (static_cast<double>(column) - rowCenter) * spacing;
                const double forwardOffset = -static_cast<double>(row) * spacing;
                targets << (destination + (lateral * sideOffset) + (forward * forwardOffset));
            }

            startIndex += rowCount;
            ++row;
        }

        return targets;
    }

    inline void IssueFormationMove(BattleWorld& world, const DefinitionStores& defs, const Array<UnitId>& units, const Vec2& destination, const Vec2& facingDirection)
    {
        Array<UnitId> validUnits;
        for (const UnitId unit : units)
        {
            if (IsValidUnit(world, unit))
            {
                validUnits << unit;
            }
        }

        if (validUnits.isEmpty())
        {
            return;
        }

        const Array<Vec2> targets = BuildFormationMoveTargets(world, defs, validUnits, destination, facingDirection);
        for (size_t i = 0; i < validUnits.size() && i < targets.size(); ++i)
        {
            IssueMove(world, validUnits[i], targets[i]);
        }
    }

    inline bool TryStartBuild(BattleWorld& world, const DefinitionStores& defs, UnitId builder, BuildActionDefId actionId)
    {
        if (!CanStartBuildAction(world, defs, builder, actionId)) return false;

        const BuildActionDef& action = defs.buildActions[actionId];
        world.resources.playerGold -= action.costGold;
        Array<BuildActionDefId>& queue = world.buildQueues.actionIds[builder];
        const bool wasEmpty = queue.isEmpty();
        queue << actionId;
        if (wasEmpty)
        {
            world.units.task[builder] = UnitTask::Building;
            world.buildQueues.progressSec[builder] = 0.0;
        }
        return true;
    }
}
