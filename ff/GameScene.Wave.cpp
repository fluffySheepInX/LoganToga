# include "GameScene.h"
# include "UnitLogic.h"

namespace
{
	String GetWaveTraitBannerText(const int32 wave)
	{
       if (!ff::HasWaveDefinition(wave))
		{
			return U"";
		}

		const auto& definition = ff::GetWaveDefinition(wave);

      if (definition.trait == ff::WaveTrait::None)
		{
			return U"";
		}

      return U"特性: {} / {}"_fmt(String{ ff::GetWaveTraitLabel(definition.trait) }, String{ ff::GetWaveTraitDescription(definition.trait) });
	}

	String FormatTimeRangeLabel(const double startTime, const double duration)
	{
		return U"{:.0f}s - {:.0f}s"_fmt(startTime, (startTime + duration));
	}

	String JoinLabels(const Array<String>& labels, const StringView separator, const size_t maxCount)
	{
		String result;
		const size_t count = Min(labels.size(), maxCount);

		for (size_t index = 0; index < count; ++index)
		{
			if (index > 0)
			{
				result += separator;
			}

			result += labels[index];
		}

		if (labels.size() > count)
		{
			if (!result.isEmpty())
			{
				result += separator;
			}

			result += U"ほか{}種"_fmt(labels.size() - count);
		}

		return result;
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

void GameScene::DrawWaveBanner() const
{
 if ((m_currentWave <= 0) && ((m_currentWave + 1) <= 0))
	{
		return;
	}

	if ((m_waveBannerTimer > 0.0) && (m_currentWave > 0))
	{
        const auto& waveDefinition = ff::GetWaveDefinition(m_currentWave);
		const ColorF accent = waveDefinition.accentColor;
		const String traitLine = GetWaveTraitBannerText(m_currentWave);
		const double alpha = Min(1.0, (m_waveBannerTimer / ff::GetWaveBannerDuration()));
		const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, traitLine.isEmpty() ? 68 : 90 };
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent.lerp(Palette::White, 0.35), (0.92 * alpha) };
		waveRect.rounded(14).draw(ColorF{ 0.10, 0.08, 0.18, (0.76 * alpha) });
        waveRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center().movedBy(0, traitLine.isEmpty() ? -12 : -22), ColorF{ 1.0, 1.0, 1.0, alpha });
		m_font(U"{} / {}"_fmt(waveDefinition.label, waveDefinition.description)).drawAt(14, waveRect.center().movedBy(0, traitLine.isEmpty() ? 14 : 4), accentText);
		if (not traitLine.isEmpty())
		{
			m_font(traitLine).drawAt(12, waveRect.center().movedBy(0, 28), ColorF{ 0.92, 0.96, 1.0, alpha });
		}
		return;
	}

 if ((not m_waveActive) && (m_nextWaveTimer <= ff::GetWaveBannerDuration()))
	{
		const int32 nextWave = (m_currentWave + 1);
        if (!ff::HasWaveDefinition(nextWave))
		{
			return;
		}

		const auto& waveDefinition = ff::GetWaveDefinition(nextWave);
		const double previewDuration = ff::GetWaveBannerDuration();
		const ColorF accent = waveDefinition.accentColor;
		const String traitLine = GetWaveTraitBannerText(nextWave);
		const double alpha = Min(1.0, (previewDuration - Max(0.0, m_nextWaveTimer)) / previewDuration);
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent, (0.78 * alpha) };
     const RectF previewRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, traitLine.isEmpty() ? 56 : 78 };
		previewRect.rounded(14).draw(ColorF{ 0.08, 0.10, 0.18, (0.52 * alpha) });
        previewRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Next Wave {}: {}"_fmt(nextWave, waveDefinition.label)).drawAt(18, previewRect.center().movedBy(0, traitLine.isEmpty() ? -8 : -16), ColorF{ 1.0, 1.0, 1.0, alpha });
		m_font(waveDefinition.description).drawAt(13, previewRect.center().movedBy(0, traitLine.isEmpty() ? 12 : 6), accentText);
		if (not traitLine.isEmpty())
		{
			m_font(traitLine).drawAt(12, previewRect.center().movedBy(0, 26), ColorF{ 0.92, 0.96, 1.0, alpha });
		}
	}
}

void GameScene::DrawDefeatMessage() const
{
	if (m_playerHp > 0.0)
	{
		return;
	}

	constexpr double BusyWindowDuration = 10.0;
	constexpr int32 UnitsPerPage = 6;
	const auto& availableUnitIds = ff::GetAvailableUnitIds();
	const int32 pageCount = Max(1, GetDefeatReportPageCount());
	const int32 currentPage = Clamp(m_defeatReportPage, 0, (pageCount - 1));
	const int32 totalSummons = [&]()
		{
			int32 count = 0;
			for (const auto unitId : availableUnitIds)
			{
				count += m_unitBattleAnalytics[ff::ToIndex(unitId)].summonCount;
			}

			return count;
		}();
	const double totalDamage = [&]()
		{
			double value = 0.0;
			for (const auto unitId : availableUnitIds)
			{
				value += m_unitBattleAnalytics[ff::ToIndex(unitId)].totalDamage;
			}

			return value;
		}();
	const int32 totalCost = [&]()
		{
			int32 value = 0;
			for (const auto unitId : availableUnitIds)
			{
				value += m_unitBattleAnalytics[ff::ToIndex(unitId)].totalCost;
			}

			return value;
		}();
	const double averageResources = (m_elapsedBattleTime > 0.0)
		? (m_resourceIntegral / m_elapsedBattleTime)
		: static_cast<double>(m_resourceCount);
	const BusyWindowAnalytics* busiestWindow = nullptr;

	for (const auto& busyWindow : m_busyWindowAnalytics)
	{
		if ((not busiestWindow) || (busyWindow.activityScore > busiestWindow->activityScore))
		{
			busiestWindow = &busyWindow;
		}
	}

	Optional<ff::UnitId> bestEfficiencyUnit;
	Optional<ff::UnitId> bestLifetimeUnit;
	double bestEfficiencyValue = -1.0;
	double bestLifetimeValue = -1.0;
	Array<String> unusedUnits;

	for (const auto unitId : availableUnitIds)
	{
		const auto& analytics = m_unitBattleAnalytics[ff::ToIndex(unitId)];
		if (analytics.summonCount <= 0)
		{
			unusedUnits << ff::GetUnitDefinition(unitId).label;
			continue;
		}

		const double efficiency = (analytics.totalCost > 0)
			? (analytics.totalDamage / analytics.totalCost)
			: 0.0;
		const double averageLifetime = (analytics.summonCount > 0)
			? (analytics.totalLifetime / analytics.summonCount)
			: 0.0;

		if (efficiency > bestEfficiencyValue)
		{
			bestEfficiencyValue = efficiency;
			bestEfficiencyUnit = unitId;
		}

		if (averageLifetime > bestLifetimeValue)
		{
			bestLifetimeValue = averageLifetime;
			bestLifetimeUnit = unitId;
		}
	}

	struct DefeatCauseCard
	{
		String title;
		String detail;
		String advice;
		double score = 0.0;
	};

	const int32 closeEnemyCount = static_cast<int32>(std::count_if(m_enemies.begin(), m_enemies.end(), [&](const ff::Enemy& enemy)
		{
			return (enemy.pos.distanceFrom(m_playerPos) <= 2.2);
		}));
	const int32 nearbyAllyCount = static_cast<int32>(std::count_if(m_allies.begin(), m_allies.end(), [&](const ff::Ally& ally)
		{
			return (ally.pos.distanceFrom(m_playerPos) <= 2.4);
		}));
	const int32 midBossCount = static_cast<int32>(std::count_if(m_enemies.begin(), m_enemies.end(), [](const ff::Enemy& enemy)
		{
			return (enemy.kind == ff::EnemyKind::MidBoss);
		}));
	const bool trueBossAlive = std::any_of(m_enemies.begin(), m_enemies.end(), [](const ff::Enemy& enemy)
		{
			return (enemy.kind == ff::EnemyKind::TrueBoss);
		});
	Array<DefeatCauseCard> causes = {
		{
			U"前線崩壊",
			U"主人公の近くに敵{}体、近接護衛{}体でした"_fmt(closeEnemyCount, nearbyAllyCount),
			U"前に出る兵を1枠増やすか、護衛を早めに出すと安定しやすいです",
			(closeEnemyCount * 2.1) + Max(0, (closeEnemyCount - nearbyAllyCount)) + (m_allies.isEmpty() ? 2.0 : 0.0)
		},
		{
			U"資源不足",
			U"資源不足の召喚失敗{}回 / 平均資源{:.1f}"_fmt(m_totalDeniedSummons, averageResources),
			U"低コスト兵を混ぜるか、資源を使い切る前の前倒し配置を試すと良さそうです",
			(m_totalDeniedSummons * 1.8) + m_lowResourceTime + Max(0.0, (3.0 - averageResources))
		},
		{
			U"ボス処理遅延",
			trueBossAlive ? U"真ボスが残ったまま押し切られました" : U"中ボス{}体が残っていました"_fmt(midBossCount),
			U"高コスト火力を温存しすぎず、ボス波までに仕事役を揃えると押し返しやすいです",
			(trueBossAlive ? 8.0 : 0.0) + (midBossCount * 3.2)
		},
		{
			U"操作過多",
			busiestWindow
				? U"{} に操作密度が上がり、召喚{}回 / 被弾{}回でした"_fmt(FormatTimeRangeLabel(busiestWindow->startTime, BusyWindowDuration), busiestWindow->summonAttempts, busiestWindow->playerHits)
				: U"操作ピークは検出できませんでした",
			U"忙しい10秒の前に配置を済ませるか、連打しやすい編成を減らすと楽になります",
			(busiestWindow ? (busiestWindow->activityScore * 1.4) : 0.0) + (busiestWindow ? (busiestWindow->playerHits * 1.6) : 0.0)
		},
		{
			U"戦力未投入",
			U"所持資源{} / 高資源帯{:.1f}s / 未使用{}種"_fmt(m_resourceCount, m_highResourceTime, unusedUnits.size()),
			U"抱えた資源を前線に変えるだけで、次はかなり粘れる可能性があります",
			(m_highResourceTime * 0.9) + Max(0, (m_resourceCount - 8)) + (unusedUnits.isEmpty() ? 0.0 : 0.6)
		},
		{
			U"継戦力不足",
			U"総出撃{}回 / 生存中{}体 / 主人公HPが尽きました"_fmt(totalSummons, m_allies.size()),
			U"平均生存時間の短い兵科を見直すと、同じ資源でも粘りやすくなります",
			2.0 + Max(0.0, (static_cast<double>(totalSummons) - static_cast<double>(m_allies.size())) * 0.35)
		}
	};
	std::sort(causes.begin(), causes.end(), [](const DefeatCauseCard& a, const DefeatCauseCard& b)
		{
			return (a.score > b.score);
		});

	const RectF overlay{ Scene::Rect() };
	const RectF panel{ Arg::center = Scene::Center(), 920, 560 };
	overlay.draw(ColorF{ 0.02, 0.02, 0.04, 0.76 });
	panel.rounded(22).draw(ColorF{ 0.10, 0.06, 0.10, 0.96 });
	panel.rounded(22).drawFrame(2, ColorF{ 0.92, 0.56, 0.52, 0.92 });
	m_font(U"主人公が撃破されました").drawAt(26, panel.center().movedBy(0, -252), Palette::White);
	m_font(U"Page {}/{}  /  Tab・←→で切替  /  Enter・クリックでタイトルへ"_fmt(currentPage + 1, pageCount)).drawAt(14, panel.center().movedBy(0, -220), ColorF{ 1.0, 0.88, 0.88, 0.92 });

	if (currentPage == 0)
	{
		const RectF causeRect{ panel.x + 28, panel.y + 78, 408, 192 };
		const RectF busyRect{ panel.x + 456, panel.y + 78, 436, 192 };
		const RectF performanceRect{ panel.x + 28, panel.y + 290, 408, 218 };
		const RectF usageRect{ panel.x + 456, panel.y + 290, 436, 218 };
		for (const auto& cardRect : { causeRect, busyRect, performanceRect, usageRect })
		{
			cardRect.rounded(16).draw(ColorF{ 0.14, 0.10, 0.16, 0.92 });
			cardRect.rounded(16).drawFrame(1.5, ColorF{ 0.82, 0.72, 0.88, 0.38 });
		}

		m_font(U"敗因分類").draw(19, causeRect.pos.movedBy(16, 12), Palette::White);
		m_font(U"1. {}"_fmt(causes[0].title)).draw(18, causeRect.pos.movedBy(18, 48), ColorF{ 1.0, 0.86, 0.78, 0.98 });
		m_font(causes[0].detail).draw(13, causeRect.pos.movedBy(18, 78), ColorF{ 0.94, 0.96, 1.0, 0.92 });
		m_font(U"2. {}"_fmt(causes[1].title)).draw(16, causeRect.pos.movedBy(18, 114), ColorF{ 0.98, 0.82, 0.74, 0.96 });
		m_font(causes[1].detail).draw(13, causeRect.pos.movedBy(18, 142), ColorF{ 0.90, 0.94, 1.0, 0.88 });
		m_font(U"次の一手: {}"_fmt(causes[0].advice)).draw(12, causeRect.pos.movedBy(18, 170), ColorF{ 0.88, 0.96, 0.86, 0.90 });

		m_font(U"忙しさピーク").draw(19, busyRect.pos.movedBy(16, 12), Palette::White);
		if (busiestWindow)
		{
			m_font(FormatTimeRangeLabel(busiestWindow->startTime, BusyWindowDuration)).draw(18, busyRect.pos.movedBy(18, 48), ColorF{ 0.82, 0.90, 1.0, 0.98 });
			m_font(U"移動 {:.1f}s / 召喚試行 {} / 成功 {}"_fmt(busiestWindow->movementSeconds, busiestWindow->summonAttempts, busiestWindow->successfulSummons)).draw(14, busyRect.pos.movedBy(18, 86), ColorF{ 0.94, 0.96, 1.0, 0.92 });
			m_font(U"資源不足 {} / 被弾 {} / 忙しさ {:.1f}"_fmt(busiestWindow->deniedSummons, busiestWindow->playerHits, busiestWindow->activityScore)).draw(14, busyRect.pos.movedBy(18, 118), ColorF{ 0.94, 0.96, 1.0, 0.92 });
			m_font(U"忙しい10秒の直前に兵を置いておくと操作が楽になります").draw(12, busyRect.pos.movedBy(18, 154), ColorF{ 0.88, 0.96, 0.86, 0.90 });
		}
		else
		{
			m_font(U"十分なデータがありません").draw(16, busyRect.pos.movedBy(18, 60), ColorF{ 0.94, 0.96, 1.0, 0.92 });
		}

		m_font(U"ユニット所感").draw(19, performanceRect.pos.movedBy(16, 12), Palette::White);
		m_font(bestEfficiencyUnit
			? U"最高効率: {}  ({:.2f} 与ダメ/資源)"_fmt(ff::GetUnitDefinition(*bestEfficiencyUnit).label, bestEfficiencyValue)
			: U"最高効率: 出撃データなし").draw(15, performanceRect.pos.movedBy(18, 52), ColorF{ 0.98, 0.94, 0.72, 0.98 });
		m_font(bestLifetimeUnit
			? U"最長生存: {}  ({:.1f}s)"_fmt(ff::GetUnitDefinition(*bestLifetimeUnit).label, bestLifetimeValue)
			: U"最長生存: 出撃データなし").draw(15, performanceRect.pos.movedBy(18, 88), ColorF{ 0.84, 0.96, 1.0, 0.96 });
		m_font(U"総出撃 {}回 / 総与ダメ {:.1f} / 総コスト {}"_fmt(totalSummons, totalDamage, totalCost)).draw(14, performanceRect.pos.movedBy(18, 126), ColorF{ 0.94, 0.96, 1.0, 0.92 });
		m_font(U"仕事量は現在「与ダメージ換算」です").draw(12, performanceRect.pos.movedBy(18, 156), ColorF{ 0.88, 0.90, 1.0, 0.86 });
		m_font(U"詳細は次ページで兵科ごとの採用率・平均生存・効率を確認できます").draw(12, performanceRect.pos.movedBy(18, 182), ColorF{ 0.88, 0.96, 0.86, 0.90 });

		m_font(U"使っていない兵科").draw(19, usageRect.pos.movedBy(16, 12), Palette::White);
		m_font(unusedUnits.isEmpty() ? U"全兵科を1回以上使っています" : JoinLabels(unusedUnits, U" / ", 4)).draw(15, usageRect.pos.movedBy(18, 52), unusedUnits.isEmpty() ? ColorF{ 0.84, 0.96, 0.86, 0.96 } : ColorF{ 1.0, 0.88, 0.82, 0.96 });
		m_font(U"平均資源 {:.1f} / 現在資源 {} / 低資源帯 {:.1f}s"_fmt(averageResources, m_resourceCount, m_lowResourceTime)).draw(14, usageRect.pos.movedBy(18, 96), ColorF{ 0.94, 0.96, 1.0, 0.92 });
		m_font(U"高資源帯 {:.1f}s / 召喚試行 {} / 資源不足 {}"_fmt(m_highResourceTime, m_totalSummonAttempts, m_totalDeniedSummons)).draw(14, usageRect.pos.movedBy(18, 126), ColorF{ 0.94, 0.96, 1.0, 0.92 });
		m_font(U"未使用兵科が多い時は、編成に役割の近い兵が偏っている可能性があります").draw(12, usageRect.pos.movedBy(18, 162), ColorF{ 0.88, 0.96, 0.86, 0.90 });
		return;
	}

	const int32 unitPageIndex = (currentPage - 1);
	const int32 startIndex = (unitPageIndex * UnitsPerPage);
	const RectF tableRect{ panel.x + 28, panel.y + 82, panel.w - 56, panel.h - 116 };
	tableRect.rounded(16).draw(ColorF{ 0.14, 0.10, 0.16, 0.92 });
	tableRect.rounded(16).drawFrame(1.5, ColorF{ 0.82, 0.72, 0.88, 0.38 });
	m_font(U"兵科別レポート").draw(20, tableRect.pos.movedBy(18, 12), Palette::White);
	m_font(U"採用率=出撃回数比率 / 仕事量=与ダメージ換算").draw(12, tableRect.pos.movedBy(18, 42), ColorF{ 0.88, 0.92, 1.0, 0.86 });

	const double headerY = (tableRect.y + 78);
	m_font(U"兵科").draw(14, Vec2{ tableRect.x + 24, headerY }, ColorF{ 0.94, 0.96, 1.0, 0.92 });
	m_font(U"出撃").draw(14, Vec2{ tableRect.x + 280, headerY }, ColorF{ 0.94, 0.96, 1.0, 0.92 });
	m_font(U"採用率").draw(14, Vec2{ tableRect.x + 400, headerY }, ColorF{ 0.94, 0.96, 1.0, 0.92 });
	m_font(U"平均生存").draw(14, Vec2{ tableRect.x + 530, headerY }, ColorF{ 0.94, 0.96, 1.0, 0.92 });
	m_font(U"仕事/資源").draw(14, Vec2{ tableRect.x + 700, headerY }, ColorF{ 0.94, 0.96, 1.0, 0.92 });

	for (int32 row = 0; row < UnitsPerPage; ++row)
	{
		const int32 unitIndex = (startIndex + row);
		if (unitIndex >= static_cast<int32>(availableUnitIds.size()))
		{
			break;
		}

		const ff::UnitId unitId = availableUnitIds[unitIndex];
		const auto& definition = ff::GetUnitDefinition(unitId);
		const auto& analytics = m_unitBattleAnalytics[ff::ToIndex(unitId)];
		const double usageRate = (totalSummons > 0)
			? (100.0 * static_cast<double>(analytics.summonCount) / static_cast<double>(totalSummons))
			: 0.0;
		const double averageLifetime = (analytics.summonCount > 0)
			? (analytics.totalLifetime / analytics.summonCount)
			: 0.0;
		const double efficiency = (analytics.totalCost > 0)
			? (analytics.totalDamage / analytics.totalCost)
			: 0.0;
		const RectF rowRect{ tableRect.x + 18, headerY + 28 + (row * 62.0), tableRect.w - 36, 52 };
		rowRect.rounded(12).draw((analytics.summonCount > 0) ? ColorF{ 0.18, 0.14, 0.22, 0.92 } : ColorF{ 0.16, 0.12, 0.14, 0.82 });
		rowRect.rounded(12).drawFrame(1.5, definition.color.lerp(Palette::White, 0.18));
		m_font(definition.label).draw(16, rowRect.pos.movedBy(16, 10), Palette::White);
		m_font(definition.roleDescription).draw(11, rowRect.pos.movedBy(16, 30), ColorF{ 0.88, 0.92, 1.0, 0.76 });
		m_font(U"{}回"_fmt(analytics.summonCount)).draw(16, rowRect.pos.movedBy(268, 14), ColorF{ 0.94, 0.96, 1.0, 0.94 });
		m_font(U"{:.0f}%"_fmt(usageRate)).draw(16, rowRect.pos.movedBy(392, 14), ColorF{ 0.94, 0.96, 1.0, 0.94 });
		m_font(U"{:.1f}s"_fmt(averageLifetime)).draw(16, rowRect.pos.movedBy(520, 14), ColorF{ 0.94, 0.96, 1.0, 0.94 });
		m_font((analytics.totalCost > 0) ? U"{:.2f}"_fmt(efficiency) : U"--").draw(16, rowRect.pos.movedBy(700, 14), (analytics.totalCost > 0) ? ColorF{ 0.98, 0.94, 0.72, 0.98 } : ColorF{ 0.76, 0.78, 0.82, 0.80 });
	}
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
