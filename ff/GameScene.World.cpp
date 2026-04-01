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
        ++m_totalSummonAttempts;
		const int32 busyWindowIndex = static_cast<int32>(m_elapsedBattleTime / 10.0);
		if (m_busyWindowAnalytics.size() <= static_cast<size_t>(busyWindowIndex))
		{
			m_busyWindowAnalytics.resize(busyWindowIndex + 1);
		}

		auto& busyWindow = m_busyWindowAnalytics[busyWindowIndex];
		busyWindow.startTime = (busyWindowIndex * 10.0);
		++busyWindow.summonAttempts;
		busyWindow.activityScore += 1.0;
        const int32 summonCost = ff::GetSummonCost(summonRequest->behavior, m_currentWaveTrait, getData().summonDiscountTraits);

		if (m_resourceCount < summonCost)
		{
          ++m_totalDeniedSummons;
			++busyWindow.deniedSummons;
			busyWindow.activityScore += 0.45;
			m_deniedSummonSlot = summonRequest->slotIndex;
			m_deniedSummonTimer = 0.28;
			return;
		}

     const size_t allyCountBeforeSummon = m_allies.size();

		if (ff::SpawnAlly(m_allies, m_terrain, m_playerPos, summonRequest->behavior))
		{
			m_resourceCount -= summonCost;
          ++busyWindow.successfulSummons;
			m_unitBattleAnalytics[ff::ToIndex(summonRequest->behavior)].summonCount += 1;
			m_unitBattleAnalytics[ff::ToIndex(summonRequest->behavior)].totalCost += summonCost;
			m_activeAllyAnalytics << ActiveAllyAnalytics{ summonRequest->behavior, 0.0 };
           m_deniedSummonSlot.reset();
			m_deniedSummonTimer = 0.0;

			if (m_allies.size() > allyCountBeforeSummon)
			{
				m_summonEffects << SummonEffect{ m_allies.back().pos, 0.32 };
			}
		}
	}
}

void GameScene::UpdateBattleAnalytics(const ff::CombatTelemetry& combatTelemetry, const bool playerWasHit)
{
	if (playerWasHit)
	{
		const int32 busyWindowIndex = static_cast<int32>(m_elapsedBattleTime / 10.0);
		if (m_busyWindowAnalytics.size() <= static_cast<size_t>(busyWindowIndex))
		{
			m_busyWindowAnalytics.resize(busyWindowIndex + 1);
		}

		auto& busyWindow = m_busyWindowAnalytics[busyWindowIndex];
		busyWindow.startTime = (busyWindowIndex * 10.0);
		++busyWindow.playerHits;
		busyWindow.activityScore += 1.2;
	}

	Array<ActiveAllyAnalytics> survivingAnalytics;
	survivingAnalytics.reserve(m_activeAllyAnalytics.size());

	for (size_t index = 0; index < m_activeAllyAnalytics.size(); ++index)
	{
		auto& unitAnalytics = m_unitBattleAnalytics[ff::ToIndex(m_activeAllyAnalytics[index].unitId)];
		if (index < combatTelemetry.allyDamageDealt.size())
		{
			unitAnalytics.totalDamage += combatTelemetry.allyDamageDealt[index];
		}

		const bool survived = ((index < combatTelemetry.allySurvived.size()) ? combatTelemetry.allySurvived[index] : false);
		if (survived)
		{
			survivingAnalytics << m_activeAllyAnalytics[index];
		}
		else
		{
			unitAnalytics.totalLifetime += m_activeAllyAnalytics[index].lifetime;
		}
	}

	m_activeAllyAnalytics = std::move(survivingAnalytics);
}

void GameScene::UpdateDefeatState()
{
	FinalizeBattleAnalytics();
	m_menuOpen = false;

	if (KeyTab.down() || KeyRight.down())
	{
		m_defeatReportPage = ((m_defeatReportPage + 1) % Max(1, GetDefeatReportPageCount()));
		return;
	}

	if (KeyLeft.down())
	{
		const int32 pageCount = Max(1, GetDefeatReportPageCount());
		m_defeatReportPage = ((m_defeatReportPage + pageCount - 1) % pageCount);
		return;
	}

	if (KeyEnter.down() || MouseL.down())
	{
		changeScene(U"Title");
	}
}

void GameScene::FinalizeBattleAnalytics()
{
	if (m_battleAnalyticsFinalized)
	{
		return;
	}

	for (const auto& allyAnalytics : m_activeAllyAnalytics)
	{
		m_unitBattleAnalytics[ff::ToIndex(allyAnalytics.unitId)].totalLifetime += allyAnalytics.lifetime;
	}

	m_activeAllyAnalytics.clear();
	m_battleAnalyticsFinalized = true;
}

int32 GameScene::GetDefeatReportPageCount() const
{
	constexpr int32 UnitsPerPage = 6;
	const int32 unitCount = static_cast<int32>(ff::GetAvailableUnitIds().size());
	return (1 + Max(1, ((unitCount + UnitsPerPage - 1) / UnitsPerPage)));
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
                 ff::DrawEnemy(worldOrigin + ff::ToIsometric(enemy.pos), (enemy.hp / Max(1.0, enemy.maxHp)), enemy.kind);
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
