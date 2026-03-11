#pragma once
# include <Siv3D.hpp>
# include "BattleState.h"
# include "Commands.h"

class BattleSession
{
public:
    BattleSession();

    void enqueue(BattleCommand command);
    void update(double deltaTime);

    [[nodiscard]] const BattleState& state() const noexcept;
    [[nodiscard]] BattleState& state() noexcept;

    [[nodiscard]] s3d::Array<s3d::int32> getSelectedPlayerUnitIds() const;
    [[nodiscard]] s3d::Optional<s3d::int32> findEnemyAt(const s3d::Vec2& position) const;

    bool trySpawnPlayerUnit(UnitArchetype archetype);

private:
    BattleState m_state;
    s3d::Array<BattleCommand> m_pendingCommands;

    void setupInitialState();
    void processCommands();
    void updateEconomy(double deltaTime);
    void updateEnemyAI();
    void updateMovement(double deltaTime);
    void updateCombat(double deltaTime);
    void cleanupDeadUnits();
    void updateVictoryState();

    s3d::int32 spawnUnit(Owner owner, UnitArchetype archetype, const s3d::Vec2& position);
    static UnitState makeUnit(s3d::int32 id, Owner owner, UnitArchetype archetype, const s3d::Vec2& position);
    [[nodiscard]] static s3d::int32 getUnitCost(UnitArchetype archetype);
    [[nodiscard]] static double getAggroRange(UnitArchetype archetype);
    [[nodiscard]] static const UnitState* findNearestEnemy(const BattleState& state, const UnitState& source);
};
