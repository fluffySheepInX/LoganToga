# include "DefinitionRepository.h"

namespace
{
	ff::DefinitionRepository* g_definitionRepository = nullptr;

	void NormalizeWaveConfig(ff::WaveConfig& config)
	{
		config.waveStartDelay = Max(0.05, config.waveStartDelay);
		config.waveClearDelay = Max(0.05, config.waveClearDelay);
		config.waveBannerDuration = Max(0.05, config.waveBannerDuration);

		if (config.waves.isEmpty())
		{
			config.waves << ff::MakeDefaultWaveDefinition(1);
		}

		for (size_t index = 0; index < config.waves.size(); ++index)
		{
			config.waves[index].waveNumber = static_cast<int32>(index + 1);
			ff::NormalizeWaveDefinition(config.waves[index], ff::MakeDefaultWaveDefinition(static_cast<int32>(index + 1)));
		}
	}
}

namespace ff
{
	DefinitionRepository::DefinitionRepository()
		: m_unitDefinitions{ LoadUnitDefinitions() }
		, m_enemyDefinitions{ LoadEnemyDefinitions() }
		, m_waveConfig{ LoadWaveConfig() }
	{
	}

	const std::array<UnitDefinition, AllyBehaviorCount>& DefinitionRepository::GetUnitDefinitions() const
	{
		return m_unitDefinitions;
	}

	void DefinitionRepository::SetUnitDefinition(UnitDefinition definition)
	{
		NormalizeUnitDefinition(definition, GetDefaultUnitDefinition(definition.id));
		m_unitDefinitions[ToIndex(definition.id)] = std::move(definition);
	}

	void DefinitionRepository::ReloadUnitDefinitions()
	{
		m_unitDefinitions = LoadUnitDefinitions();
	}

	const std::array<EnemyDefinition, EnemyKindCount>& DefinitionRepository::GetEnemyDefinitions() const
	{
		return m_enemyDefinitions;
	}

	void DefinitionRepository::SetEnemyDefinition(EnemyDefinition definition)
	{
		NormalizeEnemyDefinition(definition, GetDefaultEnemyDefinition(definition.kind));
		m_enemyDefinitions[ToIndex(definition.kind)] = std::move(definition);
	}

	void DefinitionRepository::ReloadEnemyDefinitions()
	{
		m_enemyDefinitions = LoadEnemyDefinitions();
	}

	const WaveConfig& DefinitionRepository::GetWaveConfig() const
	{
		return m_waveConfig;
	}

	void DefinitionRepository::SetWaveConfig(WaveConfig config)
	{
		NormalizeWaveConfig(config);
		m_waveConfig = std::move(config);
	}

	void DefinitionRepository::ReloadWaveDefinitions()
	{
		m_waveConfig = LoadWaveConfig();
	}

	void RegisterDefinitionRepository(DefinitionRepository& repository)
	{
		g_definitionRepository = &repository;
	}

	DefinitionRepository& GetDefinitionRepository()
	{
		assert(g_definitionRepository);
		return *g_definitionRepository;
	}

	void UnregisterDefinitionRepository(DefinitionRepository& repository)
	{
		if (g_definitionRepository == &repository)
		{
			g_definitionRepository = nullptr;
		}
	}

	const std::array<UnitDefinition, AllyBehaviorCount>& GetUnitDefinitions()
	{
		return GetDefinitionRepository().GetUnitDefinitions();
	}

	const UnitDefinition& GetUnitDefinition(const UnitId unitId)
	{
		return GetUnitDefinitions()[ToIndex(unitId)];
	}

	void ReloadUnitDefinitionsFromDisk()
	{
		GetDefinitionRepository().ReloadUnitDefinitions();
	}

	void SetUnitDefinition(UnitDefinition definition)
	{
		GetDefinitionRepository().SetUnitDefinition(std::move(definition));
	}

	bool SaveCurrentUnitDefinitionsToDisk()
	{
		return SaveUnitDefinitionsToDisk(GetUnitDefinitions());
	}

	const std::array<EnemyDefinition, EnemyKindCount>& GetEnemyDefinitions()
	{
		return GetDefinitionRepository().GetEnemyDefinitions();
	}

	const EnemyDefinition& GetEnemyDefinition(const EnemyKind kind)
	{
		return GetEnemyDefinitions()[ToIndex(kind)];
	}

	void ReloadEnemyDefinitionsFromDisk()
	{
		GetDefinitionRepository().ReloadEnemyDefinitions();
	}

	void SetEnemyDefinition(const EnemyDefinition& definition)
	{
		GetDefinitionRepository().SetEnemyDefinition(definition);
	}

	bool SaveCurrentEnemyDefinitionsToDisk()
	{
		return SaveEnemyDefinitionsToDisk(GetEnemyDefinitions());
	}

	const WaveConfig& GetWaveConfig()
	{
		return GetDefinitionRepository().GetWaveConfig();
	}

	void SetWaveConfig(WaveConfig config)
	{
		GetDefinitionRepository().SetWaveConfig(std::move(config));
	}

	int32 GetWaveCount()
	{
		return static_cast<int32>(GetWaveConfig().waves.size());
	}

	bool HasWaveDefinition(const int32 wave)
	{
		return InRange(wave, 1, GetWaveCount());
	}

	const WaveDefinition& GetWaveDefinition(const int32 wave)
	{
		return GetWaveConfig().waves[Clamp(wave, 1, GetWaveCount()) - 1];
	}

	double GetWaveStartDelay()
	{
		return GetWaveConfig().waveStartDelay;
	}

	double GetWaveClearDelay()
	{
		return GetWaveConfig().waveClearDelay;
	}

	double GetWaveBannerDuration()
	{
		return GetWaveConfig().waveBannerDuration;
	}

	void ReloadWaveDefinitionsFromDisk()
	{
		GetDefinitionRepository().ReloadWaveDefinitions();
	}

	bool SaveCurrentWaveDefinitionsToDisk()
	{
		return SaveWaveDefinitionsToDisk(GetWaveConfig());
	}
}
