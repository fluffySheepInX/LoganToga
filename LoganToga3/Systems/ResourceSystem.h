# pragma once
# include <Siv3D.hpp>
# include "../App/BattleNotificationState.h"
# include "BattleQueries.h"
# include "BattleUnitState.h"

namespace LT3
{
    inline bool IsResourceNodeControlledByFaction(const BattleWorld& world, const DefinitionStores& defs, size_t nodeIndex, Faction faction)
    {
        if (nodeIndex >= world.resourceNodes.defId.size() || nodeIndex >= world.resourceNodes.owner.size())
        {
            return false;
        }

        const ResourceDefId resourceId = world.resourceNodes.defId[nodeIndex];
        if (resourceId >= defs.resources.size())
        {
            return false;
        }

        return world.resourceNodes.amount[nodeIndex] > 0 && world.resourceNodes.owner[nodeIndex] == faction;
    }

    inline double GetResourceNodeCaptureTimeSec(const BattleWorld& world, size_t nodeIndex)
    {
        if (nodeIndex < world.resourceNodes.captureTimeSec.size())
        {
            return Max(0.1, world.resourceNodes.captureTimeSec[nodeIndex]);
        }

        return 1.5;
    }

    inline bool IsOneShotResourceNode(const BattleWorld& world, size_t nodeIndex)
    {
        return nodeIndex < world.resourceNodes.oneShot.size() && world.resourceNodes.oneShot[nodeIndex];
    }

    inline bool IsCollectedResourceNode(const BattleWorld& world, size_t nodeIndex)
    {
        return nodeIndex < world.resourceNodes.collected.size() && world.resourceNodes.collected[nodeIndex];
    }

    inline void MarkResourceNodeCollected(BattleWorld& world, size_t nodeIndex)
    {
        if (nodeIndex < world.resourceNodes.collected.size())
        {
            world.resourceNodes.collected[nodeIndex] = true;
        }
    }

    inline void GrantOneShotResourceNode(BattleWorld& world, size_t nodeIndex, Faction faction)
    {
        if (nodeIndex >= world.resourceNodes.defId.size() || nodeIndex >= world.resourceNodes.amount.size())
        {
            return;
        }

        const ResourceDefId resourceId = world.resourceNodes.defId[nodeIndex];
        if (faction == Faction::Player)
        {
            if (resourceId < world.resources.playerAmounts.size())
            {
                world.resources.playerAmounts[resourceId] += world.resourceNodes.amount[nodeIndex];
            }
        }
        else if (faction == Faction::Enemy)
        {
            if (resourceId < world.resources.enemyAmounts.size())
            {
                world.resources.enemyAmounts[resourceId] += world.resourceNodes.amount[nodeIndex];
            }
        }

        MarkResourceNodeCollected(world, nodeIndex);
    }

    inline String BuildResourceCaptureNotificationMessage(const BattleWorld& world, const DefinitionStores& defs, size_t nodeIndex)
    {
        if (nodeIndex < world.resourceNodes.defId.size())
        {
            const ResourceDefId resourceId = world.resourceNodes.defId[nodeIndex];
            if (resourceId < defs.resources.size())
            {
                return U"資源占領: {}"_fmt(defs.resources[resourceId].name);
            }
        }

        return U"資源を占領しました";
    }

    inline void UpdateResourceNodeControl(BattleWorld& world, const DefinitionStores& defs, double dt, BattleNotificationRuntimeState* notifications = nullptr)
    {
        constexpr double captureRadius = 60.0;

        for (size_t node = 0; node < world.resourceNodes.amount.size(); ++node)
        {
            if (world.resourceNodes.amount[node] <= 0)
            {
                world.resourceNodes.owner[node] = Faction::Neutral;
                world.resourceNodes.captureProgress[node] = 0.0;
                continue;
            }

            int32 playerNearbyCount = 0;
            int32 enemyNearbyCount = 0;
            for (UnitId unit = 0; unit < world.units.size(); ++unit)
            {
                if (!IsValidUnit(world, unit))
                {
                    continue;
                }
                if (world.units.position[unit].distanceFromSq(world.resourceNodes.position[node]) > Square(captureRadius))
                {
                    continue;
                }

                if (world.units.faction[unit] == Faction::Player)
                {
                    ++playerNearbyCount;
                }
                else if (world.units.faction[unit] == Faction::Enemy)
                {
                    ++enemyNearbyCount;
                }
            }

            if (playerNearbyCount == enemyNearbyCount)
            {
                continue;
            }

            if (IsOneShotResourceNode(world, node) && IsCollectedResourceNode(world, node))
            {
                world.resourceNodes.captureProgress[node] = 1.0;
                continue;
            }

            const Faction capturingFaction = (playerNearbyCount > enemyNearbyCount) ? Faction::Player : Faction::Enemy;
            if (world.resourceNodes.owner[node] == capturingFaction)
            {
                world.resourceNodes.captureProgress[node] = 1.0;
                continue;
            }

            const double captureRatePerSec = 1.0 / GetResourceNodeCaptureTimeSec(world, node);
            world.resourceNodes.captureProgress[node] += dt * captureRatePerSec;
            if (world.resourceNodes.captureProgress[node] >= 1.0)
            {
                const Faction previousOwner = world.resourceNodes.owner[node];
                world.resourceNodes.owner[node] = capturingFaction;
                world.resourceNodes.captureProgress[node] = 1.0;
                if (IsOneShotResourceNode(world, node) && !IsCollectedResourceNode(world, node))
                {
                    GrantOneShotResourceNode(world, node, capturingFaction);
                }
                if (notifications && capturingFaction == Faction::Player && previousOwner != Faction::Player)
                {
                    PushBattleNotification(*notifications,
                        BuildResourceCaptureNotificationMessage(world, defs, node),
                        BattleNotificationType::Success);
                }
            }
        }
    }

    inline void ApplyPassiveResourceIncome(BattleWorld& world, const DefinitionStores& defs)
    {
        if (world.resources.playerAmounts.size() != defs.resources.size())
        {
            world.resources.playerAmounts.assign(defs.resources.size(), 0);
            world.resources.enemyAmounts.assign(defs.resources.size(), 0);
        }

        for (ResourceDefId resourceId = 0; resourceId < defs.resources.size(); ++resourceId)
        {
            const ResourceDef& def = defs.resources[resourceId];
            world.resources.playerAmounts[resourceId] += def.passiveIncomePerSec;
            world.resources.enemyAmounts[resourceId] += def.passiveIncomePerSec;
        }
    }

    inline void ApplyControlledResourceIncome(BattleWorld& world, const DefinitionStores& defs)
    {
        for (size_t node = 0; node < world.resourceNodes.amount.size(); ++node)
        {
            const ResourceDefId resourceId = world.resourceNodes.defId[node];
            if (resourceId >= defs.resources.size())
            {
                continue;
            }

            if (world.resourceNodes.amount[node] <= 0)
            {
                continue;
            }
            if (IsOneShotResourceNode(world, node))
            {
                continue;
            }

            const int32 incomePerSec = (node < world.resourceNodes.incomePerSec.size())
                ? world.resourceNodes.incomePerSec[node]
                : 0;
            if (incomePerSec <= 0)
            {
                continue;
            }

            if (world.resourceNodes.owner[node] == Faction::Player && resourceId < world.resources.playerAmounts.size())
            {
                world.resources.playerAmounts[resourceId] += incomePerSec;
            }
            else if (world.resourceNodes.owner[node] == Faction::Enemy && resourceId < world.resources.enemyAmounts.size())
            {
                world.resources.enemyAmounts[resourceId] += incomePerSec;
            }
        }
    }

    inline void UpdateResourceIncome(BattleWorld& world, const DefinitionStores& defs, double dt)
    {
        world.resources.incomeTickAccumSec += dt;
        while (world.resources.incomeTickAccumSec >= 1.0)
        {
            world.resources.incomeTickAccumSec -= 1.0;
            ApplyPassiveResourceIncome(world, defs);
            ApplyControlledResourceIncome(world, defs);
        }
    }

    inline void UpdateGathering(BattleWorld& world, const DefinitionStores& defs, double dt, BattleNotificationRuntimeState* notifications = nullptr)
    {
        UpdateResourceNodeControl(world, defs, dt, notifications);
        UpdateResourceIncome(world, defs, dt);

        for (UnitId unit = 0; unit < world.units.size(); ++unit)
        {
            if (!IsValidUnit(world, unit)) continue;
            if (IsBuildQueueLocked(world, unit)) continue;

            const int32 resourceTargetNode = world.units.resourceTargetNode[unit];
            if (resourceTargetNode >= 0 && static_cast<size_t>(resourceTargetNode) < world.resourceNodes.position.size())
            {
                const size_t node = static_cast<size_t>(resourceTargetNode);
                if (world.resourceNodes.amount[node] <= 0)
                {
                    ClearUnitResourceTarget(world, unit);
                    if (world.units.task[unit] != UnitTask::Building && world.units.task[unit] != UnitTask::Attacking)
                    {
                        SetUnitIdle(world, unit);
                    }
                }
                else if (world.units.position[unit].distanceFrom(world.resourceNodes.position[node]) <= 34.0)
                {
                    if (world.resourceNodes.owner[node] == world.units.faction[unit]
                        && world.resourceNodes.captureProgress[node] >= 1.0)
                    {
                        ClearUnitResourceTarget(world, unit);
                        if (world.units.task[unit] != UnitTask::Building && world.units.task[unit] != UnitTask::Attacking)
                        {
                            SetUnitIdle(world, unit);
                        }
                    }
                    else if (world.units.task[unit] != UnitTask::Building && world.units.task[unit] != UnitTask::Attacking)
                    {
                        SetUnitGathering(world, unit);
                    }
                }
            }

        }
    }
}
