#pragma once
# include <Siv3D.hpp>

enum class Owner
{
    Player,
    Enemy
};

enum class UnitArchetype
{
    Base,
    Worker,
    Soldier,
    Archer
};

enum class UnitOrderType
{
    Idle,
    Move,
    AttackTarget
};

struct UnitOrder
{
    UnitOrderType type = UnitOrderType::Idle;
    s3d::Vec2 targetPoint = s3d::Vec2::Zero();
    s3d::Optional<s3d::int32> targetUnitId;
};

struct UnitState
{
    s3d::int32 id = -1;
    Owner owner = Owner::Player;
    UnitArchetype archetype = UnitArchetype::Soldier;
    s3d::Vec2 position = s3d::Vec2::Zero();
    s3d::Vec2 moveTarget = s3d::Vec2::Zero();
    double radius = 12.0;
    double moveSpeed = 80.0;
    double attackRange = 24.0;
    double attackCooldown = 0.7;
    double attackCooldownRemaining = 0.0;
    s3d::int32 attackPower = 8;
    s3d::int32 hp = 40;
    s3d::int32 maxHp = 40;
    bool canMove = true;
    bool isSelected = false;
    bool isAlive = true;
    UnitOrder order;
};

struct BattleState
{
    s3d::RectF worldBounds{ 0, 0, 1280, 720 };
    s3d::Array<UnitState> units;
    bool isSelecting = false;
    s3d::Vec2 selectionStart = s3d::Vec2::Zero();
    s3d::RectF selectionRect{ 0, 0, 0, 0 };
    s3d::int32 nextUnitId = 1;
    s3d::int32 playerGold = 200;
    s3d::int32 enemyGold = 200;
    double playerIncomeTimer = 0.0;
    double enemyIncomeTimer = 0.0;
    double enemySpawnTimer = 0.0;
    s3d::Optional<Owner> winner;

    [[nodiscard]] UnitState* findUnit(const s3d::int32 id)
    {
        for (auto& unit : units)
        {
            if (unit.id == id)
            {
                return &unit;
            }
        }

        return nullptr;
    }

    [[nodiscard]] const UnitState* findUnit(const s3d::int32 id) const
    {
        for (const auto& unit : units)
        {
            if (unit.id == id)
            {
                return &unit;
            }
        }

        return nullptr;
    }
};
