# include "GameScene.h"
# include "IsoMap.h"
# include "ResourceBalance.h"
# include "RenderWorld.h"
# include "Terrain.h"
# include "TextureAssets.h"
# include "UI.h"
# include "UnitLogic.h"

GameScene::GameScene(const InitData& init)
	: App::Scene{ init }
	, m_tileTextures{ ff::LoadTerrainTextures() }
	, m_terrain{ ff::MakeTerrain() }
	, m_specialTiles{ ff::MakeOpeningSpecialTiles(m_terrain, ff::ToTileIndex(m_playerPos)) }
	, m_enemySpawnTiles{ ff::CollectEnemySpawnTiles(m_terrain) }
{
	SetTimeOfDay(getData().timeOfDay);
   m_resourceIntegral = static_cast<double>(m_resourceCount);
}

void GameScene::update()
{
   if (m_playerHp <= 0.0)
	{
		UpdateDefeatState();
		return;
	}

	if (KeyEscape.down())
	{
		m_menuOpen = (not m_menuOpen);
	}

	if (UpdateMenu())
	{
		return;
	}

	UpdateTimeOfDayButtons();
	UpdateSummonFeedback();
	UpdateResourceGainPopups();
   UpdateRushTileBoost();
	m_playerInvincibilityTimer = Max(0.0, (m_playerInvincibilityTimer - Scene::DeltaTime()));
	UpdateStageClearState();

  if (m_stageCleared)
	{
		return;
	}

  const double deltaTime = Scene::DeltaTime();
	const double passiveResourcePerSecond = ff::GetPassiveResourcePerSecond();
	if (passiveResourcePerSecond > 0.0)
	{
		m_passiveResourceAccumulator += (passiveResourcePerSecond * deltaTime);
		const int32 gainedPassiveResourceCount = static_cast<int32>(m_passiveResourceAccumulator);
		if (gainedPassiveResourceCount > 0)
		{
			m_passiveResourceAccumulator -= gainedPassiveResourceCount;
			m_resourceCount += gainedPassiveResourceCount;
		}
	}

	m_elapsedBattleTime += deltaTime;
	m_resourceIntegral += (static_cast<double>(m_resourceCount) * deltaTime);

	if (m_resourceCount <= 2)
	{
		m_lowResourceTime += deltaTime;
	}

	if (m_resourceCount >= 10)
	{
		m_highResourceTime += deltaTime;
	}

	if (ff::GetMovementInput() != Vec2{ 0.0, 0.0 })
	{
		const int32 busyWindowIndex = static_cast<int32>(m_elapsedBattleTime / 10.0);
		if (m_busyWindowAnalytics.size() <= static_cast<size_t>(busyWindowIndex))
		{
			m_busyWindowAnalytics.resize(busyWindowIndex + 1);
		}

		auto& busyWindow = m_busyWindowAnalytics[busyWindowIndex];
		busyWindow.startTime = (busyWindowIndex * 10.0);
		busyWindow.movementSeconds += deltaTime;
		busyWindow.activityScore += (0.55 * deltaTime);
	}

	for (auto& allyAnalytics : m_activeAllyAnalytics)
	{
		allyAnalytics.lifetime += deltaTime;
	}

  UpdateSummoning();
	UpdateSpecialTiles();
	ff::UpdatePlayerPosition(m_playerPos, m_terrain, ((m_rushTileBoostTimer > 0.0) ? ff::RushTilePlayerSpeedMultiplier : 1.0));
	const Point playerTileIndex = ff::ToTileIndex(m_playerPos);
	if (m_specialTiles[playerTileIndex] == ff::SpecialTileKind::Rush)
	{
		m_rushTileBoostTimer = ff::RushTileBoostDuration;
		m_specialTiles[playerTileIndex] = ff::SpecialTileKind::None;
	}
 const ff::SpecialTileKind currentPlayerTileKind = m_specialTiles[playerTileIndex];
	const int32 currentKillReward = (ff::GetResourceRewardPerEnemyKill(currentPlayerTileKind)
		+ (m_waveActive ? m_currentWaveRewardBonusPerKill : 0));
  Array<ff::Enemy> defeatedEnemies;
	bool playerWasHit = false;
  ff::UpdateAllies(m_allies, m_enemies, m_terrain, m_playerPos, ((m_rushTileBoostTimer > 0.0) ? ff::RushTileAllySpeedMultiplier : 1.0));
	ff::UpdateEnemies(m_enemies, m_terrain, m_playerPos);
   ff::CombatTelemetry combatTelemetry;
   ff::UpdateAutoCombat(m_allies, m_enemies, m_playerPos, m_playerHp, ff::GetAllyAttackIntervalMultiplier(currentPlayerTileKind), &defeatedEnemies, (m_playerInvincibilityTimer <= 0.0), &playerWasHit, &combatTelemetry);
    UpdateBattleAnalytics(combatTelemetry, playerWasHit);

	if (playerWasHit)
	{
		m_playerInvincibilityTimer = ff::PlayerHitInvincibilityDuration;
	}

    int32 gainedResourceCount = 0;
	bool trueBossDefeated = false;

  for (size_t index = 0; index < defeatedEnemies.size(); ++index)
	{
       const int32 rewardAmount = (currentKillReward * ff::GetEnemyRewardMultiplier(defeatedEnemies[index].kind));
		gainedResourceCount += rewardAmount;
		m_resourceGainPopups << ResourceGainPopup{ defeatedEnemies[index].pos.movedBy(0, (-0.16 * static_cast<double>(index % 3))), rewardAmount, 0.75 };

		if (defeatedEnemies[index].kind == ff::EnemyKind::TrueBoss)
		{
			trueBossDefeated = true;
		}
	}

	m_resourceCount += gainedResourceCount;

	if (trueBossDefeated)
	{
      FinalizeBattleAnalytics();
		m_stageCleared = true;
       m_menuOpen = false;
		m_waveActive = false;
        m_stageClearTransitionTimer = ff::StageClearReturnDelay;
		m_enemySpawnTimer = 0.0;
		m_nextWaveTimer = 0.0;
		m_waveBannerTimer = 0.0;
		m_pendingWaveEnemies.clear();
		return;
	}

	UpdateWaveState();
	m_waveBannerTimer = Max(0.0, (m_waveBannerTimer - Scene::DeltaTime()));
}

void GameScene::draw() const
{
	const auto& formationSlots = getData().formationSlots;
	const int32 pendingEnemyCount = m_waveActive
		? (static_cast<int32>(m_enemies.size()) + Max(0, (m_enemiesToSpawnInWave - m_enemiesSpawnedInWave)))
		: 0;
	const Point playerTile = ff::ToTileIndex(m_playerPos);
	const ff::SpecialTileKind currentSpecialTile = m_specialTiles[playerTile];
  const int32 currentKillReward = (ff::GetResourceRewardPerEnemyKill(currentSpecialTile)
		+ (m_waveActive ? m_currentWaveRewardBonusPerKill : 0));
	const Vec2 worldOrigin = (Scene::Center().movedBy(0, 110) - ff::ToIsometric(m_playerPos));
	const Vec2 playerScreenPos = (worldOrigin + ff::ToIsometric(m_playerPos));

	DrawWorld();
    ff::DrawPlayer(playerScreenPos, (m_playerHp / ff::PlayerMaxHp), (m_playerInvincibilityTimer > 0.0), (m_rushTileBoostTimer > 0.0));
	DrawTimeOfDayOverlay();
  ff::DrawHud(m_font, m_enemies.size(), m_allies.size(), m_playerHp, m_resourceCount, currentKillReward, ff::GetPassiveResourcePerSecond(), m_currentWave, m_waveActive, pendingEnemyCount, m_nextWaveTimer, currentSpecialTile, m_rushTileBoostTimer, (m_waveActive ? String{ ff::GetWaveTraitLabel(m_currentWaveTrait) } : U""));


	if (not m_stageCleared)
	{
        ff::DrawSummonAllyButtons(m_font, m_resourceCount, formationSlots, m_currentWaveTrait, getData().summonDiscountTraits, m_deniedSummonSlot, m_deniedSummonTimer);
		DrawTimeOfDayButtons();
	}

	DrawWaveBanner();
	DrawDefeatMessage();
	DrawVictoryMessage();

	if (m_menuOpen)
	{
		DrawMenuOverlay();
	}
}
