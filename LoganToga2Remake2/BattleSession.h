#pragma once

#include "BattleConfig.h"

class BattleSession
{
public:
	BattleSession();

	void enqueue(BattleCommand command);
	void update(double deltaTime);

	[[nodiscard]] const BattleState& state() const noexcept;
	[[nodiscard]] BattleState& state() noexcept;
	[[nodiscard]] const BattleConfigData& config() const noexcept;
	[[nodiscard]] Array<int32> getSelectedPlayerUnitIds() const;
	[[nodiscard]] Optional<int32> findEnemyAt(const Vec2& position) const;
	bool trySpawnPlayerUnit(UnitArchetype archetype);
	bool cancelLastPlayerProduction();

private:
	BattleConfigData m_config;
	BattleState m_state;
	Array<BattleCommand> m_pendingCommands;

	void setupInitialState();
	void processCommands();
	void updateEconomy(double deltaTime);
	void updateProduction(double deltaTime);
	void updateEnemyAI();
	void updateMovement(double deltaTime);
	void updateCombat();
	void cleanupDeadUnits();
	void updateVictoryState();
	int32 spawnUnit(Owner owner, UnitArchetype archetype, const Vec2& position);

	[[nodiscard]] BuildingState* findProductionBuilding(Owner owner);
	[[nodiscard]] const BuildingState* findProductionBuilding(Owner owner) const;
	[[nodiscard]] bool tryQueueUnitProduction(Owner owner, UnitArchetype archetype);
	[[nodiscard]] Vec2 getProductionSpawnPoint(const UnitState& buildingUnit, UnitArchetype archetype) const;
	[[nodiscard]] UnitState makeUnit(int32 id, Owner owner, UnitArchetype archetype, const Vec2& position) const;
	[[nodiscard]] const UnitDefinition& getUnitDefinition(UnitArchetype archetype) const;
	[[nodiscard]] int32 getUnitCost(UnitArchetype archetype) const;
	[[nodiscard]] double getProductionTime(UnitArchetype archetype) const;
	[[nodiscard]] double getAggroRange(UnitArchetype archetype) const;
	[[nodiscard]] const UnitState* findNearestEnemy(const UnitState& source) const;
};
