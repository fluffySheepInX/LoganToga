# include "GameScene.h"
# include "UnitLogic.h"

void GameScene::UpdateWaveState()
{
	if (m_waveActive)
	{
		m_enemySpawnTimer += Scene::DeltaTime();
		const double waveSpawnInterval = Max(ff::MinEnemySpawnInterval, (ff::EnemySpawnInterval - ((m_currentWave - 1) * ff::EnemySpawnIntervalStepPerWave)));

		while ((m_enemiesSpawnedInWave < m_enemiesToSpawnInWave) && (m_enemySpawnTimer >= waveSpawnInterval))
		{
			m_enemySpawnTimer -= waveSpawnInterval;

			if (ff::SpawnEnemy(m_enemies, m_enemySpawnTiles))
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
			m_waveActive = false;
			m_enemySpawnTimer = 0.0;
			m_nextWaveTimer = ff::WaveClearDelay;
		}

		return;
	}

	m_nextWaveTimer -= Scene::DeltaTime();

	if (m_nextWaveTimer <= 0.0)
	{
		m_waveActive = true;
		++m_currentWave;
		m_enemiesSpawnedInWave = 0;
		m_enemiesToSpawnInWave = (ff::BaseEnemiesPerWave + ((m_currentWave - 1) * ff::AdditionalEnemiesPerWave));
		m_enemySpawnTimer = 0.0;
		m_waveBannerTimer = ff::WaveBannerDuration;
	}
}

void GameScene::DrawWaveBanner() const
{
	if ((m_waveBannerTimer <= 0.0) || (m_currentWave <= 0))
	{
		return;
	}

	const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 240, 46 };
	const double alpha = Min(1.0, (m_waveBannerTimer / ff::WaveBannerDuration));
	waveRect.rounded(12).draw(ColorF{ 0.16, 0.10, 0.30, (0.72 * alpha) });
	waveRect.rounded(12).drawFrame(2, ColorF{ 0.92, 0.88, 1.0, (0.86 * alpha) });
	m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center(), ColorF{ 1.0, 1.0, 1.0, alpha });
}

void GameScene::DrawDefeatMessage() const
{
	if (m_playerHp > 0.0)
	{
		return;
	}

	const RectF messageRect{ Arg::center = Scene::Center(), 300, 52 };
	messageRect.rounded(12).draw(ColorF{ 0.24, 0.04, 0.04, 0.88 });
	messageRect.rounded(12).drawFrame(2, ColorF{ 0.92, 0.54, 0.48, 0.92 });
	m_font(U"主人公が撃破されました").drawAt(20, messageRect.center(), Palette::White);
}
