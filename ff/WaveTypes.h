# pragma once
# include "EnemyTypes.h"

namespace ff
{
	enum class WaveTrait : uint8
	{
		None,
		Reinforced,
		Assault,
		Bounty,
	};

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

	enum class WaveType : uint8
	{
		Standard,
		MidBoss,
		TrueBoss,
	};

	struct WaveDefinition
	{
		int32 waveNumber = 1;
		WaveType type = WaveType::Standard;
		String label;
		String description;
		ColorF accentColor = Palette::White;
		WaveTrait trait = WaveTrait::None;
		double enemyHpMultiplier = 1.0;
		double enemySpeedMultiplier = 1.0;
		double enemyAttackIntervalMultiplier = 1.0;
		int32 rewardBonusPerKill = 0;
		double spawnInterval = EnemySpawnInterval;
		Array<EnemyKind> spawnQueue;
	};

	struct WaveConfig
	{
		double waveStartDelay = WaveStartDelay;
		double waveClearDelay = WaveClearDelay;
		double waveBannerDuration = WaveBannerDuration;
		Array<WaveDefinition> waves;
	};
}
