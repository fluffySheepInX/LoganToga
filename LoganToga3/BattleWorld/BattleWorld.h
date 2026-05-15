#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionStores.h"

namespace LT3
{
    inline constexpr int32 DefaultBattleMapWidth = 12;
    inline constexpr int32 DefaultBattleMapHeight = 8;

    struct QueuedBuildAction
    {
        BuildActionDefId actionId = InvalidBuildActionDefId;
        Vec2 targetPosition{ 0, 0 };
        bool hasTargetPosition = false;
    };

    struct PathRequest
    {
        UnitId unit = InvalidUnitId;
        Vec2 destination{ 0, 0 };
        Point startCell{ 0, 0 };
        Point goalCell{ 0, 0 };
        uint32 mapRevision = 0;
    };

    struct PathResult
    {
        UnitId unit = InvalidUnitId;
        Vec2 destination{ 0, 0 };
        Array<Vec2> waypoints;
        bool success = false;
        uint32 mapRevision = 0;
    };

    struct UnitRuntimeStore
    {
        Array<UnitDefId> defId;
        Array<Faction> faction;
        Array<bool> alive;
        Array<UnitTask> task;
        Array<Vec2> position;
        Array<Vec2> targetPosition;
        Array<UnitId> attackTarget;
        Array<int32> hp;
        Array<int32> resourceTargetNode;

        UnitId add(UnitDefId unitDef, Faction unitFaction, const Vec2& pos, const DefinitionStores& defs)
        {
            const UnitId id = static_cast<UnitId>(defId.size());
            defId << unitDef;
            faction << unitFaction;
            alive << true;
            task << UnitTask::Idle;
            position << pos;
            targetPosition << pos;
            attackTarget << InvalidUnitId;
            hp << defs.units[unitDef].hp;
            resourceTargetNode << -1;
            return id;
        }

        [[nodiscard]] size_t size() const
        {
            return defId.size();
        }
    };

    struct PathRuntimeStore
    {
        Array<Array<Vec2>> waypoints;
        Array<int32> waypointIndex;
        Array<Vec2> destination;
        Array<uint32> pathMapRevision;
        Array<bool> hasPath;
        Array<bool> requestPending;
        Array<double> repathCooldownSec;
        Array<PathRequest> requests;
        Array<PathResult> results;
        int32 maxRequestsPerFrame = 8;

        void addUnit(const Vec2& initialPosition)
        {
            waypoints << Array<Vec2>{};
            waypointIndex << 0;
            destination << initialPosition;
            pathMapRevision << 0u;
            hasPath << false;
            requestPending << false;
            repathCooldownSec << 0.0;
        }

        void clearUnitPath(UnitId unit)
        {
            if (unit >= waypoints.size())
            {
                return;
            }

            waypoints[unit].clear();
            waypointIndex[unit] = 0;
            pathMapRevision[unit] = 0u;
            hasPath[unit] = false;
            requestPending[unit] = false;
            repathCooldownSec[unit] = 0.0;
        }
    };

    struct CooldownStore
    {
        Array<double> attackLeftSec;

        void addUnit()
        {
            attackLeftSec << Random(0.0, 0.25);
        }
    };

    struct BuildQueueStore
    {
        Array<double> progressSec;
        Array<Array<QueuedBuildAction>> entries;
        Array<QueuedBuildAction> pendingEntry;
        Array<bool> hasPendingEntry;
        Array<bool> locked;

        void addUnit()
        {
            progressSec << 0.0;
            entries << Array<QueuedBuildAction>{};
            pendingEntry << QueuedBuildAction{};
            hasPendingEntry << false;
            locked << false;
        }
    };

    struct PlacedObjectStore
    {
        Array<Vec2> position;
        Array<String> tag;
        Array<String> icon;

        void add(const Vec2& placedPosition, const String& objectTag, const String& iconName)
        {
            position << placedPosition;
            tag << objectTag;
            icon << iconName;
        }
    };

    struct CarrierStore
    {
        Array<Array<UnitId>> storedUnits;

        void addUnit()
        {
            storedUnits << Array<UnitId>{};
        }
    };

    struct ResourceNodeStore
    {
        Array<ResourceDefId> defId;
        Array<Vec2> position;
        Array<int32> amount;
        Array<int32> incomePerSec;
        Array<Faction> owner;
        Array<double> captureProgress;

        void add(ResourceDefId resourceDef, const Vec2& pos, int32 value, int32 income)
        {
            defId << resourceDef;
            position << pos;
            amount << value;
            incomePerSec << income;
            owner << Faction::Neutral;
            captureProgress << 0.0;
        }
    };

    struct ProjectileStore
    {
        Array<Vec2> position;
        Array<Vec2> velocity;
        Array<UnitId> target;
        Array<Faction> faction;
        Array<SkillDefId> skill;
        Array<double> lifeSec;

        void add(const Vec2& pos, const Vec2& vel, UnitId targetUnit, Faction owner, SkillDefId skillDef)
        {
            position << pos;
            velocity << vel;
            target << targetUnit;
            faction << owner;
            skill << skillDef;
            lifeSec << 2.5;
        }

        void removeAt(size_t index)
        {
            const size_t last = position.size() - 1;
            if (index != last)
            {
                position[index]  = position[last];
                velocity[index]  = velocity[last];
                target[index]    = target[last];
                faction[index]   = faction[last];
                skill[index]     = skill[last];
                lifeSec[index]   = lifeSec[last];
            }
            position.pop_back();
            velocity.pop_back();
            target.pop_back();
            faction.pop_back();
            skill.pop_back();
            lifeSec.pop_back();
        }
    };

    struct ResourceRuntimeStore
    {
        Array<int32> playerAmounts;
        Array<int32> enemyAmounts;
        double incomeTickAccumSec = 0.0;
    };

    inline ResourceRuntimeStore MakeResourceRuntimeStore(const DefinitionStores& defs)
    {
        ResourceRuntimeStore store;
        store.playerAmounts.assign(defs.resources.size(), 0);
        store.enemyAmounts.assign(defs.resources.size(), 0);

        for (ResourceDefId id = 0; id < defs.resources.size(); ++id)
        {
            const ResourceDef& def = defs.resources[id];
            store.playerAmounts[id] = def.initialAmount;
            store.enemyAmounts[id] = def.initialAmount;
        }
        return store;
    }

    struct SelectionStore
    {
        UnitId selected = InvalidUnitId;
        Array<UnitId> selectedUnits;
        bool areaDragging = false;
        Vec2 areaDragStartScreen{ 0, 0 };
        Vec2 areaDragCurrentScreen{ 0, 0 };
        bool formationPlacementActive = false;
        Array<UnitId> formationUnits;
        Vec2 formationDestinationWorld{ 0, 0 };
        Vec2 formationCurrentWorld{ 0, 0 };
        bool actionPlacementActive = false;
        UnitId actionBuilder = InvalidUnitId;
        BuildActionDefId actionId = InvalidBuildActionDefId;
        Vec2 actionTargetWorld{ 0, 0 };
        bool actionLineDragging = false;
        Vec2 actionLineStartWorld{ 0, 0 };
        Array<Vec2> actionLineTargets;
        int32 hoveredResourceNode = -1;
    };

    struct BattleMapStore
    {
        int32 width  = 0;
        int32 height = 0;
        Array<uint32>  flags;          // bit 0 = passable, bit 1 = reserved barrier block
        Array<UnitId>  occupying;      // InvalidUnitId = empty
        uint32 revision = 0;

        void init(int32 w, int32 h)
        {
            width  = w;
            height = h;
            const size_t n = static_cast<size_t>(w * h);
            flags.assign(n, 1u);       // all passable by default
            occupying.assign(n, InvalidUnitId);
            revision = 0;
        }

        TileIndex index(int32 row, int32 col) const
        {
            return static_cast<TileIndex>(row * width + col);
        }

        bool inBounds(int32 row, int32 col) const
        {
            return row >= 0 && col >= 0 && row < height && col < width;
        }

        bool isPassable(int32 row, int32 col) const
        {
            return inBounds(row, col) && (flags[index(row, col)] & 1u) != 0;
        }

        bool hasBarrierReservation(int32 row, int32 col) const
        {
            return inBounds(row, col) && (flags[index(row, col)] & 2u) != 0;
        }

        void setBarrierReservation(int32 row, int32 col, bool reserved)
        {
            if (!inBounds(row, col))
            {
                return;
            }

            uint32& tileFlags = flags[index(row, col)];
            if (reserved)
            {
                tileFlags |= 2u;
            }
            else
            {
                tileFlags &= ~2u;
            }
        }
    };

    inline void ResizeBattleMapStore(BattleMapStore& map, int32 width, int32 height)
    {
        if (map.width == width && map.height == height)
        {
            return;
        }

        const int32 oldWidth = map.width;
        const int32 oldHeight = map.height;
        const Array<uint32> oldFlags = map.flags;
        const Array<UnitId> oldOccupying = map.occupying;

        map.init(width, height);
        const int32 copyWidth = Min(oldWidth, width);
        const int32 copyHeight = Min(oldHeight, height);
        for (int32 row = 0; row < copyHeight; ++row)
        {
            for (int32 col = 0; col < copyWidth; ++col)
            {
                const size_t oldIndex = static_cast<size_t>(row * oldWidth + col);
                const TileIndex newIndex = map.index(row, col);
                if (oldIndex < oldFlags.size())
                {
                    map.flags[newIndex] = oldFlags[oldIndex];
                }
                if (oldIndex < oldOccupying.size())
                {
                    map.occupying[newIndex] = oldOccupying[oldIndex];
                }
            }
        }
        ++map.revision;
    }

    struct BattleWorld
    {
        int32 mapWidth  = DefaultBattleMapWidth;
        int32 mapHeight = DefaultBattleMapHeight;
        UnitRuntimeStore  units;
        CooldownStore cooldowns;
        BuildQueueStore buildQueues;
        ResourceNodeStore resourceNodes;
        ProjectileStore   projectiles;
        PlacedObjectStore placedObjects;
        CarrierStore carriers;
        PathRuntimeStore pathing;
        ResourceRuntimeStore resources;
        SelectionStore    selection;
        BattleMapStore    map;
        bool enemyDirectorPaused = false;
        double enemySpawnTimerSec = 0.0;
        double elapsedSec         = 0.0;
        bool victory = false;
        bool defeat  = false;
    };

    inline void EnsureBattleWorldMapSize(BattleWorld& world, int32 width, int32 height)
    {
        width = Max(1, width);
        height = Max(1, height);
        world.mapWidth = width;
        world.mapHeight = height;
        ResizeBattleMapStore(world.map, width, height);
    }

    inline UnitId AddUnitToBattleWorld(BattleWorld& world, UnitDefId unitDef, Faction faction, const Vec2& pos, const DefinitionStores& defs)
    {
        const UnitId id = world.units.add(unitDef, faction, pos, defs);
        world.cooldowns.addUnit();
        world.buildQueues.addUnit();
        world.carriers.addUnit();
        world.pathing.addUnit(pos);
        return id;
    }

    inline void AddPlacedObjectToBattleWorld(BattleWorld& world, const Vec2& pos, const String& objectTag, const String& iconName)
    {
        world.placedObjects.add(pos, objectTag, iconName);
    }
}
