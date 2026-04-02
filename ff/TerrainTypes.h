# pragma once
# include "BalanceConstants.h"

namespace ff
{
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
		Haste,
		Rush,
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

		case SpecialTileKind::Haste:
		case SpecialTileKind::Rush:
		case SpecialTileKind::None:
		default:
			return ResourcePerEnemyKill;
		}
	}

	inline constexpr double GetAllyAttackIntervalMultiplier(const SpecialTileKind tileKind)
	{
		switch (tileKind)
		{
		case SpecialTileKind::Haste:
			return HasteTileAllyAttackIntervalMultiplier;

		case SpecialTileKind::Bonus:
		case SpecialTileKind::Penalty:
		case SpecialTileKind::Rush:
		case SpecialTileKind::None:
		default:
			return 1.0;
		}
	}

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
