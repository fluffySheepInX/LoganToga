# include "GameScene.h"
# include "UnitLogic.h"

namespace
{
    enum class WaveType
	{
		Standard,
		MidBoss,
		TrueBoss,
	};

	enum class WaveFocus
	{
		Rush,
		Swarm,
		Pressure,
	};

	WaveType GetWaveType(const int32 wave)
	{
		if (wave == ff::FinalBossWave)
		{
			return WaveType::TrueBoss;
		}

		if ((wave > 0) && ((wave % ff::MidBossWaveInterval) == 0))
		{
			return WaveType::MidBoss;
		}

		return WaveType::Standard;
	}

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

   int32 GetStandardWaveEnemyCount(const int32 wave)
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

	Array<ff::EnemyKind> BuildWaveSpawnQueue(const int32 wave)
	{
		Array<ff::EnemyKind> result;

		switch (GetWaveType(wave))
		{
		case WaveType::MidBoss:
			result << ff::EnemyKind::MidBoss;
			for (int32 i = 0; i < (ff::MidBossBaseSupportEnemyCount + (wave / ff::MidBossWaveInterval) - 1); ++i)
			{
				result << ff::EnemyKind::Normal;
			}
			return result;

		case WaveType::TrueBoss:
			result << ff::EnemyKind::TrueBoss;
			return result;

		case WaveType::Standard:
		default:
			for (int32 i = 0; i < GetStandardWaveEnemyCount(wave); ++i)
			{
				result << ff::EnemyKind::Normal;
			}
			return result;
		}
	}

	StringView GetWaveLabel(const int32 wave)
	{
		switch (GetWaveType(wave))
		{
		case WaveType::MidBoss:
			return U"Mid Boss";

		case WaveType::TrueBoss:
			return U"True Boss";

		case WaveType::Standard:
		default:
			return GetWaveFocusLabel(GetWaveFocus(wave));
		}
	}

	StringView GetWaveDescription(const int32 wave)
	{
		switch (GetWaveType(wave))
		{
		case WaveType::MidBoss:
			return U"中ボス + 少数護衛";

		case WaveType::TrueBoss:
			return U"撃破でクリア";

		case WaveType::Standard:
		default:
			return GetWaveFocusDescription(GetWaveFocus(wave));
		}
	}

	ColorF GetWaveAccentColor(const int32 wave)
	{
		switch (GetWaveType(wave))
		{
		case WaveType::MidBoss:
			return ColorF{ 0.96, 0.40, 0.22 };

		case WaveType::TrueBoss:
			return ColorF{ 0.76, 0.34, 0.96 };

		case WaveType::Standard:
		default:
			return GetWaveFocusColor(GetWaveFocus(wave));
		}
	}

	double GetWaveSpawnInterval(const int32 wave)
	{
        switch (GetWaveType(wave))
		{
		case WaveType::MidBoss:
			return 0.82;

		case WaveType::TrueBoss:
			return 1.15;

		case WaveType::Standard:
		default:
			break;
		}

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
   if (m_stageCleared)
	{
		return;
	}

	if (m_waveActive)
	{
		m_enemySpawnTimer += Scene::DeltaTime();
     const double waveSpawnInterval = GetWaveSpawnInterval(m_currentWave);

		while ((m_enemiesSpawnedInWave < m_enemiesToSpawnInWave) && (m_enemySpawnTimer >= waveSpawnInterval))
		{
			m_enemySpawnTimer -= waveSpawnInterval;

           if (ff::SpawnEnemy(m_enemies, m_enemySpawnTiles, m_pendingWaveEnemies[m_enemiesSpawnedInWave]))
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
           if (GetWaveType(m_currentWave) == WaveType::TrueBoss)
			{
				m_stageCleared = true;
				m_pendingWaveEnemies.clear();
				return;
			}

			m_waveActive = false;
			m_enemySpawnTimer = 0.0;
			m_nextWaveTimer = ff::WaveClearDelay;
           m_pendingWaveEnemies.clear();
		}

		return;
	}

	m_nextWaveTimer -= Scene::DeltaTime();

	if (m_nextWaveTimer <= 0.0)
	{
		m_waveActive = true;
		++m_currentWave;
     m_pendingWaveEnemies = BuildWaveSpawnQueue(m_currentWave);
		m_enemiesSpawnedInWave = 0;
       m_enemiesToSpawnInWave = static_cast<int32>(m_pendingWaveEnemies.size());
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
        const ColorF accent = GetWaveAccentColor(m_currentWave);
		const double alpha = Min(1.0, (m_waveBannerTimer / ff::WaveBannerDuration));
      const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, 68 };
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent.lerp(Palette::White, 0.35), (0.92 * alpha) };
		waveRect.rounded(14).draw(ColorF{ 0.10, 0.08, 0.18, (0.76 * alpha) });
     waveRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center().movedBy(0, -12), ColorF{ 1.0, 1.0, 1.0, alpha });
        m_font(U"{} / {}"_fmt(GetWaveLabel(m_currentWave), GetWaveDescription(m_currentWave))).drawAt(14, waveRect.center().movedBy(0, 14), accentText);
		return;
	}

	if ((not m_waveActive) && (m_nextWaveTimer <= 1.6))
	{
		const int32 nextWave = (m_currentWave + 1);
     const ColorF accent = GetWaveAccentColor(nextWave);
		const double alpha = Min(1.0, (1.6 - Max(0.0, m_nextWaveTimer)) / 1.6);
      const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent, (0.78 * alpha) };
		const RectF previewRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, 56 };
		previewRect.rounded(14).draw(ColorF{ 0.08, 0.10, 0.18, (0.52 * alpha) });
      previewRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Next Wave {}: {}"_fmt(nextWave, GetWaveLabel(nextWave))).drawAt(18, previewRect.center().movedBy(0, -8), ColorF{ 1.0, 1.0, 1.0, alpha });
		m_font(GetWaveDescription(nextWave)).drawAt(13, previewRect.center().movedBy(0, 12), accentText);
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

void GameScene::UpdateStageClearState()
{
	if (not m_stageCleared)
	{
		return;
	}

	if (KeyEnter.down() || MouseL.down())
	{
		changeScene(U"Title");
		return;
	}

	m_stageClearTransitionTimer = Max(0.0, (m_stageClearTransitionTimer - Scene::DeltaTime()));

	if (m_stageClearTransitionTimer <= 0.0)
	{
		changeScene(U"Title");
	}
}

void GameScene::DrawVictoryMessage() const
{
	if (not m_stageCleared)
	{
		return;
	}

	const RectF messageRect{ Arg::center = Scene::Center(), 360, 64 };
	messageRect.rounded(14).draw(ColorF{ 0.18, 0.08, 0.26, 0.90 });
	messageRect.rounded(14).drawFrame(2, ColorF{ 0.86, 0.52, 1.0, 0.94 });
	m_font(U"真ボスを撃破しました").drawAt(22, messageRect.center().movedBy(0, -10), Palette::White);
 m_font(U"{:.1f}s後にタイトルへ / Enter・クリックで戻る"_fmt(m_stageClearTransitionTimer)).drawAt(14, messageRect.center().movedBy(0, 14), ColorF{ 0.92, 0.88, 1.0, 0.94 });
}
