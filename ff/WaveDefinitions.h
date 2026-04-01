# pragma once
# include "GameConstants.h"

namespace ff
{
	template <class TomlLike>
	[[nodiscard]] inline Array<String> ReadTomlBasicStringArray(const TomlLike& toml, const String& key)
	{
		Array<String> values;

		try
		{
			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<String>();
			}
		}
		catch (const std::exception&)
		{
			values.clear();
		}

		return values;
	}

	[[nodiscard]] inline String BuildTomlBasicStringArray(const Array<String>& values)
	{
		String result = U"[";

		for (size_t index = 0; index < values.size(); ++index)
		{
			if (index > 0)
			{
				result += U", ";
			}

			result += U"\"{}\""_fmt(EscapeTomlBasicString(values[index]));
		}

		result += U"]";
		return result;
	}

	[[nodiscard]] inline StringView GetWaveTypeStableId(const WaveType type)
	{
		switch (type)
		{
		case WaveType::MidBoss:
			return U"mid_boss";

		case WaveType::TrueBoss:
			return U"true_boss";

		case WaveType::Standard:
		default:
			return U"standard";
		}
	}

	[[nodiscard]] inline Optional<WaveType> ParseWaveType(const StringView stableId)
	{
		if (stableId == U"standard")
		{
			return WaveType::Standard;
		}

		if (stableId == U"mid_boss")
		{
			return WaveType::MidBoss;
		}

		if (stableId == U"true_boss")
		{
			return WaveType::TrueBoss;
		}

		return none;
	}

	[[nodiscard]] inline StringView GetWaveTraitStableId(const WaveTrait trait)
	{
		switch (trait)
		{
		case WaveTrait::Reinforced:
			return U"reinforced";

		case WaveTrait::Assault:
			return U"assault";

		case WaveTrait::Bounty:
			return U"bounty";

		case WaveTrait::None:
		default:
			return U"none";
		}
	}

	[[nodiscard]] inline Optional<WaveTrait> ParseWaveTraitStableId(const StringView stableId)
	{
		if (stableId == U"none")
		{
			return WaveTrait::None;
		}

		if (stableId == U"reinforced")
		{
			return WaveTrait::Reinforced;
		}

		if (stableId == U"assault")
		{
			return WaveTrait::Assault;
		}

		if (stableId == U"bounty")
		{
			return WaveTrait::Bounty;
		}

		return none;
	}

	[[nodiscard]] inline StringView GetEnemyKindStableId(const EnemyKind kind)
	{
		switch (kind)
		{
		case EnemyKind::MidBoss:
			return U"mid_boss";

		case EnemyKind::TrueBoss:
			return U"true_boss";

		case EnemyKind::Normal:
		default:
			return U"normal";
		}
	}

	[[nodiscard]] inline Optional<EnemyKind> ParseEnemyKind(const StringView stableId)
	{
		if (stableId == U"normal")
		{
			return EnemyKind::Normal;
		}

		if (stableId == U"mid_boss")
		{
			return EnemyKind::MidBoss;
		}

		if (stableId == U"true_boss")
		{
			return EnemyKind::TrueBoss;
		}

		return none;
	}

	[[nodiscard]] inline String GetBundledWaveDefinitionsPath()
	{
		const String runtimeRelativePath = U"waveDefinitions.toml";
		if (FileSystem::Exists(runtimeRelativePath))
		{
			return runtimeRelativePath;
		}

		const String projectRelativePath = U"App/waveDefinitions.toml";
		if (FileSystem::Exists(projectRelativePath))
		{
			return projectRelativePath;
		}

		return runtimeRelativePath;
	}

	[[nodiscard]] inline String GetUserWaveDefinitionsPath()
	{
		return U"save/waveDefinitions.toml";
	}

	[[nodiscard]] inline String GetWaveDefinitionsPath()
	{
		const String userPath = GetUserWaveDefinitionsPath();
		if (FileSystem::Exists(userPath))
		{
			return userPath;
		}

		return GetBundledWaveDefinitionsPath();
	}

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

	[[nodiscard]] inline String BuildWaveDefinitionsToml(const WaveConfig& sourceConfig)
	{
		WaveConfig config = sourceConfig;
		if (config.waves.isEmpty())
		{
			config.waves << MakeDefaultWaveDefinition(1);
		}

		String content;
		content += U"schemaVersion = {}\n"_fmt(WaveDefinitionSchemaVersion);
		content += U"wave_start_delay = {:.3f}\n"_fmt(Max(0.05, config.waveStartDelay));
		content += U"wave_clear_delay = {:.3f}\n"_fmt(Max(0.05, config.waveClearDelay));
		content += U"wave_banner_duration = {:.3f}\n"_fmt(Max(0.05, config.waveBannerDuration));
		content += U"waveCount = {}\n"_fmt(config.waves.size());

		for (size_t index = 0; index < config.waves.size(); ++index)
		{
			WaveDefinition definition = config.waves[index];
			definition.waveNumber = static_cast<int32>(index + 1);
			NormalizeWaveDefinition(definition, MakeDefaultWaveDefinition(definition.waveNumber));

			Array<String> spawnQueue;
			spawnQueue.reserve(definition.spawnQueue.size());
			for (const auto kind : definition.spawnQueue)
			{
				spawnQueue << String{ GetEnemyKindStableId(kind) };
			}

			content += U"\n[[waves]]\n";
			content += U"wave = {}\n"_fmt(definition.waveNumber);
			content += U"type = \"{}\"\n"_fmt(EscapeTomlBasicString(String{ GetWaveTypeStableId(definition.type) }));
			content += U"label = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.label));
			content += U"description = \"{}\"\n"_fmt(EscapeTomlBasicString(definition.description));
			content += U"accent_color = {}\n"_fmt(BuildTomlColorArray(definition.accentColor));
			content += U"trait = \"{}\"\n"_fmt(EscapeTomlBasicString(String{ GetWaveTraitStableId(definition.trait) }));
			content += U"enemy_hp_multiplier = {:.3f}\n"_fmt(definition.enemyHpMultiplier);
			content += U"enemy_speed_multiplier = {:.3f}\n"_fmt(definition.enemySpeedMultiplier);
			content += U"enemy_attack_interval_multiplier = {:.3f}\n"_fmt(definition.enemyAttackIntervalMultiplier);
			content += U"reward_bonus_per_kill = {}\n"_fmt(definition.rewardBonusPerKill);
			content += U"spawn_interval = {:.3f}\n"_fmt(definition.spawnInterval);
			content += U"spawn_queue = {}\n"_fmt(BuildTomlBasicStringArray(spawnQueue));
		}

		return content;
	}

	[[nodiscard]] inline bool SaveWaveDefinitionsToDisk(const WaveConfig& config)
	{
		const String savePath = GetUserWaveDefinitionsPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(savePath));

		TextWriter writer{ savePath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildWaveDefinitionsToml(config));
		return true;
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
