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

	enum class WaveTrait : uint8
	{
		None,
		Reinforced,
		Assault,
		Bounty,
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

	inline bool IsWithinPlayerCommandRange(const Vec2& unitPos, const Vec2& playerPos)
	{
		return (unitPos.distanceFrom(playerPos) <= PlayerCommandRadius);
	}

	inline StringView GetWaveTraitLabel(const WaveTrait trait)
	{
		switch (trait)
		{
		case WaveTrait::Reinforced:
			return U"装甲";

		case WaveTrait::Assault:
			return U"急襲";

		case WaveTrait::Bounty:
			return U"収穫";

		case WaveTrait::None:
		default:
			return U"";
		}
	}

	inline StringView GetWaveTraitDescription(const WaveTrait trait)
	{
		switch (trait)
		{
		case WaveTrait::Reinforced:
			return U"敵HP増加";

		case WaveTrait::Assault:
			return U"敵加速・攻撃短縮";

		case WaveTrait::Bounty:
			return U"撃破資源 +1";

		case WaveTrait::None:
		default:
			return U"";
		}
	}

	enum class EnemyKind : uint8
	{
		Normal,
		MidBoss,
		TrueBoss,
	};

	struct Enemy
	{
		Vec2 pos;
      EnemyKind kind = EnemyKind::Normal;
		double hp = EnemyMaxHp;
        double maxHp = EnemyMaxHp;
		double speedMultiplier = 1.0;
		double attackIntervalMultiplier = 1.0;
		double attackCooldown = 0.0;
	};

	inline constexpr double GetEnemyMaxHp(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossMaxHp;

		case EnemyKind::TrueBoss:
			return TrueBossMaxHp;

		case EnemyKind::Normal:
		default:
			return EnemyMaxHp;
		}
	}

	inline constexpr double GetEnemySpeed(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossSpeed;

		case EnemyKind::TrueBoss:
			return TrueBossSpeed;

		case EnemyKind::Normal:
		default:
			return EnemySpeed;
		}
	}

	inline constexpr double GetEnemyAttackRange(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackRange;

		case EnemyKind::TrueBoss:
			return TrueBossAttackRange;

		case EnemyKind::Normal:
		default:
			return EnemyAttackRange;
		}
	}

	inline constexpr double GetEnemyAttackInterval(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackInterval;

		case EnemyKind::TrueBoss:
			return TrueBossAttackInterval;

		case EnemyKind::Normal:
		default:
			return EnemyAttackInterval;
		}
	}

	inline constexpr double GetEnemyAttackDamage(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossAttackDamage;

		case EnemyKind::TrueBoss:
			return TrueBossAttackDamage;

		case EnemyKind::Normal:
		default:
			return EnemyAttackDamage;
		}
	}

	inline constexpr int32 GetEnemyRewardMultiplier(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return MidBossRewardMultiplier;

		case EnemyKind::TrueBoss:
			return TrueBossRewardMultiplier;

		case EnemyKind::Normal:
		default:
			return 1;
		}
	}

	enum class AllyBehavior : uint8
	{
		ChaseEnemies,
		HoldPosition,
		GuardPlayer,
		OrbitPlayer,
      FixedTurret,
	};

	inline constexpr size_t AllyBehaviorCount = 5;
	inline constexpr int32 WaveTraitSummonDiscountAmount = 1;
	using SummonDiscountTraitConfig = std::array<Array<WaveTrait>, AllyBehaviorCount>;

	inline constexpr size_t ToIndex(const AllyBehavior behavior)
	{
		return static_cast<size_t>(behavior);
	}

	inline SummonDiscountTraitConfig MakeDefaultSummonDiscountTraitConfig()
	{
		SummonDiscountTraitConfig config;
		config[ToIndex(AllyBehavior::ChaseEnemies)] = { WaveTrait::Reinforced };
		config[ToIndex(AllyBehavior::HoldPosition)] = { WaveTrait::Bounty };
		config[ToIndex(AllyBehavior::GuardPlayer)] = { WaveTrait::Assault };
		config[ToIndex(AllyBehavior::OrbitPlayer)] = { WaveTrait::Assault, WaveTrait::Bounty };
		config[ToIndex(AllyBehavior::FixedTurret)] = { WaveTrait::Reinforced };
		return config;
	}

	inline bool HasWaveTraitSummonDiscount(const SummonDiscountTraitConfig& config, const AllyBehavior behavior, const WaveTrait activeWaveTrait)
	{
		if (activeWaveTrait == WaveTrait::None)
		{
			return false;
		}

		const auto& discountedTraits = config[ToIndex(behavior)];
		return discountedTraits.contains(activeWaveTrait);
	}

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

	inline int32 GetSummonCost(const AllyBehavior behavior, const WaveTrait activeWaveTrait, const SummonDiscountTraitConfig& config)
	{
		const int32 baseCost = GetSummonCost(behavior);

		if (HasWaveTraitSummonDiscount(config, behavior, activeWaveTrait))
		{
			return Max(1, (baseCost - WaveTraitSummonDiscountAmount));
		}

		return baseCost;
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
