# include "GameScene.h"
# include "UnitLogic.h"

void GameScene::UpdateWaveState()
{
	if (m_stageCleared)
	{
		return;
	}

	if (m_waveActive)
	{
		const auto& waveDefinition = ff::GetWaveDefinition(m_currentWave);
		m_enemySpawnTimer += Scene::DeltaTime();
		const double waveSpawnInterval = waveDefinition.spawnInterval;

		while ((m_enemiesSpawnedInWave < m_enemiesToSpawnInWave) && (m_enemySpawnTimer >= waveSpawnInterval))
		{
			m_enemySpawnTimer -= waveSpawnInterval;

			if (ff::SpawnEnemy(
				m_enemies,
				m_enemySpawnTiles,
				m_pendingWaveEnemies[m_enemiesSpawnedInWave],
				m_currentWaveEnemyHpMultiplier,
				m_currentWaveEnemySpeedMultiplier,
				m_currentWaveEnemyAttackIntervalMultiplier))
			{
				++m_enemiesSpawnedInWave;
			}
			else
			{
				break;
			}
		}

		if ((m_enemiesSpawnedInWave >= m_enemiesToSpawnInWave) && m_enemies.isEmpty())
		{
			if ((waveDefinition.type == ff::WaveType::TrueBoss) || (not ff::HasWaveDefinition(m_currentWave + 1)))
			{
				m_stageCleared = true;
				m_pendingWaveEnemies.clear();
				return;
			}

			m_waveActive = false;
			m_enemySpawnTimer = 0.0;
			m_nextWaveTimer = ff::GetWaveClearDelay();
			m_pendingWaveEnemies.clear();
		}

		return;
	}

	m_nextWaveTimer -= Scene::DeltaTime();

	if ((m_nextWaveTimer <= 0.0) && ff::HasWaveDefinition(m_currentWave + 1))
	{
		m_waveActive = true;
		++m_currentWave;
		const auto& waveDefinition = ff::GetWaveDefinition(m_currentWave);
		m_pendingWaveEnemies = waveDefinition.spawnQueue;
		m_enemiesSpawnedInWave = 0;
		m_enemiesToSpawnInWave = static_cast<int32>(m_pendingWaveEnemies.size());
		m_enemySpawnTimer = 0.0;
		m_waveBannerTimer = ff::GetWaveBannerDuration();
		m_currentWaveTrait = waveDefinition.trait;
		m_currentWaveEnemyHpMultiplier = waveDefinition.enemyHpMultiplier;
		m_currentWaveEnemySpeedMultiplier = waveDefinition.enemySpeedMultiplier;
		m_currentWaveEnemyAttackIntervalMultiplier = waveDefinition.enemyAttackIntervalMultiplier;
		m_currentWaveRewardBonusPerKill = waveDefinition.rewardBonusPerKill;
	}
}
