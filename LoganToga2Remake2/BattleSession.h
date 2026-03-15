#pragma once

#include <unordered_map>

#include "BattleCommands.h"
#include "BattleConfig.h"
#include "BattleState.h"

class BattleSession
{
public:
	BattleSession();
	explicit BattleSession(const BattleConfigData& config);

	void reset(const BattleConfigData& config);
	void enqueue(BattleCommand command);
	void update(double deltaTime);

	[[nodiscard]] const BattleState& state() const noexcept;
	[[nodiscard]] BattleState& state() noexcept;
	[[nodiscard]] const BattleConfigData& config() const noexcept;
	[[nodiscard]] Array<int32> getSelectedPlayerUnitIds() const;
	[[nodiscard]] Optional<int32> findSelectedPlayerWorkerId() const;
	[[nodiscard]] Optional<int32> findSelectedPlayerTurretId() const;
	[[nodiscard]] Optional<int32> findPlayerUnitAt(const Vec2& position) const;
	[[nodiscard]] Optional<int32> findPlayerBuildingAt(const Vec2& position) const;
	[[nodiscard]] Optional<int32> findEnemyAt(const Vec2& position) const;
	[[nodiscard]] Optional<int32> findEnemyNear(const Vec2& position, double snapRadius) const;
	bool trySpawnPlayerUnit(UnitArchetype archetype);
	bool tryUpgradeSelectedTurret(TurretUpgradeType type);
	bool cancelLastPlayerProduction();
	void toggleEnemyAiDebugPanel();
	void cycleEnemyAiDebugMode();
	void setEnemyAiDebugOverrideMode(const Optional<EnemyAiMode>& mode);

private:
	BattleConfigData m_config;
	BattleState m_state;
	Array<BattleCommand> m_pendingCommands;
	mutable std::unordered_map<int32, size_t> m_unitIndexById;
	mutable std::unordered_map<int32, size_t> m_buildingIndexByUnitId;
	mutable bool m_unitIndexDirty = true;
	mutable bool m_buildingIndexDirty = true;
	mutable Array<size_t> m_playerUnitIndices;
	mutable Array<size_t> m_enemyUnitIndices;
	mutable Array<size_t> m_playerBuildingIndices;
	mutable Array<size_t> m_enemyBuildingIndices;
	mutable bool m_frameUnitCacheDirty = true;
	mutable RectF m_spatialQueryBounds{ 0, 0, 0, 0 };
	mutable double m_spatialQueryCellSize = 128.0;
	mutable int32 m_spatialQueryColumns = 1;
	mutable int32 m_spatialQueryRows = 1;
	mutable Array<Array<size_t>> m_playerSpatialUnitIndices;
	mutable Array<Array<size_t>> m_enemySpatialUnitIndices;
	mutable Array<size_t> m_nearbyOpponentIndicesScratch;
	mutable Array<size_t> m_nearbyUnitIndicesScratch;
	mutable bool m_spatialQueryCacheDirty = true;
	mutable RectF m_navigationGridBounds{ 0, 0, 0, 0 };
	mutable double m_navigationGridCellSize = 24.0;
	mutable int32 m_navigationGridColumns = 1;
	mutable int32 m_navigationGridRows = 1;
	mutable Array<char> m_navigationGridBlocked;
	mutable bool m_navigationGridDirty = true;

	void setupInitialState();
	void processCommands();
	void updateEconomy(double deltaTime);
	void updateProduction(double deltaTime);
	void updateEnemyAI(double deltaTime);
	void updateMovement(double deltaTime);
	void updateConstructionOrders();
	void updateResourcePoints(double deltaTime);
	void updateCombat();
	void cleanupDeadUnits();
	void updateVictoryState();
	void assignFormationMove(const Array<int32>& unitIds, const Vec2& destination, FormationType formation, const Vec2& facingDirection);
	void cancelPendingConstructionOrders(const Array<int32>& unitIds, bool refundReservedCost);
	void removeUnitsFromSquads(const Array<int32>& unitIds);
	void cleanupSquads();
	[[nodiscard]] bool tryPlaceBuilding(Owner owner, UnitArchetype archetype, const Vec2& position, Optional<int32> builderUnitId = none, bool chargeCost = true);
	[[nodiscard]] bool canPlaceBuilding(Owner owner, UnitArchetype archetype, const Vec2& position, Optional<int32> ignoredUnitId = none) const;
	int32 spawnUnit(Owner owner, UnitArchetype archetype, const Vec2& position);

	[[nodiscard]] BuildingState* findProductionBuilding(Owner owner, UnitArchetype producerArchetype);
	[[nodiscard]] const BuildingState* findProductionBuilding(Owner owner, UnitArchetype producerArchetype) const;
	[[nodiscard]] const ProductionSlot* findProductionSlot(UnitArchetype archetype) const;
	[[nodiscard]] bool tryQueueUnitProduction(Owner owner, UnitArchetype archetype);
	[[nodiscard]] Vec2 getProductionSpawnPoint(const UnitState& buildingUnit, UnitArchetype archetype) const;
	[[nodiscard]] UnitState makeUnit(int32 id, Owner owner, UnitArchetype archetype, const Vec2& position) const;
	[[nodiscard]] const UnitDefinition& getUnitDefinition(UnitArchetype archetype) const;
	[[nodiscard]] int32 getUnitCost(UnitArchetype archetype) const;
	[[nodiscard]] double getProductionTime(Owner owner, UnitArchetype archetype) const;
	[[nodiscard]] double getAggroRange(Owner owner, UnitArchetype archetype) const;
	[[nodiscard]] const UnitState* findNearestEnemy(const UnitState& source) const;
	[[nodiscard]] const UnitState* findBestKatyushaTarget(const UnitState& source) const;
	[[nodiscard]] const UnitState* tryReacquireCombatTarget(const UnitState& source, UnitOrder& order) const;
	void applyUnitHpDelta(UnitState& target, int32 hpDelta);
	void triggerGoliathExplosion(UnitState& unit);
	void invalidateUnitIndex() noexcept;
	void invalidateBuildingIndex() noexcept;
	void invalidateSpatialQueryCache() noexcept;
	void invalidateNavigationGrid() noexcept;
	void rebuildUnitIndex() const;
	void rebuildBuildingIndex() const;
	void rebuildFrameUnitCache() const;
	void rebuildSpatialQueryCache() const;
	void rebuildNavigationGrid() const;
	void updateTutorial(double deltaTime);
	void beginTutorialPhase(TutorialPhase phase, double timer = 0.0);
	void spawnTutorialEnemyWave();
	void gatherNearbyUnitIndices(Owner owner, const Vec2& center, double searchRadius, Array<size_t>& indices) const;
	void gatherNearbyOpponentIndices(const UnitState& source, double searchRadius, Array<size_t>& indices) const;
	[[nodiscard]] const Array<size_t>& getOwnerUnitIndices(Owner owner) const;
	[[nodiscard]] const Array<size_t>& getOwnerBuildingIndices(Owner owner) const;
	[[nodiscard]] const UnitState* findOwnerUnitByArchetype(Owner owner, UnitArchetype archetype) const;
	[[nodiscard]] const UnitState* findNearestIntrudingPlayerUnit(const Vec2& assetPosition, double defenseRadius, double& inOutDistanceSq) const;
	[[nodiscard]] bool hasBaseDefenseTurret(const UnitState& base, double lockRadius) const;
	[[nodiscard]] UnitState* findCachedUnit(int32 id);
	[[nodiscard]] const UnitState* findCachedUnit(int32 id) const;
	[[nodiscard]] BuildingState* findCachedBuilding(int32 unitId);
	[[nodiscard]] const BuildingState* findCachedBuilding(int32 unitId) const;
	void resetEnemyAiAssaultState();
};
