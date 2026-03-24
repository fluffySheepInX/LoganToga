#pragma once

#include "BattleConfigTypes.h"
#include "BattleStateHelpers.h"

struct UnitState
{
	int32 id = -1;
	Owner owner = Owner::Player;
	UnitArchetype archetype = UnitArchetype::Soldier;
	Vec2 position = Vec2::Zero();
	Vec2 previousPosition = Vec2::Zero();
	Vec2 moveTarget = Vec2::Zero();
	Array<Vec2> pathPoints;
	int32 pathIndex = 0;
	Vec2 pathDestination = Vec2::Zero();
	bool pathDirty = false;
	int32 pathStuckFrames = 0;
	double radius = 12.0;
	double moveSpeed = 80.0;
	double attackRange = 24.0;
	double attackCooldown = 0.7;
	double attackCooldownRemaining = 0.0;
	double damageFlashTime = 0.0;
	double movementDistanceLastFrame = 0.0;
	int32 attackPower = 8;
	int32 hp = 40;
	int32 maxHp = 40;
	bool canMove = true;
	bool isDetonating = false;
	int32 detonationFramesRemaining = 0;
	bool isSelected = false;
	bool isAlive = true;
    bool enemyAiAwaitingRallyAssignment = false;
	Optional<int32> squadId;
	Vec2 formationOffset = Vec2::Zero();
	UnitOrder order;
};

struct ProductionQueueItem
{
	UnitArchetype archetype = UnitArchetype::Soldier;
	double remainingTime = 0.0;
	double totalTime = 0.0;
	int32 batchCount = 1;
	int32 queuedCost = 0;
};

struct BuildingState
{
	int32 unitId = -1;
	bool isConstructed = true;
	double constructionRemaining = 0.0;
	double constructionTotal = 0.0;
	Array<ProductionQueueItem> productionQueue;
	Optional<TurretUpgradeType> turretUpgrade;
};

struct ResourcePointState
{
	String label = U"Resource";
	Vec2 position = Vec2::Zero();
	double radius = 42.0;
	int32 incomeAmount = 10;
	double captureTime = 2.0;
	Owner owner = Owner::Neutral;
	Optional<Owner> capturingOwner;
	double captureProgress = 0.0;
};

struct SquadState
{
	int32 id = -1;
	Owner owner = Owner::Player;
	FormationType formation = FormationType::Line;
	Vec2 destination = Vec2::Zero();
	Array<int32> unitIds;
};

struct AttackVisualEffect
{
	int32 sourceUnitId = -1;
	Vec2 start = Vec2::Zero();
	Vec2 end = Vec2::Zero();
	Owner owner = Owner::Player;
	UnitArchetype sourceArchetype = UnitArchetype::Soldier;
	int32 framesRemaining = 0;
	int32 totalFrames = 0;
	double areaRadius = 0.0;
};

enum class BattleAudioEventKind
{
	Hit,
	Death,
	Explosion,
};

struct BattleAudioEvent
{
	Vec2 position = Vec2::Zero();
	Owner targetOwner = Owner::Player;
	UnitArchetype sourceArchetype = UnitArchetype::Soldier;
	bool isBuilding = false;
	BattleAudioEventKind kind = BattleAudioEventKind::Hit;
};

struct DeathVisualEffect
{
	Vec2 position = Vec2::Zero();
	double radius = 12.0;
	Owner owner = Owner::Player;
	UnitArchetype archetype = UnitArchetype::Soldier;
	bool isBuilding = false;
	double remainingTime = 0.0;
	double totalTime = 0.0;
};

struct ProductionCompletionEffect
{
	int32 unitId = -1;
	double remainingTime = 0.0;
	double totalTime = 0.0;
};

struct PendingConstructionOrder
{
	int32 workerUnitId = -1;
	UnitArchetype archetype = UnitArchetype::Barracks;
	Vec2 position = Vec2::Zero();
	int32 reservedCost = 0;
};

struct BattleState
{
	RectF worldBounds{ 0, 0, 1280, 720 };
	Array<UnitState> units;
	Array<BuildingState> buildings;
	Array<ResourcePointState> resourcePoints;
	Array<SquadState> squads;
	Array<AttackVisualEffect> attackVisualEffects;
	Array<BattleAudioEvent> battleAudioEvents;
	Array<DeathVisualEffect> deathVisualEffects;
	Array<ProductionCompletionEffect> productionCompletionEffects;
	Array<PendingConstructionOrder> pendingConstructionOrders;
	bool isSelecting = false;
	Vec2 selectionStart = Vec2::Zero();
	RectF selectionRect{ 0, 0, 0, 0 };
	bool isCommandDragging = false;
	Vec2 commandDragStart = Vec2::Zero();
	Vec2 commandDragCurrent = Vec2::Zero();
	Optional<UnitArchetype> pendingConstructionArchetype;
	Vec2 buildingPreviewPosition = Vec2::Zero();
	bool pendingRepairTargeting = false;
	Vec2 repairPreviewPosition = Vec2::Zero();
	int32 nextUnitId = 1;
	int32 nextSquadId = 1;
	int32 playerGold = 200;
	int32 enemyGold = 200;
	double playerIncomeTimer = 0.0;
	double enemyIncomeTimer = 0.0;
	double enemySpawnTimer = 0.0;
	double enemyAiDecisionTimer = 0.0;
	bool enemyAiDebugPanelVisible = false;
	Optional<EnemyAiMode> enemyAiDebugOverrideMode;
	EnemyAiMode enemyAiResolvedMode = EnemyAiMode::Default;
	int32 enemyAiDebugCombatUnitCount = 0;
	int32 enemyAiDebugReadyUnitCount = 0;
	int32 enemyAiSearchPhase = 0;
	double enemyAiStagingTimer = 0.0;
  bool enemyAiStagingCompleted = false;
	double enemyAiAssaultCommitTimer = 0.0;
    bool enemyAiAssaultActive = false;
	Vec2 enemyAiAssaultDestination = Vec2::Zero();
	Optional<int32> enemyAiAssaultTargetUnitId;
	String statusMessage;
	double statusMessageTimer = 0.0;
	FormationType playerFormation = FormationType::Line;
	bool tutorialActive = false;
	TutorialPhase tutorialPhase = TutorialPhase::None;
	String tutorialObjective;
	double tutorialPhaseTimer = 0.0;
	Optional<int32> tutorialWorkerUnitId;
	int32 tutorialProducedUnitCount = 0;
	bool tutorialEnemyWaveStarted = false;
	Optional<Owner> winner;

	[[nodiscard]] UnitState* findUnit(const int32 id)
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

	[[nodiscard]] const UnitState* findUnit(const int32 id) const
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

	[[nodiscard]] BuildingState* findBuildingByUnitId(const int32 unitId)
	{
		for (auto& building : buildings)
		{
			if (building.unitId == unitId)
			{
				return &building;
			}
		}

		return nullptr;
	}

	[[nodiscard]] const BuildingState* findBuildingByUnitId(const int32 unitId) const
	{
		for (const auto& building : buildings)
		{
			if (building.unitId == unitId)
			{
				return &building;
			}
		}

		return nullptr;
	}
};

