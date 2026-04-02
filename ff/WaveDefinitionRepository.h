# pragma once
# include "WaveDefinitionSerialization.h"

namespace ff
{
	[[nodiscard]] inline WaveConfig LoadWaveConfig()
	{
		WaveConfig config = MakeDefaultWaveConfig();
		const TOMLReader toml{ GetWaveDefinitionsPath() };
		if (!toml)
		{
			return config;
		}

		const int32 schemaVersion = ReadTomlInt(toml, U"schemaVersion", 0);
		if ((schemaVersion <= 0) || (schemaVersion > WaveDefinitionSchemaVersion))
		{
			return config;
		}

		config.waveStartDelay = Max(0.05, ReadTomlDouble(toml, U"wave_start_delay", config.waveStartDelay));
		config.waveClearDelay = Max(0.05, ReadTomlDouble(toml, U"wave_clear_delay", config.waveClearDelay));
		config.waveBannerDuration = Max(0.05, ReadTomlDouble(toml, U"wave_banner_duration", config.waveBannerDuration));

		const int32 waveCount = Max(1, ReadTomlInt(toml, U"waveCount", static_cast<int32>(config.waves.size())));
		config.waves = MakeDefaultWaveDefinitions(waveCount);

		if (!toml[U"waves"].isTableArray())
		{
			return config;
		}

		for (const auto& table : toml[U"waves"].tableArrayView())
		{
			const int32 waveNumber = ReadTomlInt(table, U"wave", 0);
			if ((waveNumber <= 0) || (waveNumber > static_cast<int32>(config.waves.size())))
			{
				continue;
			}

			const WaveDefinition fallback = MakeDefaultWaveDefinition(waveNumber);
			WaveDefinition definition = fallback;

			if (const auto type = ParseWaveType(ReadTomlString(table, U"type", String{ GetWaveTypeStableId(definition.type) })))
			{
				definition.type = *type;
			}

			definition.label = ReadTomlString(table, U"label", definition.label);
			definition.description = ReadTomlString(table, U"description", definition.description);
			definition.accentColor = ReadTomlColor(table, U"accent_color", definition.accentColor);

			if (const auto trait = ParseWaveTraitStableId(ReadTomlString(table, U"trait", String{ GetWaveTraitStableId(definition.trait) })))
			{
				definition.trait = *trait;
			}

			definition.enemyHpMultiplier = ReadTomlDouble(table, U"enemy_hp_multiplier", definition.enemyHpMultiplier);
			definition.enemySpeedMultiplier = ReadTomlDouble(table, U"enemy_speed_multiplier", definition.enemySpeedMultiplier);
			definition.enemyAttackIntervalMultiplier = ReadTomlDouble(table, U"enemy_attack_interval_multiplier", definition.enemyAttackIntervalMultiplier);
			definition.rewardBonusPerKill = ReadTomlInt(table, U"reward_bonus_per_kill", definition.rewardBonusPerKill);
			definition.spawnInterval = ReadTomlDouble(table, U"spawn_interval", definition.spawnInterval);

			Array<EnemyKind> spawnQueue;
			for (const auto& value : ReadTomlBasicStringArray(table, U"spawn_queue"))
			{
				if (const auto enemyKind = ParseEnemyKind(value))
				{
					spawnQueue << *enemyKind;
				}
			}

			if (!spawnQueue.isEmpty())
			{
				definition.spawnQueue = std::move(spawnQueue);
			}

			NormalizeWaveDefinition(definition, fallback);
			config.waves[waveNumber - 1] = std::move(definition);
		}

		return config;
	}

	[[nodiscard]] inline WaveConfig& GetMutableWaveConfig()
	{
		static WaveConfig Config = LoadWaveConfig();
		return Config;
	}

	[[nodiscard]] inline const WaveConfig& GetWaveConfig()
	{
		return GetMutableWaveConfig();
	}

	inline void SetWaveConfig(WaveConfig config)
	{
		config.waveStartDelay = Max(0.05, config.waveStartDelay);
		config.waveClearDelay = Max(0.05, config.waveClearDelay);
		config.waveBannerDuration = Max(0.05, config.waveBannerDuration);

		if (config.waves.isEmpty())
		{
			config.waves << MakeDefaultWaveDefinition(1);
		}

		for (size_t index = 0; index < config.waves.size(); ++index)
		{
			config.waves[index].waveNumber = static_cast<int32>(index + 1);
			NormalizeWaveDefinition(config.waves[index], MakeDefaultWaveDefinition(static_cast<int32>(index + 1)));
		}

		GetMutableWaveConfig() = std::move(config);
	}

	[[nodiscard]] inline int32 GetWaveCount()
	{
		return static_cast<int32>(GetWaveConfig().waves.size());
	}

	[[nodiscard]] inline bool HasWaveDefinition(const int32 wave)
	{
		return InRange(wave, 1, GetWaveCount());
	}

	[[nodiscard]] inline const WaveDefinition& GetWaveDefinition(const int32 wave)
	{
		return GetWaveConfig().waves[Clamp(wave, 1, GetWaveCount()) - 1];
	}

	[[nodiscard]] inline double GetWaveStartDelay()
	{
		return GetWaveConfig().waveStartDelay;
	}

	[[nodiscard]] inline double GetWaveClearDelay()
	{
		return GetWaveConfig().waveClearDelay;
	}

	[[nodiscard]] inline double GetWaveBannerDuration()
	{
		return GetWaveConfig().waveBannerDuration;
	}

	inline void ReloadWaveDefinitionsFromDisk()
	{
		GetMutableWaveConfig() = LoadWaveConfig();
	}

	[[nodiscard]] inline bool SaveCurrentWaveDefinitionsToDisk()
	{
		return SaveWaveDefinitionsToDisk(GetWaveConfig());
	}
}
