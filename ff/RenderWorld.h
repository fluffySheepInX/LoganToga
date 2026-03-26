# pragma once
# include "GameConstants.h"

namespace ff
{
	void DrawWaterEdgeOverlay(const Vec2& center, const WaterEdgeMask& edgeMask);
     void DrawSpecialTileOverlay(const Vec2& center, SpecialTileKind tileKind, bool active);
 void DrawPlayer(const Vec2& screenPos, double hpRate);
	void DrawEnemy(const Vec2& screenPos, double hpRate);
	void DrawAlly(const Vec2& screenPos, double hpRate);
      void DrawHud(const Font& font, size_t enemyCount, size_t allyCount, double playerHp, int32 resourceCount, int32 currentWave, bool waveActive, int32 pendingEnemyCount, double nextWaveTime, SpecialTileKind tileKind);
}
