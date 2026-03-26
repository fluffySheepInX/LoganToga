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

  void DrawPlayer(const Vec2& screenPos, const double hpRate)
	{
		Ellipse{ screenPos.movedBy(0, 16), 14, 7 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.2 });
		RectF{ Arg::center = screenPos.movedBy(0, -18), 18, 24 }.draw(ColorF{ 0.28, 0.45, 0.95 });
		Circle{ screenPos.movedBy(0, -38), 10 }.draw(ColorF{ 1.0, 0.88, 0.76 });
		Line{ screenPos.movedBy(-5, -3), screenPos.movedBy(-2, PlayerFootOffsetY) }.draw(3, ColorF{ 0.18, 0.24, 0.35 });
		Line{ screenPos.movedBy(5, -3), screenPos.movedBy(2, PlayerFootOffsetY) }.draw(3, ColorF{ 0.18, 0.24, 0.35 });
       DrawHealthBar(screenPos.movedBy(0, -54), hpRate, ColorF{ 0.22, 0.64, 0.96 });
	}

   void DrawEnemy(const Vec2& screenPos, const double hpRate)
	{
		Ellipse{ screenPos.movedBy(0, 14), 12, 6 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.18 });
		RectF{ Arg::center = screenPos.movedBy(0, -16), 16, 22 }.draw(ColorF{ 0.82, 0.28, 0.32 });
		Circle{ screenPos.movedBy(0, -34), 9 }.draw(ColorF{ 0.96, 0.84, 0.76 });
		Line{ screenPos.movedBy(-4, -3), screenPos.movedBy(-2, 0) }.draw(3, ColorF{ 0.30, 0.12, 0.16 });
		Line{ screenPos.movedBy(4, -3), screenPos.movedBy(2, 0) }.draw(3, ColorF{ 0.30, 0.12, 0.16 });
       DrawHealthBar(screenPos.movedBy(0, -50), hpRate, ColorF{ 0.94, 0.34, 0.28 });
	}

    void DrawAlly(const Vec2& screenPos, const double hpRate)
	{
		Ellipse{ screenPos.movedBy(0, 15), 12, 6 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.18 });
		RectF{ Arg::center = screenPos.movedBy(0, -16), 16, 22 }.draw(ColorF{ 0.26, 0.70, 0.42 });
		Circle{ screenPos.movedBy(0, -34), 9 }.draw(ColorF{ 0.96, 0.88, 0.76 });
		Line{ screenPos.movedBy(-4, -3), screenPos.movedBy(-2, 0) }.draw(3, ColorF{ 0.16, 0.30, 0.18 });
		Line{ screenPos.movedBy(4, -3), screenPos.movedBy(2, 0) }.draw(3, ColorF{ 0.16, 0.30, 0.18 });
       DrawHealthBar(screenPos.movedBy(0, -50), hpRate, ColorF{ 0.28, 0.80, 0.42 });
	}

   void DrawHud(const Font& font, const size_t enemyCount, const size_t allyCount, const double playerHp, const int32 resourceCount, const int32 currentWave, const bool waveActive, const int32 pendingEnemyCount, const double nextWaveTime, const SpecialTileKind tileKind)
	{
        RectF{ 16, 16, 340, 210 }.draw(ColorF{ 1.0, 1.0, 1.0, 0.72 });
		font(U"WASD: Move").draw(24, 24, ColorF{ 0.1, 0.14, 0.2 });
		font(U"Quarter view world / 水辺は通行不可").draw(24, 48, ColorF{ 0.1, 0.14, 0.2 });
		font(U"外周から敵が接近中: {}"_fmt(enemyCount)).draw(24, 72, ColorF{ 0.45, 0.12, 0.16 });
      font(U"味方数: {}"_fmt(allyCount)).draw(24, 96, ColorF{ 0.10, 0.34, 0.16 });
		font(U"主人公 HP: {:.0f} / {:.0f}"_fmt(playerHp, PlayerMaxHp)).draw(24, 120, ColorF{ 0.16, 0.26, 0.58 });
       font(U"資源: {}  (+{} / 撃破)"_fmt(resourceCount, ResourcePerEnemyKill)).draw(24, 144, ColorF{ 0.58, 0.42, 0.10 });
       font(waveActive
			? U"Wave {}  残り脅威: {}"_fmt(currentWave, pendingEnemyCount)
			: U"次の Wave {} まで {:.1f}s"_fmt((currentWave + 1), Max(0.0, nextWaveTime))).draw(24, 168, ColorF{ 0.36, 0.20, 0.60 });
       const String tileStatus = (tileKind == SpecialTileKind::Bonus)
			? U"ボーナスタイル: 撃破資源 +{}"_fmt(BonusTileResourcePerEnemyKill)
			: ((tileKind == SpecialTileKind::Penalty)
				? U"ペナルティタイル: 撃破資源 +{}"_fmt(PenaltyTileResourcePerEnemyKill)
				: U"通常タイル: 撃破資源 +{}"_fmt(ResourcePerEnemyKill));
		const ColorF tileStatusColor = (tileKind == SpecialTileKind::Bonus)
			? ColorF{ 0.78, 0.56, 0.08 }
			: ((tileKind == SpecialTileKind::Penalty)
				? ColorF{ 0.54, 0.20, 0.70 }
				: ColorF{ 0.28, 0.28, 0.32 });
		font(tileStatus).draw(24, 192, tileStatusColor);
	}
}
