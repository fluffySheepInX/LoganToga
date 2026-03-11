# include "BattleSession.h"
# include <algorithm>

namespace
{
    s3d::Vec2 ClampToWorld(const s3d::RectF& bounds, const s3d::Vec2& position, const double radius)
    {
        return {
            s3d::Clamp(position.x, bounds.leftX() + radius, bounds.rightX() - radius),
            s3d::Clamp(position.y, bounds.topY() + radius, bounds.bottomY() - radius)
        };
    }

    bool IsEnemy(const UnitState& lhs, const UnitState& rhs)
    {
        return lhs.owner != rhs.owner;
    }
}

BattleSession::BattleSession()
{
    setupInitialState();
}

void BattleSession::enqueue(BattleCommand command)
{
    m_pendingCommands << std::move(command);
}

void BattleSession::update(const double deltaTime)
{
    if (m_state.winner)
    {
        return;
    }

    processCommands();
    updateEconomy(deltaTime);
    updateEnemyAI();
    updateMovement(deltaTime);
    updateCombat(deltaTime);
    cleanupDeadUnits();
    updateVictoryState();
}

const BattleState& BattleSession::state() const noexcept
{
    return m_state;
}

BattleState& BattleSession::state() noexcept
{
    return m_state;
}

s3d::Array<s3d::int32> BattleSession::getSelectedPlayerUnitIds() const
{
    s3d::Array<s3d::int32> selected;
    for (const auto& unit : m_state.units)
    {
        if (unit.isAlive && unit.isSelected && (unit.owner == Owner::Player) && (unit.archetype != UnitArchetype::Base))
        {
            selected << unit.id;
        }
    }

    return selected;
}

s3d::Optional<s3d::int32> BattleSession::findEnemyAt(const s3d::Vec2& position) const
{
    for (auto it = m_state.units.rbegin(); it != m_state.units.rend(); ++it)
    {
        if (it->isAlive && (it->owner == Owner::Enemy) && s3d::Circle{ it->position, it->radius + 4 }.intersects(position))
        {
            return it->id;
        }
    }

    return s3d::none;
}

bool BattleSession::trySpawnPlayerUnit(const UnitArchetype archetype)
{
    const s3d::int32 cost = getUnitCost(archetype);
    if ((cost <= 0) || (m_state.playerGold < cost) || m_state.winner)
    {
        return false;
    }

    m_state.playerGold -= cost;
    spawnUnit(Owner::Player, archetype, s3d::Vec2{ 130, 520 + s3d::Random(-40.0, 40.0) });
    return true;
}

void BattleSession::setupInitialState()
{
    spawnUnit(Owner::Player, UnitArchetype::Base, s3d::Vec2{ 110, 540 });
    spawnUnit(Owner::Enemy, UnitArchetype::Base, s3d::Vec2{ 1170, 180 });

    spawnUnit(Owner::Player, UnitArchetype::Soldier, s3d::Vec2{ 180, 500 });
    spawnUnit(Owner::Player, UnitArchetype::Soldier, s3d::Vec2{ 200, 560 });
    spawnUnit(Owner::Player, UnitArchetype::Archer, s3d::Vec2{ 210, 530 });

    spawnUnit(Owner::Enemy, UnitArchetype::Soldier, s3d::Vec2{ 1020, 170 });
    spawnUnit(Owner::Enemy, UnitArchetype::Soldier, s3d::Vec2{ 1040, 230 });
    spawnUnit(Owner::Enemy, UnitArchetype::Archer, s3d::Vec2{ 980, 200 });
}

void BattleSession::processCommands()
{
    for (const auto& command : m_pendingCommands)
    {
        std::visit([this](const auto& value)
        {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, ClearSelectionCommand>)
            {
                for (auto& unit : m_state.units)
                {
                    unit.isSelected = false;
                }
            }
            else if constexpr (std::is_same_v<T, SelectUnitsInRectCommand>)
            {
                if (!value.additive)
                {
                    for (auto& unit : m_state.units)
                    {
                        unit.isSelected = false;
                    }
                }

                for (auto& unit : m_state.units)
                {
                    if (!unit.isAlive || (unit.owner != Owner::Player) || (unit.archetype == UnitArchetype::Base))
                    {
                        continue;
                    }

                    if (value.rect.intersects(unit.position))
                    {
                        unit.isSelected = true;
                    }
                }
            }
            else if constexpr (std::is_same_v<T, MoveUnitsCommand>)
            {
                const s3d::int32 count = static_cast<s3d::int32>(value.unitIds.size());
                const double spacing = 26.0;
                const double startOffset = -((count - 1) * spacing * 0.5);

                for (s3d::int32 i = 0; i < count; ++i)
                {
                    if (auto* unit = m_state.findUnit(value.unitIds[i]))
                    {
                        if (!unit->isAlive || !unit->canMove)
                        {
                            continue;
                        }

                        unit->order.type = UnitOrderType::Move;
                        unit->order.targetUnitId.reset();
                        unit->order.targetPoint = value.destination + s3d::Vec2{ 0, startOffset + spacing * i };
                        unit->moveTarget = unit->order.targetPoint;
                    }
                }
            }
            else if constexpr (std::is_same_v<T, AttackUnitCommand>)
            {
                for (const auto unitId : value.unitIds)
                {
                    if (auto* unit = m_state.findUnit(unitId))
                    {
                        if (!unit->isAlive)
                        {
                            continue;
                        }

                        unit->order.type = UnitOrderType::AttackTarget;
                        unit->order.targetUnitId = value.targetUnitId;
                    }
                }
            }
        }, command);
    }

    m_pendingCommands.clear();
}

void BattleSession::updateEconomy(const double deltaTime)
{
    m_state.playerIncomeTimer += deltaTime;
    m_state.enemyIncomeTimer += deltaTime;
    m_state.enemySpawnTimer += deltaTime;

    if (m_state.playerIncomeTimer >= 1.0)
    {
        m_state.playerIncomeTimer -= 1.0;
        m_state.playerGold += 20;
    }

    if (m_state.enemyIncomeTimer >= 1.0)
    {
        m_state.enemyIncomeTimer -= 1.0;
        m_state.enemyGold += 20;
    }

    if (m_state.enemySpawnTimer >= 4.0)
    {
        m_state.enemySpawnTimer -= 4.0;
        const UnitArchetype archetype = (m_state.enemyGold >= getUnitCost(UnitArchetype::Archer) && s3d::RandomBool(0.4))
            ? UnitArchetype::Archer
            : UnitArchetype::Soldier;
        const s3d::int32 cost = getUnitCost(archetype);
        if (m_state.enemyGold >= cost)
        {
            m_state.enemyGold -= cost;
            spawnUnit(Owner::Enemy, archetype, s3d::Vec2{ 1130, 170 + s3d::Random(-50.0, 50.0) });
        }
    }
}

void BattleSession::updateEnemyAI()
{
    for (auto& unit : m_state.units)
    {
        if (!unit.isAlive || (unit.owner != Owner::Enemy) || !unit.canMove)
        {
            continue;
        }

        const UnitState* target = findNearestEnemy(m_state, unit);
        if (target)
        {
            unit.order.type = UnitOrderType::AttackTarget;
            unit.order.targetUnitId = target->id;
            unit.order.targetPoint = target->position;
        }
    }
}

void BattleSession::updateMovement(const double deltaTime)
{
    for (auto& unit : m_state.units)
    {
        if (!unit.isAlive)
        {
            continue;
        }

        unit.attackCooldownRemaining = s3d::Max(unit.attackCooldownRemaining - deltaTime, 0.0);

        if (!unit.canMove)
        {
            continue;
        }

        s3d::Vec2 destination = unit.position;
        double stopDistance = 4.0;

        if ((unit.order.type == UnitOrderType::Move) || (unit.order.type == UnitOrderType::AttackTarget))
        {
            destination = unit.order.targetPoint;
        }

        if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
        {
            if (const auto* target = m_state.findUnit(*unit.order.targetUnitId))
            {
                if (target->isAlive && IsEnemy(unit, *target))
                {
                    destination = target->position;
                    stopDistance = s3d::Max(unit.attackRange - 2.0, 1.0);
                    unit.order.targetPoint = destination;
                }
                else
                {
                    unit.order.type = UnitOrderType::Idle;
                    unit.order.targetUnitId.reset();
                }
            }
            else
            {
                unit.order.type = UnitOrderType::Idle;
                unit.order.targetUnitId.reset();
            }
        }

        const s3d::Vec2 delta = destination - unit.position;
        const double distance = delta.length();

        if (distance <= stopDistance)
        {
            if (unit.order.type == UnitOrderType::Move)
            {
                unit.order.type = UnitOrderType::Idle;
            }
            continue;
        }

        const s3d::Vec2 velocity = delta.normalized() * unit.moveSpeed * deltaTime;
        if (velocity.length() >= distance)
        {
            unit.position = destination;
        }
        else
        {
            unit.position += velocity;
        }

        unit.position = ClampToWorld(m_state.worldBounds, unit.position, unit.radius);
    }
}

void BattleSession::updateCombat(const double)
{
    struct DamageEvent
    {
        s3d::int32 targetId = -1;
        s3d::int32 damage = 0;
    };

    s3d::Array<DamageEvent> damageEvents;

    for (auto& unit : m_state.units)
    {
        if (!unit.isAlive)
        {
            continue;
        }

        const UnitState* target = nullptr;

        if ((unit.order.type == UnitOrderType::AttackTarget) && unit.order.targetUnitId)
        {
            target = m_state.findUnit(*unit.order.targetUnitId);
            if (!(target && target->isAlive && IsEnemy(unit, *target)))
            {
                target = nullptr;
                unit.order.type = UnitOrderType::Idle;
                unit.order.targetUnitId.reset();
            }
        }

        if (!target)
        {
            target = findNearestEnemy(m_state, unit);
        }

        if (!target)
        {
            continue;
        }

        if (unit.position.distanceFrom(target->position) > unit.attackRange)
        {
            continue;
        }

        if (unit.attackCooldownRemaining > 0.0)
        {
            continue;
        }

        damageEvents << DamageEvent{ target->id, unit.attackPower };
        unit.attackCooldownRemaining = unit.attackCooldown;
    }

    for (const auto& event : damageEvents)
    {
        if (auto* target = m_state.findUnit(event.targetId))
        {
            target->hp -= event.damage;
            if (target->hp <= 0)
            {
                target->hp = 0;
                target->isAlive = false;
            }
        }
    }
}

void BattleSession::cleanupDeadUnits()
{
    for (auto& unit : m_state.units)
    {
        if (!unit.isAlive)
        {
            unit.isSelected = false;
        }
    }

    m_state.units.remove_if([](const UnitState& unit)
    {
        return !unit.isAlive && (unit.archetype != UnitArchetype::Base);
    });
}

void BattleSession::updateVictoryState()
{
    bool hasPlayerBase = false;
    bool hasEnemyBase = false;

    for (const auto& unit : m_state.units)
    {
        if (!unit.isAlive || (unit.archetype != UnitArchetype::Base))
        {
            continue;
        }

        if (unit.owner == Owner::Player)
        {
            hasPlayerBase = true;
        }
        else
        {
            hasEnemyBase = true;
        }
    }

    if (!hasEnemyBase)
    {
        m_state.winner = Owner::Player;
    }
    else if (!hasPlayerBase)
    {
        m_state.winner = Owner::Enemy;
    }
}

s3d::int32 BattleSession::spawnUnit(const Owner owner, const UnitArchetype archetype, const s3d::Vec2& position)
{
    const s3d::int32 id = m_state.nextUnitId++;
    m_state.units << makeUnit(id, owner, archetype, position);
    return id;
}

UnitState BattleSession::makeUnit(const s3d::int32 id, const Owner owner, const UnitArchetype archetype, const s3d::Vec2& position)
{
    UnitState unit;
    unit.id = id;
    unit.owner = owner;
    unit.archetype = archetype;
    unit.position = position;
    unit.moveTarget = position;

    switch (archetype)
    {
    case UnitArchetype::Base:
        unit.radius = 28.0;
        unit.moveSpeed = 0.0;
        unit.attackRange = 0.0;
        unit.attackCooldown = 1.0;
        unit.attackPower = 0;
        unit.hp = 300;
        unit.maxHp = 300;
        unit.canMove = false;
        break;
    case UnitArchetype::Worker:
        unit.radius = 10.0;
        unit.moveSpeed = 85.0;
        unit.attackRange = 18.0;
        unit.attackCooldown = 1.0;
        unit.attackPower = 4;
        unit.hp = 28;
        unit.maxHp = 28;
        break;
    case UnitArchetype::Soldier:
        unit.radius = 12.0;
        unit.moveSpeed = 92.0;
        unit.attackRange = 24.0;
        unit.attackCooldown = 0.7;
        unit.attackPower = 8;
        unit.hp = 48;
        unit.maxHp = 48;
        break;
    case UnitArchetype::Archer:
        unit.radius = 11.0;
        unit.moveSpeed = 82.0;
        unit.attackRange = 120.0;
        unit.attackCooldown = 1.0;
        unit.attackPower = 6;
        unit.hp = 34;
        unit.maxHp = 34;
        break;
    }

    return unit;
}

s3d::int32 BattleSession::getUnitCost(const UnitArchetype archetype)
{
    switch (archetype)
    {
    case UnitArchetype::Worker:
        return 40;
    case UnitArchetype::Soldier:
        return 60;
    case UnitArchetype::Archer:
        return 80;
    case UnitArchetype::Base:
    default:
        return 0;
    }
}

double BattleSession::getAggroRange(const UnitArchetype archetype)
{
    switch (archetype)
    {
    case UnitArchetype::Archer:
        return 220.0;
    case UnitArchetype::Base:
        return 0.0;
    default:
        return 170.0;
    }
}

const UnitState* BattleSession::findNearestEnemy(const BattleState& state, const UnitState& source)
{
    const UnitState* nearest = nullptr;
    double nearestDistance = getAggroRange(source.archetype);

    for (const auto& candidate : state.units)
    {
        if (!candidate.isAlive || !IsEnemy(source, candidate))
        {
            continue;
        }

        const double distance = source.position.distanceFrom(candidate.position);
        if (distance <= nearestDistance)
        {
            nearestDistance = distance;
            nearest = &candidate;
        }
    }

    return nearest;
}
