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
        Array<double> cooldownLeftSec;
        Array<double> buildProgressSec;
        Array<BuildActionDefId> buildAction;

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
            cooldownLeftSec << Random(0.0, 0.25);
            buildProgressSec << 0.0;
            buildAction << InvalidBuildActionDefId;
            return id;
        }

        [[nodiscard]] size_t size() const
        {
            return defId.size();
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
            position.remove_at(index);
            velocity.remove_at(index);
            target.remove_at(index);
            faction.remove_at(index);
            skill.remove_at(index);
            lifeSec.remove_at(index);
        }
    };

    struct ResourceRuntimeStore
    {
        int32 playerGold = 110;
        int32 enemyGold = 120;
    };

    struct SelectionStore
    {
        UnitId selected = InvalidUnitId;
    };

    struct BattleWorld
    {
        int32 mapWidth = DefaultBattleMapWidth;
        int32 mapHeight = DefaultBattleMapHeight;
        UnitRuntimeStore units;
        ResourceNodeStore resourceNodes;
        ProjectileStore projectiles;
        ResourceRuntimeStore resources;
        SelectionStore selection;
        double enemySpawnTimerSec = 0.0;
        double elapsedSec = 0.0;
        bool victory = false;
        bool defeat = false;
    };
}
