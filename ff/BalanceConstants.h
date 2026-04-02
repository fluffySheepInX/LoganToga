# pragma once
# include <array>
# include <Siv3D.hpp>

namespace ff
{
	inline constexpr Size MapSize{ 24, 24 };
	inline constexpr Vec2 TileHalfSize{ 48, 24 };
	inline constexpr int32 TileWidth = 96;
	inline constexpr int32 TileHeight = 48;
	inline constexpr int32 TileThickness = 17;
	inline constexpr double PlayerSpeed = 3.8;
	inline constexpr double PlayerFootOffsetY = 0.0;
	inline constexpr double PlayerMaxHp = 18.0;
	inline constexpr double PlayerHitInvincibilityDuration = 0.55;
	inline constexpr int32 InitialResources = 8;
	inline constexpr int32 ResourcePerEnemyKill = 2;
	inline constexpr int32 BonusTileResourcePerEnemyKill = 4;
	inline constexpr int32 PenaltyTileResourcePerEnemyKill = 1;
	inline constexpr double HasteTileAllyAttackIntervalMultiplier = 0.72;
	inline constexpr double RushTileBoostDuration = 5.0;
	inline constexpr double RushTilePlayerSpeedMultiplier = 1.55;
	inline constexpr double RushTileAllySpeedMultiplier = 1.35;
	inline constexpr double SpecialTileVisibleDuration = 7.0;
	inline constexpr double SpecialTileHiddenDuration = 5.0;
	inline constexpr double WaveStartDelay = 1.8;
	inline constexpr double WaveClearDelay = 4.0;
	inline constexpr double StageClearReturnDelay = 3.2;
	inline constexpr int32 BaseEnemiesPerWave = 5;
	inline constexpr int32 AdditionalEnemiesPerWave = 2;
	inline constexpr double EnemySpawnIntervalStepPerWave = 0.12;
	inline constexpr double MinEnemySpawnInterval = 0.55;
	inline constexpr double WaveBannerDuration = 1.6;
	inline constexpr double WaterBoundaryHalfWidth = 10.0;
	inline constexpr double EnemySpeed = 2.1;
	inline constexpr double EnemySpawnInterval = 1.8;
	inline constexpr double EnemyStopDistance = 0.35;
	inline constexpr double EnemyMaxHp = 4.0;
	inline constexpr double EnemyAttackRange = 0.9;
	inline constexpr double EnemyAttackInterval = 0.95;
	inline constexpr double EnemyAttackDamage = 1.0;
	inline constexpr size_t MaxEnemyCount = 24;
	inline constexpr int32 MidBossWaveInterval = 5;
	inline constexpr int32 FinalBossWave = 20;
	inline constexpr int32 MidBossBaseSupportEnemyCount = 3;
	inline constexpr double MidBossMaxHp = 24.0;
	inline constexpr double MidBossSpeed = 1.7;
	inline constexpr double MidBossAttackRange = 1.0;
	inline constexpr double MidBossAttackInterval = 0.88;
	inline constexpr double MidBossAttackDamage = 2.0;
	inline constexpr int32 MidBossRewardMultiplier = 6;
	inline constexpr double TrueBossMaxHp = 72.0;
	inline constexpr double TrueBossSpeed = 1.45;
	inline constexpr double TrueBossAttackRange = 1.2;
	inline constexpr double TrueBossAttackInterval = 0.72;
	inline constexpr double TrueBossAttackDamage = 3.0;
	inline constexpr int32 TrueBossRewardMultiplier = 20;
	inline constexpr double AllySpeed = 2.8;
	inline constexpr double AllyStopDistance = 0.2;
	inline constexpr double AllyOrbitRadius = 1.4;
	inline constexpr double AllyMaxHp = 6.0;
	inline constexpr double AllyAttackRange = 1.15;
	inline constexpr double AllyAttackInterval = 0.55;
	inline constexpr double AllyAttackDamage = 1.0;
	inline constexpr double PlayerCommandRadius = 2.35;
	inline constexpr double PlayerCommandAttackIntervalMultiplier = 0.82;
	inline constexpr double PlayerCommandDamageMultiplier = 1.25;
	inline constexpr double FixedTurretMaxHp = 10.0;
	inline constexpr double FixedTurretAttackRange = 3.2;
	inline constexpr double FixedTurretAttackInterval = 1.45;
	inline constexpr double FixedTurretAttackDamage = 3.0;
	inline constexpr int32 EnemyDefinitionSchemaVersion = 1;
	inline constexpr int32 WaveDefinitionSchemaVersion = 1;
	inline constexpr int32 UnitDefinitionSchemaVersion = 1;
}
