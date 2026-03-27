# pragma once
# include "AppState.h"

class GameScene : public App::Scene
{
public:
	using InitData = App::Scene::InitData;

	GameScene(const InitData& init);

	void update() override;
	void draw() const override;

private:
	bool UpdateMenu();
 void UpdateTimeOfDayButtons();
	void UpdateSummoning();
  void UpdateSummonFeedback();
  void UpdateResourceGainPopups();
	void UpdateSpecialTiles();
 void UpdateStageClearState();
	void UpdateWaveState();
    void SetTimeOfDay(ff::TimeOfDay timeOfDay);
	void DrawWorld() const;
    void DrawSummonEffects(const Vec2& worldOrigin) const;
    void DrawResourceGainPopups(const Vec2& worldOrigin) const;
  void DrawTimeOfDayOverlay() const;
	void DrawTimeOfDayButtons() const;
	void DrawWaveBanner() const;
	void DrawDefeatMessage() const;
   void DrawVictoryMessage() const;
	void DrawMenuOverlay() const;
   RectF GetEveningButton() const;
	RectF GetNightButton() const;
	RectF GetResumeButton() const;
	RectF GetExitGameButton() const;
	RectF GetBackToTitleButton() const;

	struct SummonEffect
	{
		Vec2 pos;
		double timer = 0.0;
	};

	struct ResourceGainPopup
	{
		Vec2 pos;
		int32 amount = 0;
		double timer = 0.0;
	};

	Array<Texture> m_tileTextures;
	const ff::TerrainGrid m_terrain;
	ff::SpecialTileGrid m_specialTiles;
	bool m_menuOpen = false;
	bool m_specialTilesVisible = true;
	double m_specialTileTimer = ff::SpecialTileVisibleDuration;
	const Array<Point> m_enemySpawnTiles;
	Vec2 m_playerPos{ 12.0, 12.0 };
	double m_playerHp = ff::PlayerMaxHp;
   double m_playerInvincibilityTimer = 0.0;
	int32 m_resourceCount = ff::InitialResources;
	Array<ff::Ally> m_allies;
	Array<ff::Enemy> m_enemies;
    Array<ff::EnemyKind> m_pendingWaveEnemies;
	int32 m_currentWave = 0;
	int32 m_enemiesToSpawnInWave = 0;
	int32 m_enemiesSpawnedInWave = 0;
	bool m_waveActive = false;
 bool m_stageCleared = false;
 double m_stageClearTransitionTimer = ff::StageClearReturnDelay;
	double m_enemySpawnTimer = 0.0;
	double m_nextWaveTimer = ff::WaveStartDelay;
	double m_waveBannerTimer = ff::WaveBannerDuration;
  Optional<size_t> m_deniedSummonSlot;
	double m_deniedSummonTimer = 0.0;
	Array<SummonEffect> m_summonEffects;
  Array<ResourceGainPopup> m_resourceGainPopups;
	Font m_font{ 18 };
};
