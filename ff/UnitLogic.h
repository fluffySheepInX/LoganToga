# pragma once
# include "GameConstants.h"

namespace ff
{
   struct CombatTelemetry
	{
		Array<double> allyDamageDealt;
		Array<bool> allySurvived;
	};

	Optional<Vec2> FindClosestEnemyPos(const Array<Enemy>& enemies, const Vec2& fromPos);
 bool SpawnEnemy(Array<Enemy>& enemies, const Array<Point>& spawnTiles, EnemyKind kind = EnemyKind::Normal, double hpMultiplier = 1.0, double speedMultiplier = 1.0, double attackIntervalMultiplier = 1.0);
	void UpdateEnemies(Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos);
 void UpdateAllies(Array<Ally>& allies, const Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos, double speedMultiplier = 1.0);
 bool SpawnAlly(Array<Ally>& allies, const TerrainGrid& terrain, const Vec2& playerPos, AllyBehavior behavior);
    int32 UpdateAutoCombat(Array<Ally>& allies, Array<Enemy>& enemies, const Vec2& playerPos, double& playerHp, double allyAttackIntervalMultiplier = 1.0, Array<Enemy>* defeatedEnemies = nullptr, bool canDamagePlayer = true, bool* playerWasHit = nullptr, CombatTelemetry* combatTelemetry = nullptr);
	Vec2 GetMovementInput();
 void UpdatePlayerPosition(Vec2& playerPos, const TerrainGrid& terrain, double speedMultiplier = 1.0);
}
