# pragma once
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
  inline constexpr int32 InitialResources = 8;
	inline constexpr int32 ResourcePerEnemyKill = 2;
 inline constexpr int32 BonusTileResourcePerEnemyKill = 4;
	inline constexpr int32 PenaltyTileResourcePerEnemyKill = 1;
 inline constexpr double SpecialTileVisibleDuration = 7.0;
	inline constexpr double SpecialTileHiddenDuration = 5.0;
 inline constexpr double WaveStartDelay = 1.8;
	inline constexpr double WaveClearDelay = 4.0;
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
	inline constexpr double AllySpeed = 2.8;
	inline constexpr double AllyStopDistance = 0.2;
	inline constexpr double AllyOrbitRadius = 1.4;
   inline constexpr double AllyMaxHp = 6.0;
	inline constexpr double AllyAttackRange = 1.15;
	inline constexpr double AllyAttackInterval = 0.55;
	inline constexpr double AllyAttackDamage = 1.0;
	inline constexpr double FixedTurretMaxHp = 10.0;
	inline constexpr double FixedTurretAttackRange = 3.2;
	inline constexpr double FixedTurretAttackInterval = 1.45;
	inline constexpr double FixedTurretAttackDamage = 3.0;

	enum class TerrainTile : uint8
	{
		Grass,
		Path,
		Sand,
		Water,
	};

	using TerrainGrid = Grid<TerrainTile>;

	enum class SpecialTileKind : uint8
	{
		None,
		Bonus,
		Penalty,
	};

	using SpecialTileGrid = Grid<SpecialTileKind>;

	inline constexpr size_t ToTextureIndex(const TerrainTile tile)
	{
		return static_cast<size_t>(tile);
	}

	inline constexpr bool IsWaterTile(const TerrainTile tile)
	{
		return (tile == TerrainTile::Water);
	}

	inline constexpr bool IsLandTile(const TerrainTile tile)
	{
		return (not IsWaterTile(tile));
	}

	inline constexpr int32 GetResourceRewardPerEnemyKill(const SpecialTileKind tileKind)
	{
		switch (tileKind)
		{
		case SpecialTileKind::Bonus:
			return BonusTileResourcePerEnemyKill;

		case SpecialTileKind::Penalty:
			return PenaltyTileResourcePerEnemyKill;

		case SpecialTileKind::None:
		default:
			return ResourcePerEnemyKill;
		}
	}

	struct Enemy
	{
		Vec2 pos;
      double hp = EnemyMaxHp;
		double attackCooldown = 0.0;
	};

	enum class AllyBehavior : uint8
	{
		ChaseEnemies,
		HoldPosition,
		GuardPlayer,
		OrbitPlayer,
      FixedTurret,
	};

	inline constexpr double GetAllyMaxHp(const AllyBehavior behavior)
	{
		switch (behavior)
		{
		case AllyBehavior::FixedTurret:
			return FixedTurretMaxHp;

		case AllyBehavior::ChaseEnemies:
		case AllyBehavior::HoldPosition:
		case AllyBehavior::GuardPlayer:
		case AllyBehavior::OrbitPlayer:
		default:
			return AllyMaxHp;
		}
	}

	inline constexpr double GetAllyAttackRange(const AllyBehavior behavior)
	{
		switch (behavior)
		{
		case AllyBehavior::FixedTurret:
			return FixedTurretAttackRange;

		case AllyBehavior::ChaseEnemies:
		case AllyBehavior::HoldPosition:
		case AllyBehavior::GuardPlayer:
		case AllyBehavior::OrbitPlayer:
		default:
			return AllyAttackRange;
		}
	}

	inline constexpr double GetAllyAttackInterval(const AllyBehavior behavior)
	{
		switch (behavior)
		{
		case AllyBehavior::FixedTurret:
			return FixedTurretAttackInterval;

		case AllyBehavior::ChaseEnemies:
		case AllyBehavior::HoldPosition:
		case AllyBehavior::GuardPlayer:
		case AllyBehavior::OrbitPlayer:
		default:
			return AllyAttackInterval;
		}
	}

	inline constexpr double GetAllyAttackDamage(const AllyBehavior behavior)
	{
		switch (behavior)
		{
		case AllyBehavior::FixedTurret:
			return FixedTurretAttackDamage;

		case AllyBehavior::ChaseEnemies:
		case AllyBehavior::HoldPosition:
		case AllyBehavior::GuardPlayer:
		case AllyBehavior::OrbitPlayer:
		default:
			return AllyAttackDamage;
		}
	}

	inline constexpr int32 GetSummonCost(const AllyBehavior behavior)
	{
		switch (behavior)
		{
		case AllyBehavior::ChaseEnemies:
			return 3;

		case AllyBehavior::HoldPosition:
			return 2;

		case AllyBehavior::GuardPlayer:
			return 2;

		case AllyBehavior::OrbitPlayer:
			return 4;

		case AllyBehavior::FixedTurret:
			return 7;
		}

		return 2;
	}

	struct Ally
	{
		Vec2 pos;
		AllyBehavior behavior = AllyBehavior::GuardPlayer;
		double orbitAngle = 0.0;
        double hp = GetAllyMaxHp(AllyBehavior::GuardPlayer);
		double attackCooldown = 0.0;
	};

	struct WaterEdgeMask
	{
		bool upperRight = false;
		bool lowerRight = false;
		bool lowerLeft = false;
		bool upperLeft = false;
		bool topCornerOnly = false;
		bool bottomCornerOnly = false;
		bool rightCornerOnly = false;
		bool leftCornerOnly = false;
	};
}
