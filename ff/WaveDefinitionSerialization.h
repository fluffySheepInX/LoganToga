# pragma once
# include "WaveDefinitionDefaults.h"

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
		return GetEnemyStableId(kind);
	}

	[[nodiscard]] inline Optional<EnemyKind> ParseEnemyKind(const StringView stableId)
	{
		return ParseEnemyStableId(stableId);
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
}
