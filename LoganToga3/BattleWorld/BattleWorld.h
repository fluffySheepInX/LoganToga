#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionStores.h"

namespace LT3
{
    inline constexpr int32 DefaultBattleMapWidth = 12;
    inline constexpr int32 DefaultBattleMapHeight = 8;

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
            return id;
        }

        [[nodiscard]] size_t size() const
        {
            return defId.size();
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
        Array<Array<BuildActionDefId>> actionIds;

        void addUnit()
        {
            progressSec << 0.0;
            actionIds << Array<BuildActionDefId>{};
        }
    };

    struct ResourceNodeStore
    {
        Array<ResourceDefId> defId;
        Array<Vec2> position;
        Array<int32> amount;

        void add(ResourceDefId resourceDef, const Vec2& pos, int32 value)
        {
            defId << resourceDef;
            position << pos;
            amount << value;
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
        int32 playerGold = 0;
        int32 enemyGold  = 0;
    };

    inline ResourceRuntimeStore MakeResourceRuntimeStore(const DefinitionStores& defs)
    {
        ResourceRuntimeStore store;
        if (const auto it = defs.resourceByTag.find(U"gold"); it != defs.resourceByTag.end())
        {
            const ResourceDef& def = defs.resources[it->second];
            store.playerGold = def.initialAmount;
            store.enemyGold  = def.initialAmount;
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
    };

    struct BattleMapStore
    {
        int32 width  = 0;
        int32 height = 0;
        Array<uint32>  flags;          // bit 0 = passable
        Array<UnitId>  occupying;      // InvalidUnitId = empty

        void init(int32 w, int32 h)
        {
            width  = w;
            height = h;
            const size_t n = static_cast<size_t>(w * h);
            flags.assign(n, 1u);       // all passable by default
            occupying.assign(n, InvalidUnitId);
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
    };

    struct BattleWorld
    {
        int32 mapWidth  = DefaultBattleMapWidth;
        int32 mapHeight = DefaultBattleMapHeight;
        UnitRuntimeStore  units;
        CooldownStore cooldowns;
        BuildQueueStore buildQueues;
        ResourceNodeStore resourceNodes;
        ProjectileStore   projectiles;
        ResourceRuntimeStore resources;
        SelectionStore    selection;
        BattleMapStore    map;
        double enemySpawnTimerSec = 0.0;
        double elapsedSec         = 0.0;
        bool victory = false;
        bool defeat  = false;
    };

    inline UnitId AddUnitToBattleWorld(BattleWorld& world, UnitDefId unitDef, Faction faction, const Vec2& pos, const DefinitionStores& defs)
    {
        const UnitId id = world.units.add(unitDef, faction, pos, defs);
        world.cooldowns.addUnit();
        world.buildQueues.addUnit();
        return id;
    }
}
