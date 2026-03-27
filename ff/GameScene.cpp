# include "GameScene.h"
# include "IsoMap.h"
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
}

void GameScene::update()
{
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
	m_playerInvincibilityTimer = Max(0.0, (m_playerInvincibilityTimer - Scene::DeltaTime()));
	UpdateStageClearState();

  if (m_stageCleared)
	{
		return;
	}

	if (m_playerHp <= 0.0)
	{
		return;
	}

	UpdateSummoning();
	UpdateSpecialTiles();
	ff::UpdatePlayerPosition(m_playerPos, m_terrain);
	const int32 currentKillReward = ff::GetResourceRewardPerEnemyKill(m_specialTiles[ff::ToTileIndex(m_playerPos)]);
  Array<ff::Enemy> defeatedEnemies;
	bool playerWasHit = false;
	ff::UpdateAllies(m_allies, m_enemies, m_terrain, m_playerPos);
	ff::UpdateEnemies(m_enemies, m_terrain, m_playerPos);
    ff::UpdateAutoCombat(m_allies, m_enemies, m_playerPos, m_playerHp, &defeatedEnemies, (m_playerInvincibilityTimer <= 0.0), &playerWasHit);

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
    const int32 currentKillReward = ff::GetResourceRewardPerEnemyKill(currentSpecialTile);
	const Vec2 worldOrigin = (Scene::Center().movedBy(0, 110) - ff::ToIsometric(m_playerPos));
	const Vec2 playerScreenPos = (worldOrigin + ff::ToIsometric(m_playerPos));

	DrawWorld();
    ff::DrawPlayer(playerScreenPos, (m_playerHp / ff::PlayerMaxHp), (m_playerInvincibilityTimer > 0.0));
	DrawTimeOfDayOverlay();
 ff::DrawHud(m_font, m_enemies.size(), m_allies.size(), m_playerHp, m_resourceCount, currentKillReward, m_currentWave, m_waveActive, pendingEnemyCount, m_nextWaveTimer, currentSpecialTile);

	if (not m_stageCleared)
	{
		ff::DrawSummonAllyButtons(m_font, m_resourceCount, formationSlots, m_deniedSummonSlot, m_deniedSummonTimer);
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
