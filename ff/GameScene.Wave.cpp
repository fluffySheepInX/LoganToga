# include "GameScene.h"
# include "UnitLogic.h"

namespace
{
	enum class WaveFocus
	{
		Rush,
		Swarm,
		Pressure,
	};

	WaveFocus GetWaveFocus(const int32 wave)
	{
		switch (((wave - 1) % 3 + 3) % 3)
		{
		case 0:
			return WaveFocus::Rush;
		case 1:
			return WaveFocus::Swarm;
		default:
			return WaveFocus::Pressure;
		}
	}

	StringView GetWaveFocusLabel(const WaveFocus focus)
	{
		switch (focus)
		{
		case WaveFocus::Rush:
			return U"Rush";
		case WaveFocus::Swarm:
			return U"Swarm";
		default:
			return U"Pressure";
		}
	}

	StringView GetWaveFocusDescription(const WaveFocus focus)
	{
		switch (focus)
		{
		case WaveFocus::Rush:
			return U"高速接近";
		case WaveFocus::Swarm:
			return U"数で押す";
		default:
			return U"持続圧力";
		}
	}

	ColorF GetWaveFocusColor(const WaveFocus focus)
	{
		switch (focus)
		{
		case WaveFocus::Rush:
			return ColorF{ 0.96, 0.44, 0.34 };
		case WaveFocus::Swarm:
			return ColorF{ 0.84, 0.70, 0.22 };
		default:
			return ColorF{ 0.58, 0.34, 0.92 };
		}
	}

	int32 GetWaveEnemyCount(const int32 wave)
	{
		const int32 baseCount = (ff::BaseEnemiesPerWave + ((wave - 1) * ff::AdditionalEnemiesPerWave));

		switch (GetWaveFocus(wave))
		{
		case WaveFocus::Rush:
			return Max(4, (baseCount - 1));
		case WaveFocus::Swarm:
			return (baseCount + 3);
		default:
			return (baseCount + 1);
		}
	}

	double GetWaveSpawnInterval(const int32 wave)
	{
		const double baseInterval = Max(ff::MinEnemySpawnInterval, (ff::EnemySpawnInterval - ((wave - 1) * ff::EnemySpawnIntervalStepPerWave)));

		switch (GetWaveFocus(wave))
		{
		case WaveFocus::Rush:
			return Max(ff::MinEnemySpawnInterval, (baseInterval * 0.68));
		case WaveFocus::Swarm:
			return Max(ff::MinEnemySpawnInterval, (baseInterval * 0.82));
		default:
			return Max(ff::MinEnemySpawnInterval, (baseInterval * 1.06));
		}
	}
}

void GameScene::UpdateWaveState()
{
	if (m_waveActive)
	{
		m_enemySpawnTimer += Scene::DeltaTime();
      const double waveSpawnInterval = GetWaveSpawnInterval(m_currentWave);

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
       m_enemiesToSpawnInWave = GetWaveEnemyCount(m_currentWave);
		m_enemySpawnTimer = 0.0;
		m_waveBannerTimer = ff::WaveBannerDuration;
	}
}

void GameScene::DrawWaveBanner() const
{
 if ((m_currentWave <= 0) && ((m_currentWave + 1) <= 0))
	{
		return;
	}

	if ((m_waveBannerTimer > 0.0) && (m_currentWave > 0))
	{
		const WaveFocus focus = GetWaveFocus(m_currentWave);
		const ColorF accent = GetWaveFocusColor(focus);
		const double alpha = Min(1.0, (m_waveBannerTimer / ff::WaveBannerDuration));
      const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 292, 68 };
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent.lerp(Palette::White, 0.35), (0.92 * alpha) };
		waveRect.rounded(14).draw(ColorF{ 0.10, 0.08, 0.18, (0.76 * alpha) });
        waveRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center().movedBy(0, -12), ColorF{ 1.0, 1.0, 1.0, alpha });
      m_font(U"{} / {}"_fmt(GetWaveFocusLabel(focus), GetWaveFocusDescription(focus))).drawAt(14, waveRect.center().movedBy(0, 14), accentText);
		return;
	}

	if ((not m_waveActive) && (m_nextWaveTimer <= 1.6))
	{
		const int32 nextWave = (m_currentWave + 1);
		const WaveFocus focus = GetWaveFocus(nextWave);
		const ColorF accent = GetWaveFocusColor(focus);
		const double alpha = Min(1.0, (1.6 - Max(0.0, m_nextWaveTimer)) / 1.6);
     const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent, (0.78 * alpha) };
		const RectF previewRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, 56 };
		previewRect.rounded(14).draw(ColorF{ 0.08, 0.10, 0.18, (0.52 * alpha) });
        previewRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Next Wave {}: {}"_fmt(nextWave, GetWaveFocusLabel(focus))).drawAt(18, previewRect.center().movedBy(0, -8), ColorF{ 1.0, 1.0, 1.0, alpha });
     m_font(GetWaveFocusDescription(focus)).drawAt(13, previewRect.center().movedBy(0, 12), accentText);
	}
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
