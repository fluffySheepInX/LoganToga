#pragma once

#include "BattleTypes.h"

struct UnitDefinition
{
	UnitArchetype archetype = UnitArchetype::Soldier;
	String description;
	String flavorText;
	double radius = 12.0;
	double moveSpeed = 80.0;
	double attackRange = 24.0;
	double attackCooldown = 0.7;
	int32 attackPower = 8;
	int32 hp = 40;
	int32 cost = 60;
	double productionTime = 1.5;
	bool canMove = true;
	double aggroRange = 170.0;
};

struct PlayerUnitModifier
{
	UnitArchetype archetype = UnitArchetype::Soldier;
	int32 hpDelta = 0;
	int32 attackPowerDelta = 0;
	double moveSpeedDelta = 0.0;
	double attackRangeDelta = 0.0;
	double productionTimeDelta = 0.0;
};

struct InitialUnitPlacement
{
	Owner owner = Owner::Player;
	UnitArchetype archetype = UnitArchetype::Soldier;
	Vec2 position = Vec2::Zero();
};

struct ProductionSlot
{
	int32 slot = 1;
	UnitArchetype producer = UnitArchetype::Base;
	UnitArchetype archetype = UnitArchetype::Soldier;
	int32 cost = -1;
	int32 batchCount = 1;
};

struct ConstructionSlot
{
	int32 slot = 4;
	UnitArchetype archetype = UnitArchetype::Barracks;
};

struct TurretUpgradeDefinition
{
	TurretUpgradeType type = TurretUpgradeType::Power;
	int32 slot = 6;
	String label = U"POWER";
	String glyph = U"P";
	String description;
	String flavorText;
	int32 cost = 100;
	int32 attackPowerDelta = 0;
	double attackCooldownDelta = 0.0;
	bool unlockedByDefault = true;
};

struct EnemySpawnConfig
{
	double interval = 4.0;
	UnitArchetype basicArchetype = UnitArchetype::Soldier;
	UnitArchetype advancedArchetype = UnitArchetype::Archer;
	double advancedProbability = 0.4;
	Vec2 position = Vec2{ 1130, 170 };
	double randomYOffset = 50.0;
};

enum class EnemyAiMode
{
	Default,
	StagingAssault
};

struct EnemyAiConfig
{
	EnemyAiMode mode = EnemyAiMode::Default;
	double decisionInterval = 0.4;
	int32 assaultUnitThreshold = 4;
	double defenseRadius = 240.0;
	double rallyDistance = 120.0;
	double baseAssaultLockRadius = 180.0;
	int32 stagingAssaultMinUnits = 5;
	double stagingAssaultGatherRadius = 96.0;
	double stagingAssaultMaxWait = 4.0;
	double stagingAssaultCommitTime = 5.0;
	bool usePathfindingForAttackTarget = false;
};

struct DebugConfig
{
	bool enableEnemyAiSwitcher = false;
};

struct IncomeConfig
{
	double interval = 1.0;
	int32 playerAmount = 20;
	int32 enemyAmount = 20;
};

struct WorldConfig
{
	double width = 1280.0;
	double height = 720.0;
};

struct HudConfig
{
	String title = U"LoganToga2 Remake Prototype";
	String controls = U"L drag: select / R click: move or attack / Q-W: formation / 1-3: queue / 4: build / X: cancel";
	String escapeHint = U"Esc: back to title";
	String winHint = U"Enter: title / R: retry";
};

struct ObstacleConfig
{
	String label = U"Obstacle";
	RectF rect{ 0, 0, 96, 96 };
	bool blocksMovement = true;
};

struct ResourcePointConfig
{
	String label = U"Resource";
	Vec2 position = Vec2::Zero();
	double radius = 42.0;
	int32 incomeAmount = 10;
	double captureTime = 2.0;
	Owner owner = Owner::Neutral;
};

struct EnemyProgressionConfig
{
	int32 battle = 1;
	String mapSourcePath;
	int32 goldBonus = 0;
	int32 incomeBonus = 0;
	double spawnInterval = 0.0;
	int32 assaultUnitThreshold = 0;
	bool overrideEnemyAiMode = false;
	EnemyAiMode enemyAiMode = EnemyAiMode::Default;
	int32 stagingAssaultMinUnits = 0;
	double stagingAssaultGatherRadius = 0.0;
	double stagingAssaultMaxWait = 0.0;
	double stagingAssaultCommitTime = 0.0;
	int32 extraBasicUnits = 0;
	int32 extraAdvancedUnits = 0;
	bool replaceEnemyInitialUnits = false;
	Array<InitialUnitPlacement> enemyInitialUnits;
};

struct TutorialConfig
{
	bool enabled = false;
	Vec2 moveTarget = Vec2::Zero();
	double moveTargetRadius = 72.0;
	double prepareDelay = 1.8;
	double enemyWaveDelay = 6.0;
	UnitArchetype requiredConstruction = UnitArchetype::Barracks;
	UnitArchetype requiredProduction = UnitArchetype::Soldier;
	int32 requiredProductionCount = 1;
	String objectiveMove = U"Step 1: Select the Worker and move it forward.";
	String objectiveBuild = U"Step 2: Build a Barracks with the Worker.";
	String objectivePrepare = U"Enemy movement detected. Prepare to defend.";
	String objectiveProduce = U"Step 3: Produce a Soldier from the Barracks.";
	String objectiveDefend = U"Step 4: Stop the incoming enemy wave.";
	String objectiveComplete = U"Tutorial complete. Press Enter to return to title.";
	Array<InitialUnitPlacement> enemyWaveUnits;
};

struct BattleConfigData
{
	int32 playerGold = 200;
	int32 enemyGold = 200;
	WorldConfig world;
	IncomeConfig income;
	DebugConfig debug;
	HudConfig hud;
	Array<UnitDefinition> unitDefinitions;
	Array<InitialUnitPlacement> initialUnits;
	Array<ProductionSlot> playerProductionSlots;
	Array<ConstructionSlot> playerConstructionSlots;
	Array<TurretUpgradeDefinition> turretUpgradeDefinitions;
	Array<PlayerUnitModifier> playerUnitModifiers;
	Array<UnitArchetype> playerAvailableProductionArchetypes;
	Array<UnitArchetype> playerAvailableConstructionArchetypes;
	Array<TurretUpgradeType> playerAvailableTurretUpgrades;
	Array<ObstacleConfig> obstacles;
	Array<ResourcePointConfig> resourcePoints;
	Array<EnemyProgressionConfig> enemyProgression;
	EnemySpawnConfig enemySpawn;
	EnemyAiConfig enemyAI;
	TutorialConfig tutorial;
};
