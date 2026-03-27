# include "RenderWorld.h"
# include "IsoMap.h"

namespace ff
{
	namespace
	{
          void DrawHealthBar(const Vec2& center, const double hpRate, const ColorF& fillColor)
			{
				const RectF backRect{ Arg::center = center, 24, 4 };
				backRect.draw(ColorF{ 0.08, 0.08, 0.08, 0.75 });
				RectF{ backRect.pos, (backRect.w * Clamp(hpRate, 0.0, 1.0)), backRect.h }.draw(fillColor);
				backRect.drawFrame(1, ColorF{ 1.0, 1.0, 1.0, 0.5 });
			}

		void DrawBoundaryBand(const Vec2& edgeStart, const Vec2& edgeEnd, const Vec2& center, const ColorF& color)
		{
			const Vec2 edgeMid = ((edgeStart + edgeEnd) * 0.5);
			const Vec2 offset = (center - edgeMid).setLength(WaterBoundaryHalfWidth);
			Quad{ edgeStart, edgeEnd, (edgeEnd + offset), (edgeStart + offset) }.draw(color);
		}

		void DrawBoundaryBand(const Vec2& edgeStart, const Vec2& edgeEnd, const Vec2& center, const ColorF& color, const bool trimStart, const bool trimEnd)
		{
			const Vec2 edge = (edgeEnd - edgeStart);
			const double edgeLength = edge.length();

			if (edgeLength <= 0.0)
			{
				return;
			}

			const Vec2 edgeDir = (edge / edgeLength);
			const double trimLength = Min(WaterBoundaryHalfWidth, (edgeLength * 0.45));
			const Vec2 start = (trimStart ? (edgeStart + (edgeDir * trimLength)) : edgeStart);
			const Vec2 end = (trimEnd ? (edgeEnd - (edgeDir * trimLength)) : edgeEnd);
			const Vec2 edgeMid = ((start + end) * 0.5);
			const Vec2 offset = (center - edgeMid).setLength(WaterBoundaryHalfWidth);
			Quad{ start, end, (end + offset), (start + offset) }.draw(color);
		}

		Vec2 MakeBoundaryOffset(const Vec2& edgeStart, const Vec2& edgeEnd, const Vec2& center)
		{
			const Vec2 edgeMid = ((edgeStart + edgeEnd) * 0.5);
			return (center - edgeMid).setLength(WaterBoundaryHalfWidth);
		}

		void DrawBoundaryJoin(const Vec2& vertex, const Vec2& offsetA, const Vec2& offsetB, const ColorF& color)
		{
			Quad{ vertex, (vertex + offsetA), (vertex + offsetA + offsetB), (vertex + offsetB) }.draw(color);
		}
	}

	void DrawWaterEdgeOverlay(const Vec2& center, const WaterEdgeMask& edgeMask)
	{
		const Vec2 top = center.movedBy(0, -TileHalfSize.y);
		const Vec2 right = center.movedBy(TileHalfSize.x, 0);
		const Vec2 bottom = center.movedBy(0, TileHalfSize.y);
		const Vec2 left = center.movedBy(-TileHalfSize.x, 0);
		const Vec2 upperRightOffset = MakeBoundaryOffset(top, right, center);
		const Vec2 lowerRightOffset = MakeBoundaryOffset(right, bottom, center);
		const Vec2 lowerLeftOffset = MakeBoundaryOffset(bottom, left, center);
		const Vec2 upperLeftOffset = MakeBoundaryOffset(left, top, center);
		const ColorF overlayColor{ 0.02, 0.08, 0.16, 0.26 };
		const bool topJoin = (edgeMask.topCornerOnly || (edgeMask.upperLeft && edgeMask.upperRight));
		const bool rightJoin = (edgeMask.rightCornerOnly || (edgeMask.upperRight && edgeMask.lowerRight));
		const bool bottomJoin = (edgeMask.bottomCornerOnly || (edgeMask.lowerRight && edgeMask.lowerLeft));
		const bool leftJoin = (edgeMask.leftCornerOnly || (edgeMask.lowerLeft && edgeMask.upperLeft));

		if (edgeMask.upperRight)
		{
			DrawBoundaryBand(top, right, center, overlayColor, topJoin, rightJoin);
		}

		if (edgeMask.lowerRight)
		{
			DrawBoundaryBand(right, bottom, center, overlayColor, rightJoin, bottomJoin);
		}

		if (edgeMask.lowerLeft)
		{
			DrawBoundaryBand(bottom, left, center, overlayColor, bottomJoin, leftJoin);
		}

		if (edgeMask.upperLeft)
		{
			DrawBoundaryBand(left, top, center, overlayColor, leftJoin, topJoin);
		}

		if (topJoin)
		{
			DrawBoundaryJoin(top, upperLeftOffset, upperRightOffset, overlayColor);
		}

		if (bottomJoin)
		{
			DrawBoundaryJoin(bottom, lowerRightOffset, lowerLeftOffset, overlayColor);
		}

		if (rightJoin)
		{
			DrawBoundaryJoin(right, upperRightOffset, lowerRightOffset, overlayColor);
		}

		if (leftJoin)
		{
			DrawBoundaryJoin(left, lowerLeftOffset, upperLeftOffset, overlayColor);
		}
	}

   void DrawSpecialTileOverlay(const Vec2& center, const SpecialTileKind tileKind, const bool active)
	{
		if (tileKind == SpecialTileKind::None)
		{
			return;
		}

		const double pulse = (0.5 + (0.5 * Periodic::Sine1_1(1.4s)));
		const ColorF fillColor = (tileKind == SpecialTileKind::Bonus)
			? ColorF{ 0.96, 0.84, 0.24, (active ? 0.28 : (0.14 + (0.08 * pulse))) }
			: ColorF{ 0.62, 0.24, 0.86, (active ? 0.28 : (0.14 + (0.08 * pulse))) };
		const ColorF frameColor = (tileKind == SpecialTileKind::Bonus)
			? ColorF{ 1.0, 0.94, 0.52, (active ? 0.95 : 0.72) }
			: ColorF{ 0.86, 0.58, 1.0, (active ? 0.95 : 0.72) };

		MakeTileQuad(center).draw(fillColor);
		MakeTileQuad(center).drawFrame(3, frameColor);
		Circle{ center.movedBy(0, -2), 8 }.draw(frameColor);
	}

    void DrawPlayer(const Vec2& screenPos, const double hpRate, const bool invincible)
	{
     const double pulse = (0.5 + (0.5 * Periodic::Sine0_1(1.2s)));
		const ColorF auraColor = invincible
			? ColorF{ 1.0, 0.96, 0.72, (0.34 + (0.16 * pulse)) }
			: ColorF{ 0.52, 0.86, 1.0, (0.22 + (0.10 * pulse)) };
		Ellipse{ screenPos.movedBy(0, 12), (102 + (8 * pulse)), (50 + (4 * pulse)) }.drawFrame(3, auraColor);
		Ellipse{ screenPos.movedBy(0, 12), 78, 38 }.draw(ColorF{ 0.24, 0.60, 0.96, 0.06 });
		Ellipse{ screenPos.movedBy(0, 16), 14, 7 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
      RectF{ Arg::center = screenPos.movedBy(0, -18), 18, 24 }.draw(invincible ? ColorF{ 0.96, 0.92, 0.56 } : ColorF{ 0.28, 0.45, 0.95 });
		Circle{ screenPos.movedBy(0, -38), 10 }.draw(ColorF{ 1.0, 0.88, 0.76 });
		Line{ screenPos.movedBy(-5, -3), screenPos.movedBy(-2, PlayerFootOffsetY) }.draw(3, ColorF{ 0.18, 0.24, 0.35 });
		Line{ screenPos.movedBy(5, -3), screenPos.movedBy(2, PlayerFootOffsetY) }.draw(3, ColorF{ 0.18, 0.24, 0.35 });
        DrawHealthBar(screenPos.movedBy(0, -54), hpRate, invincible ? ColorF{ 0.96, 0.84, 0.38 } : ColorF{ 0.22, 0.64, 0.96 });
	}

   void DrawEnemy(const Vec2& screenPos, const double hpRate, const EnemyKind kind)
	{
     const bool boss = (kind != EnemyKind::Normal);
		const bool trueBoss = (kind == EnemyKind::TrueBoss);
		const double bodyScale = trueBoss ? 1.8 : (boss ? 1.35 : 1.0);
		const ColorF bodyColor = trueBoss
			? ColorF{ 0.44, 0.18, 0.66 }
			: (boss ? ColorF{ 0.74, 0.18, 0.26 } : ColorF{ 0.82, 0.28, 0.32 });
		const ColorF hpColor = trueBoss
			? ColorF{ 0.86, 0.40, 0.98 }
			: (boss ? ColorF{ 1.0, 0.58, 0.30 } : ColorF{ 0.94, 0.34, 0.28 });

		if (boss)
		{
			const double pulse = (0.5 + (0.5 * Periodic::Sine0_1(trueBoss ? 1.2s : 0.9s)));
			Circle{ screenPos.movedBy(0, -14), (18 * bodyScale + (4 * pulse)) }.drawFrame(3, ColorF{ hpColor, (0.36 + (0.18 * pulse)) });
		}

		Ellipse{ screenPos.movedBy(0, 14), (12 * bodyScale), (6 * bodyScale) }.draw(ColorF{ 0.0, 0.0, 0.0, 0.18 });
		RectF{ Arg::center = screenPos.movedBy(0, -16 * bodyScale), (16 * bodyScale), (22 * bodyScale) }.draw(bodyColor);
		Circle{ screenPos.movedBy(0, -34 * bodyScale), (9 * bodyScale) }.draw(ColorF{ 0.96, 0.84, 0.76 });
		Line{ screenPos.movedBy(-4 * bodyScale, -3 * bodyScale), screenPos.movedBy(-2 * bodyScale, 0) }.draw(3, ColorF{ 0.30, 0.12, 0.16 });
		Line{ screenPos.movedBy(4 * bodyScale, -3 * bodyScale), screenPos.movedBy(2 * bodyScale, 0) }.draw(3, ColorF{ 0.30, 0.12, 0.16 });
		DrawHealthBar(screenPos.movedBy(0, -50 * bodyScale), hpRate, hpColor);
	}

       void DrawAlly(const Vec2& screenPos, const double hpRate, const bool commandBuffActive)
	{
		Ellipse{ screenPos.movedBy(0, 15), 12, 6 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.18 });
      if (commandBuffActive)
		{
			const double pulse = (0.5 + (0.5 * Periodic::Sine0_1(0.9s)));
			Circle{ screenPos.movedBy(0, -18), (10 + (3 * pulse)) }.drawFrame(2.5, ColorF{ 0.82, 1.0, 0.72, (0.42 + (0.18 * pulse)) });
		}
		RectF{ Arg::center = screenPos.movedBy(0, -16), 16, 22 }.draw(ColorF{ 0.26, 0.70, 0.42 });
		Circle{ screenPos.movedBy(0, -34), 9 }.draw(ColorF{ 0.96, 0.88, 0.76 });
		Line{ screenPos.movedBy(-4, -3), screenPos.movedBy(-2, 0) }.draw(3, ColorF{ 0.16, 0.30, 0.18 });
		Line{ screenPos.movedBy(4, -3), screenPos.movedBy(2, 0) }.draw(3, ColorF{ 0.16, 0.30, 0.18 });
        if (commandBuffActive)
		{
			RectF{ Arg::center = screenPos.movedBy(0, -16), 16, 22 }.draw(ColorF{ 0.86, 1.0, 0.72, 0.18 });
		}
       DrawHealthBar(screenPos.movedBy(0, -50), hpRate, ColorF{ 0.28, 0.80, 0.42 });
	}

   void DrawHud(const Font& font, const size_t enemyCount, const size_t allyCount, const double playerHp, const int32 resourceCount, const int32 currentKillReward, const int32 currentWave, bool waveActive, const int32 pendingEnemyCount, const double nextWaveTime, const SpecialTileKind tileKind)
	{
        const RectF panel{ 16, 16, 436, 208 };
		const RectF hpCard{ 28, 28, 124, 72 };
		const RectF resourceCard{ 164, 28, 124, 72 };
		const RectF waveCard{ 300, 28, 136, 72 };
		const ColorF panelColor{ 0.96, 0.98, 1.0, 0.76 };
		const auto drawKeyCard = [&](const RectF& rect, const StringView title, const String& value, const String& subLabel, const ColorF& accent)
			{
				rect.rounded(16).draw(ColorF{ 0.08, 0.12, 0.20, 0.84 });
				rect.rounded(16).drawFrame(2, accent.lerp(Palette::White, 0.28));
				font(title).draw(12, rect.pos.movedBy(12, 8), accent.lerp(Palette::White, 0.18));
				font(value).draw(28, rect.pos.movedBy(12, 24), Palette::White);
				font(subLabel).draw(12, rect.pos.movedBy(12, 52), ColorF{ 0.84, 0.90, 0.98, 0.88 });
			};

		panel.rounded(18).draw(panelColor);
		panel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.82, 0.96, 0.62 });

		drawKeyCard(hpCard,
			U"HP",
			U"{:.0f}/{:.0f}"_fmt(playerHp, PlayerMaxHp),
			U"主人公",
			ColorF{ 0.26, 0.56, 0.96 });

		drawKeyCard(resourceCard,
			U"資源",
			U"{}"_fmt(resourceCount),
			U"撃破 +{}"_fmt(currentKillReward),
			ColorF{ 0.86, 0.64, 0.18 });

		drawKeyCard(waveCard,
			U"WAVE",
			waveActive ? U"{}"_fmt(currentWave) : U"Next {}"_fmt(currentWave + 1),
			waveActive ? U"脅威 {}"_fmt(pendingEnemyCount) : U"{:.1f}s"_fmt(Max(0.0, nextWaveTime)),
			ColorF{ 0.62, 0.42, 0.96 });

		font(U"敵 {}  /  味方 {}"_fmt(enemyCount, allyCount)).draw(13, Vec2{ 30, 114 }, ColorF{ 0.20, 0.24, 0.30, 0.92 });
		font(U"WASD: Move / 指揮範囲で近くの味方を強化").draw(13, Vec2{ 30, 136 }, ColorF{ 0.22, 0.28, 0.34, 0.82 });
		font(U"水辺は通行不可 / 夕方・夜は視覚テーマ変化").draw(12, Vec2{ 30, 156 }, ColorF{ 0.26, 0.30, 0.36, 0.72 });

		const String badgeTitle = (tileKind == SpecialTileKind::Bonus)
			? U"BONUS TILE"
			: ((tileKind == SpecialTileKind::Penalty) ? U"PENALTY TILE" : U"NORMAL TILE");
		const String badgeText = U"現在 +{} / 撃破"_fmt(currentKillReward);
		const ColorF badgeColor = (tileKind == SpecialTileKind::Bonus)
			? ColorF{ 0.92, 0.72, 0.16 }
			: ((tileKind == SpecialTileKind::Penalty)
				? ColorF{ 0.64, 0.30, 0.92 }
				: ColorF{ 0.42, 0.48, 0.56 });
		const RectF badgeRect{ 28, 176, 188, 34 };
		badgeRect.rounded(17).draw(ColorF{ badgeColor, 0.16 });
		badgeRect.rounded(17).drawFrame(2, ColorF{ badgeColor, 0.86 });
		font(badgeTitle).draw(12, badgeRect.pos.movedBy(12, 4), badgeColor);
		font(badgeText).draw(12, badgeRect.pos.movedBy(12, 18), ColorF{ 0.22, 0.24, 0.28, 0.92 });
	}
}
