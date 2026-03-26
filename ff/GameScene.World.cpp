# include "GameScene.h"
# include "IsoMap.h"
# include "RenderWorld.h"
# include "Terrain.h"
# include "UI.h"
# include "UnitLogic.h"

namespace
{
	ColorF GetTimeOfDayBackgroundColor(const ff::TimeOfDay timeOfDay)
	{
		switch (timeOfDay)
		{
		case ff::TimeOfDay::Evening:
			return ColorF{ 0.92, 0.56, 0.38 };
		case ff::TimeOfDay::Night:
			return ColorF{ 0.08, 0.10, 0.20 };
		default:
			return ColorF{ 0.60, 0.78, 0.96 };
		}
	}

	ColorF GetTimeOfDayOverlayColor(const ff::TimeOfDay timeOfDay)
	{
		switch (timeOfDay)
		{
		case ff::TimeOfDay::Evening:
			return ColorF{ 0.94, 0.42, 0.12, 0.18 };
		case ff::TimeOfDay::Night:
			return ColorF{ 0.04, 0.08, 0.20, 0.48 };
		default:
			return ColorF{ 0.0, 0.0, 0.0, 0.0 };
		}
	}
}

void GameScene::UpdateSummoning()
{
 if (const auto summonRequest = ff::CheckSummonAllyButtonPressed(getData().formationSlots))
	{
        const int32 summonCost = ff::GetSummonCost(summonRequest->behavior);

		if (m_resourceCount < summonCost)
		{
			m_deniedSummonSlot = summonRequest->slotIndex;
			m_deniedSummonTimer = 0.28;
			return;
		}

     const size_t allyCountBeforeSummon = m_allies.size();

		if (ff::SpawnAlly(m_allies, m_terrain, m_playerPos, summonRequest->behavior))
		{
			m_resourceCount -= summonCost;
           m_deniedSummonSlot.reset();
			m_deniedSummonTimer = 0.0;

			if (m_allies.size() > allyCountBeforeSummon)
			{
				m_summonEffects << SummonEffect{ m_allies.back().pos, 0.32 };
			}
		}
	}
}

void GameScene::UpdateSummonFeedback()
{
	m_deniedSummonTimer = Max(0.0, (m_deniedSummonTimer - Scene::DeltaTime()));

	if (m_deniedSummonTimer <= 0.0)
	{
		m_deniedSummonSlot.reset();
	}

	for (auto& effect : m_summonEffects)
	{
		effect.timer -= Scene::DeltaTime();
	}

	m_summonEffects.remove_if([](const SummonEffect& effect)
		{
			return (effect.timer <= 0.0);
		});
}

void GameScene::UpdateResourceGainPopups()
{
	for (auto& popup : m_resourceGainPopups)
	{
		popup.timer -= Scene::DeltaTime();
	}

	m_resourceGainPopups.remove_if([](const ResourceGainPopup& popup)
		{
			return (popup.timer <= 0.0);
		});
}

void GameScene::UpdateSpecialTiles()
{
	m_specialTileTimer -= Scene::DeltaTime();

	if (m_specialTileTimer > 0.0)
	{
		return;
	}

	m_specialTilesVisible = (not m_specialTilesVisible);

	if (m_specialTilesVisible)
	{
		m_specialTiles = ff::MakeRandomSpecialTiles(m_terrain);
		m_specialTileTimer = ff::SpecialTileVisibleDuration;
		return;
	}

	m_specialTiles = ff::MakeEmptySpecialTiles();
	m_specialTileTimer = ff::SpecialTileHiddenDuration;
}

void GameScene::SetTimeOfDay(const ff::TimeOfDay timeOfDay)
{
	getData().timeOfDay = timeOfDay;
	Scene::SetBackground(GetTimeOfDayBackgroundColor(timeOfDay));
}

void GameScene::DrawWorld() const
{
	const Point playerTile = ff::ToTileIndex(m_playerPos);
	const Vec2 worldOrigin = (Scene::Center().movedBy(0, 110) - ff::ToIsometric(m_playerPos));

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
         ff::DrawAlly(worldOrigin + ff::ToIsometric(ally.pos), (ally.hp / ff::GetAllyMaxHp(ally.behavior)), ff::IsWithinPlayerCommandRange(ally.pos, m_playerPos));
				}
			}
		});

	DrawSummonEffects(worldOrigin);
   DrawResourceGainPopups(worldOrigin);
}

void GameScene::DrawSummonEffects(const Vec2& worldOrigin) const
{
	for (const auto& effect : m_summonEffects)
	{
		const double t = (1.0 - Min(1.0, (effect.timer / 0.32)));
		const double alpha = (1.0 - t);
		const Vec2 screenPos = (worldOrigin + ff::ToIsometric(effect.pos));
		const Vec2 ringCenter = screenPos.movedBy(0, -10);
		Circle{ ringCenter, (10 + (26 * t)) }.drawFrame((3.5 - (2.0 * t)), ColorF{ 0.72, 1.0, 0.78, (0.88 * alpha) });
		ff::MakeTileQuad(ff::ToScreenPos(ff::ToTileIndex(effect.pos), worldOrigin)).draw(ColorF{ 0.46, 0.92, 0.58, (0.12 * alpha) });
	}
}

void GameScene::DrawResourceGainPopups(const Vec2& worldOrigin) const
{
	for (const auto& popup : m_resourceGainPopups)
	{
		const double t = (1.0 - Min(1.0, (popup.timer / 0.75)));
		const double alpha = (1.0 - t);
		const Vec2 screenPos = (worldOrigin + ff::ToIsometric(popup.pos)).movedBy(0, (-38 - (20 * t)));
		const String label = U"+{}"_fmt(popup.amount);
		m_font(label).drawAt(18, screenPos.movedBy(0, 1), ColorF{ 0.16, 0.10, 0.04, (0.54 * alpha) });
		m_font(label).drawAt(18, screenPos, ColorF{ 1.0, 0.92, 0.42, (0.96 * alpha) });
	}
}

void GameScene::DrawTimeOfDayOverlay() const
{
	const ColorF overlayColor = GetTimeOfDayOverlayColor(getData().timeOfDay);

	if (overlayColor.a <= 0.0)
	{
		return;
	}

	Scene::Rect().draw(overlayColor);
}
