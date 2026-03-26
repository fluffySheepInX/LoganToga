# include "libs/AddonGaussian.h"
# include <Siv3D.hpp> // Siv3D v0.6.16
# include "libs/AddonEffFs.h"
# include "GameConstants.h"
# include "IsoMap.h"
# include "RenderWorld.h"
# include "Terrain.h"
# include "TextureAssets.h"
# include "UI.h"
# include "UnitLogic.h"

using App = SceneManager<String>;

namespace
{
  void DrawMenuButton(const RectF& rect, const Font& font, const String& label)
	{
		const bool hovered = rect.mouseOver();
		const ColorF fillColor = hovered ? ColorF{ 0.34, 0.49, 0.80, 0.92 } : ColorF{ 0.20, 0.29, 0.54, 0.88 };
		const ColorF frameColor = hovered ? ColorF{ 0.95, 0.98, 1.0, 0.95 } : ColorF{ 0.80, 0.88, 1.0, 0.72 };

		rect.rounded(12).draw(fillColor);
		rect.rounded(12).drawFrame(2, frameColor);
		font(label).drawAt(24, rect.center(), Palette::White);
	}

	class TitleScene : public App::Scene
	{
	public:
		TitleScene(const InitData& init)
			: IScene{ init }
			, m_titleFont{ 48, Typeface::Heavy }
			, m_buttonFont{ 24 }
		{
			Scene::SetBackground(ColorF{ 0.10, 0.16, 0.28 });
		}

		void update() override
		{
			if (GetStartButton().leftClicked())
			{
				changeScene(U"Game");
				return;
			}

			if (GetExitButton().leftClicked())
			{
				System::Exit();
			}
		}

		void draw() const override
		{
			const RectF panel{ Arg::center = Scene::Center(), 460, 320 };

			panel.rounded(24).draw(ColorF{ 0.08, 0.12, 0.22, 0.86 });
			panel.rounded(24).drawFrame(2, ColorF{ 0.84, 0.90, 1.0, 0.70 });
			m_titleFont(U"LoganToga").drawAt(Scene::Center().movedBy(0, -80), ColorF{ 0.97, 0.98, 1.0 });
			m_buttonFont(U"Press a button").drawAt(Scene::Center().movedBy(0, -24), ColorF{ 0.82, 0.88, 1.0, 0.86 });

          DrawMenuButton(GetStartButton(), m_buttonFont, U"Start");
			DrawMenuButton(GetExitButton(), m_buttonFont, U"Exit");
		}

	private:
		RectF GetStartButton() const
		{
			return RectF{ Arg::center = Scene::Center().movedBy(0, 40), 240, 56 };
		}

		RectF GetExitButton() const
		{
			return RectF{ Arg::center = Scene::Center().movedBy(0, 116), 240, 56 };
		}

		Font m_titleFont;
		Font m_buttonFont;
	};

	class GameScene : public App::Scene
	{
	public:
		GameScene(const InitData& init)
			: IScene{ init }
			, m_tileTextures{ ff::LoadTerrainTextures() }
			, m_terrain{ ff::MakeTerrain() }
			, m_specialTiles{ ff::MakeRandomSpecialTiles(m_terrain) }
			, m_enemySpawnTiles{ ff::CollectEnemySpawnTiles(m_terrain) }
		{
			Scene::SetBackground(ColorF{ 0.60, 0.78, 0.96 });
		}

		void update() override
		{
            if (KeyEscape.down())
			{
				m_menuOpen = (not m_menuOpen);
			}

			if (m_menuOpen)
			{
				if (GetResumeButton().leftClicked())
				{
					m_menuOpen = false;
					return;
				}

				if (GetExitGameButton().leftClicked())
				{
					System::Exit();
					return;
				}

				if (GetBackToTitleButton().leftClicked())
				{
					changeScene(U"Title");
					return;
				}

				return;
			}

			const bool playerAlive = (m_playerHp > 0.0);

			if (playerAlive)
			{
				if (const auto summonBehavior = ff::CheckSummonAllyButtonPressed())
				{
					const int32 summonCost = ff::GetSummonCost(*summonBehavior);

					if ((m_resourceCount >= summonCost)
						&& ff::SpawnAlly(m_allies, m_terrain, m_playerPos, *summonBehavior))
					{
						m_resourceCount -= summonCost;
					}
				}
			}

			if (playerAlive)
			{
				m_specialTileTimer -= Scene::DeltaTime();

				if (m_specialTileTimer <= 0.0)
				{
					m_specialTilesVisible = (not m_specialTilesVisible);

					if (m_specialTilesVisible)
					{
						m_specialTiles = ff::MakeRandomSpecialTiles(m_terrain);
						m_specialTileTimer = ff::SpecialTileVisibleDuration;
					}
					else
					{
						m_specialTiles = ff::MakeEmptySpecialTiles();
						m_specialTileTimer = ff::SpecialTileHiddenDuration;
					}
				}

				ff::UpdatePlayerPosition(m_playerPos, m_terrain);
				const int32 currentKillReward = ff::GetResourceRewardPerEnemyKill(m_specialTiles[ff::ToTileIndex(m_playerPos)]);
				ff::UpdateAllies(m_allies, m_enemies, m_terrain, m_playerPos);
				ff::UpdateEnemies(m_enemies, m_terrain, m_playerPos);
				m_resourceCount += (ff::UpdateAutoCombat(m_allies, m_enemies, m_playerPos, m_playerHp) * currentKillReward);

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
				}
				else
				{
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

				m_waveBannerTimer = Max(0.0, (m_waveBannerTimer - Scene::DeltaTime()));
			}
		}

		void draw() const override
		{
			const int32 pendingEnemyCount = m_waveActive
				? (static_cast<int32>(m_enemies.size()) + Max(0, (m_enemiesToSpawnInWave - m_enemiesSpawnedInWave)))
				: 0;

			const Point playerTile = ff::ToTileIndex(m_playerPos);
			const ff::SpecialTileKind currentSpecialTile = m_specialTiles[playerTile];
			const Vec2 worldOrigin = (Scene::Center().movedBy(0, 110) - ff::ToIsometric(m_playerPos));
			const Vec2 playerScreenPos = (worldOrigin + ff::ToIsometric(m_playerPos));

			{
				const ScopedRenderStates2D blend{ BlendState::Premultiplied };

				ff::ForEachTileByDepth([&](const Point& tileIndex)
					{
						const Vec2 tileBottomCenter = ff::ToTileBottomCenter(tileIndex, worldOrigin);
						m_tileTextures[ff::ToTextureIndex(m_terrain[tileIndex])].draw(Arg::bottomCenter = tileBottomCenter);
					});
			}

			ff::ForEachTileByDepth([&](const Point& tileIndex)
				{
					const Vec2 tileScreenPos = ff::ToScreenPos(tileIndex, worldOrigin);

					if (ff::IsWaterTile(m_terrain[tileIndex]))
					{
						//ff::DrawWaterEdgeOverlay(tileScreenPos, ff::GetWaterEdgeMask(m_terrain, tileIndex));
					}

					if (tileIndex == playerTile)
					{
						ff::MakeTileQuad(tileScreenPos).draw(ColorF{ 1.0, 0.75, 0.2, 0.16 });
						ff::MakeTileQuad(tileScreenPos).drawFrame(2, ColorF{ 1.0, 0.86, 0.38, 0.45 });
					}

					ff::DrawSpecialTileOverlay(tileScreenPos, m_specialTiles[tileIndex], (tileIndex == playerTile));

					for (const auto& enemy : m_enemies)
					{
						if (ff::ToTileIndex(enemy.pos) == tileIndex)
						{
							ff::DrawEnemy(worldOrigin + ff::ToIsometric(enemy.pos), (enemy.hp / ff::EnemyMaxHp));
						}
					}

					for (const auto& ally : m_allies)
					{
						if (ff::ToTileIndex(ally.pos) == tileIndex)
						{
							ff::DrawAlly(worldOrigin + ff::ToIsometric(ally.pos), (ally.hp / ff::AllyMaxHp));
						}
					}
				});

			ff::DrawPlayer(playerScreenPos, (m_playerHp / ff::PlayerMaxHp));
			ff::DrawHud(m_font, m_enemies.size(), m_allies.size(), m_playerHp, m_resourceCount, m_currentWave, m_waveActive, pendingEnemyCount, m_nextWaveTimer, currentSpecialTile);
			ff::DrawSummonAllyButtons(m_font, m_resourceCount);

			if ((m_waveBannerTimer > 0.0) && (m_currentWave > 0))
			{
				const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 240, 46 };
				const double alpha = Min(1.0, (m_waveBannerTimer / ff::WaveBannerDuration));
				waveRect.rounded(12).draw(ColorF{ 0.16, 0.10, 0.30, (0.72 * alpha) });
				waveRect.rounded(12).drawFrame(2, ColorF{ 0.92, 0.88, 1.0, (0.86 * alpha) });
				m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center(), ColorF{ 1.0, 1.0, 1.0, alpha });
			}

			if (m_playerHp <= 0.0)
			{
				const RectF messageRect{ Arg::center = Scene::Center(), 300, 52 };
				messageRect.rounded(12).draw(ColorF{ 0.24, 0.04, 0.04, 0.88 });
				messageRect.rounded(12).drawFrame(2, ColorF{ 0.92, 0.54, 0.48, 0.92 });
				m_font(U"主人公が撃破されました").drawAt(20, messageRect.center(), Palette::White);
			}

			if (m_menuOpen)
			{
				const RectF overlay{ Scene::Rect() };
				const RectF menuRect{ Arg::center = Scene::Center(), 340, 280 };

				overlay.draw(ColorF{ 0.0, 0.0, 0.0, 0.45 });
				menuRect.rounded(20).draw(ColorF{ 0.08, 0.12, 0.22, 0.92 });
				menuRect.rounded(20).drawFrame(2, ColorF{ 0.88, 0.92, 1.0, 0.78 });
				m_font(U"メニュー").drawAt(24, menuRect.center().movedBy(0, -92), Palette::White);

				DrawMenuButton(GetResumeButton(), m_font, U"続行");
				DrawMenuButton(GetExitGameButton(), m_font, U"ゲーム終了");
				DrawMenuButton(GetBackToTitleButton(), m_font, U"タイトルバック");
			}
		}

	private:
      RectF GetResumeButton() const
		{
			return RectF{ Arg::center = Scene::Center().movedBy(0, -24), 220, 48 };
		}

		RectF GetExitGameButton() const
		{
			return RectF{ Arg::center = Scene::Center().movedBy(0, 36), 220, 48 };
		}

		RectF GetBackToTitleButton() const
		{
			return RectF{ Arg::center = Scene::Center().movedBy(0, 96), 220, 48 };
		}

		Array<Texture> m_tileTextures;
		const ff::TerrainGrid m_terrain;
		ff::SpecialTileGrid m_specialTiles;
      bool m_menuOpen = false;
		bool m_specialTilesVisible = true;
		double m_specialTileTimer = ff::SpecialTileVisibleDuration;
		const Array<Point> m_enemySpawnTiles;
		Vec2 m_playerPos{ 12.0, 12.0 };
		double m_playerHp = ff::PlayerMaxHp;
		int32 m_resourceCount = ff::InitialResources;
		Array<ff::Ally> m_allies;
		Array<ff::Enemy> m_enemies;
		int32 m_currentWave = 0;
		int32 m_enemiesToSpawnInWave = 0;
		int32 m_enemiesSpawnedInWave = 0;
		bool m_waveActive = false;
		double m_enemySpawnTimer = 0.0;
		double m_nextWaveTimer = ff::WaveStartDelay;
		double m_waveBannerTimer = ff::WaveBannerDuration;
		Font m_font{ 18 };
	};
}

void Main()
{
#pragma region Addon
	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,900 });
	GaussianFSAddon::SetLangSet({
		{ U"Japan",     U"日本語" },
		{ U"English",   U"English" },
		{ U"Deutsch",   U"Deutsch" },
		{ U"Test",      U"TestLang" },
		});
	GaussianFSAddon::SetLang(U"Japan");
	GaussianFSAddon::SetSceneSet({
		{ U"1600*900", U"1600",U"900"},
		{ U"1200*600", U"1200",U"600"},
		});
	GaussianFSAddon::SetScene(U"1600*900");

	Addon::Register<AddonEffFs>(U"AddonEffFs");
	AddonEffFs::SetKind(AddonEffFs::EffectKind::Wind);
	AddonEffFs::SetColor(ColorF(0.9, 0.95, 1.0, 0.8));
	AddonEffFs::SetSpawnDensity(0.05, 0.25);
	//AddonEffFs::SetDirection(Vec2{ 0.5, 1.0 });       // 斜めの豪雨
	//AddonEffFs::AddImpulse(1.0, 5.0);                 // 強雨タイム
#pragma endregion

	App manager;
	manager.add<TitleScene>(U"Title");
	manager.add<GameScene>(U"Game");

	while (System::Update())
	{
#pragma region Addon
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
		AddonEffFs::SetDeltaTime(Scene::DeltaTime());
#pragma endregion

		if (not manager.update())
		{
			break;
		}
	}
}
