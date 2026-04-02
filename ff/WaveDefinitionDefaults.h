# pragma once
# include "WaveTypes.h"

namespace ff
{
	[[nodiscard]] inline WaveType GetDefaultWaveType(const int32 wave)
	{
		if (wave == FinalBossWave)
		{
			return WaveType::TrueBoss;
		}

		if ((wave > 0) && ((wave % MidBossWaveInterval) == 0))
		{
			return WaveType::MidBoss;
		}

		return WaveType::Standard;
	}

	[[nodiscard]] inline int32 GetDefaultWaveFocusIndex(const int32 wave)
	{
		return (((wave - 1) % 3 + 3) % 3);
	}

	[[nodiscard]] inline WaveDefinition MakeDefaultWaveDefinition(const int32 wave)
	{
		WaveDefinition definition;
		definition.waveNumber = Max(1, wave);
		definition.type = GetDefaultWaveType(definition.waveNumber);

		switch (definition.type)
		{
		case WaveType::MidBoss:
			definition.label = U"Mid Boss";
			definition.description = U"中ボス + 少数護衛";
			definition.accentColor = ColorF{ 0.96, 0.40, 0.22 };
			definition.spawnInterval = 0.82;
			definition.spawnQueue << EnemyKind::MidBoss;
			for (int32 index = 0; index < (MidBossBaseSupportEnemyCount + (definition.waveNumber / MidBossWaveInterval) - 1); ++index)
			{
				definition.spawnQueue << EnemyKind::Normal;
			}
			return definition;

		case WaveType::TrueBoss:
			definition.label = U"True Boss";
			definition.description = U"撃破でクリア";
			definition.accentColor = ColorF{ 0.76, 0.34, 0.96 };
			definition.spawnInterval = 1.15;
			definition.spawnQueue << EnemyKind::TrueBoss;
			return definition;

		case WaveType::Standard:
		default:
			break;
		}

		const int32 baseCount = (BaseEnemiesPerWave + ((definition.waveNumber - 1) * AdditionalEnemiesPerWave));
		const double baseInterval = Max(MinEnemySpawnInterval, (EnemySpawnInterval - ((definition.waveNumber - 1) * EnemySpawnIntervalStepPerWave)));

		switch (GetDefaultWaveFocusIndex(definition.waveNumber))
		{
		case 0:
			definition.label = U"Rush";
			definition.description = U"高速接近";
			definition.accentColor = ColorF{ 0.96, 0.44, 0.34 };
			definition.spawnInterval = Max(MinEnemySpawnInterval, (baseInterval * 0.68));
			for (int32 index = 0; index < Max(4, (baseCount - 1)); ++index)
			{
				definition.spawnQueue << EnemyKind::Normal;
			}
			break;

		case 1:
			definition.label = U"Swarm";
			definition.description = U"数で押す";
			definition.accentColor = ColorF{ 0.84, 0.70, 0.22 };
			definition.spawnInterval = Max(MinEnemySpawnInterval, (baseInterval * 0.82));
			for (int32 index = 0; index < (baseCount + 3); ++index)
			{
				definition.spawnQueue << EnemyKind::Normal;
			}
			break;

		default:
			definition.label = U"Pressure";
			definition.description = U"持続圧力";
			definition.accentColor = ColorF{ 0.58, 0.34, 0.92 };
			definition.spawnInterval = Max(MinEnemySpawnInterval, (baseInterval * 1.06));
			for (int32 index = 0; index < (baseCount + 1); ++index)
			{
				definition.spawnQueue << EnemyKind::Normal;
			}
			break;
		}

		switch ((((definition.waveNumber - 1) % 4) + 4) % 4)
		{
		case 0:
			definition.trait = WaveTrait::Reinforced;
			definition.enemyHpMultiplier = 1.35;
			break;

		case 1:
			definition.trait = WaveTrait::Assault;
			definition.enemySpeedMultiplier = 1.18;
			definition.enemyAttackIntervalMultiplier = 0.86;
			break;

		case 2:
			definition.trait = WaveTrait::Bounty;
			definition.rewardBonusPerKill = 1;
			break;

		default:
			break;
		}

		return definition;
	}

	[[nodiscard]] inline Array<WaveDefinition> MakeDefaultWaveDefinitions(const int32 waveCount = FinalBossWave)
	{
		Array<WaveDefinition> definitions;
		definitions.reserve(Max(1, waveCount));

		for (int32 wave = 1; wave <= Max(1, waveCount); ++wave)
		{
			definitions << MakeDefaultWaveDefinition(wave);
		}

		return definitions;
	}

	[[nodiscard]] inline WaveConfig MakeDefaultWaveConfig()
	{
		WaveConfig config;
		config.waves = MakeDefaultWaveDefinitions();
		return config;
	}

	inline void NormalizeWaveDefinition(WaveDefinition& definition, const WaveDefinition& fallback)
	{
		definition.waveNumber = fallback.waveNumber;

		if (definition.label.isEmpty())
		{
			definition.label = fallback.label;
		}

		if (definition.description.isEmpty())
		{
			definition.description = fallback.description;
		}

		if (definition.enemyHpMultiplier <= 0.0)
		{
			definition.enemyHpMultiplier = fallback.enemyHpMultiplier;
		}

		if (definition.enemySpeedMultiplier <= 0.0)
		{
			definition.enemySpeedMultiplier = fallback.enemySpeedMultiplier;
		}

		if (definition.enemyAttackIntervalMultiplier <= 0.0)
		{
			definition.enemyAttackIntervalMultiplier = fallback.enemyAttackIntervalMultiplier;
		}

		if (definition.rewardBonusPerKill < 0)
		{
			definition.rewardBonusPerKill = 0;
		}

		if (definition.spawnInterval <= 0.0)
		{
			definition.spawnInterval = fallback.spawnInterval;
		}

		if (definition.spawnQueue.isEmpty())
		{
			definition.spawnQueue = fallback.spawnQueue;
		}
	}
}
