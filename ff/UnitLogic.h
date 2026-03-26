# pragma once
# include "GameConstants.h"

namespace ff
{
	Optional<Vec2> FindClosestEnemyPos(const Array<Enemy>& enemies, const Vec2& fromPos);
     bool SpawnEnemy(Array<Enemy>& enemies, const Array<Point>& spawnTiles);
	void UpdateEnemies(Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos);
	void UpdateAllies(Array<Ally>& allies, const Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos);
 bool SpawnAlly(Array<Ally>& allies, const TerrainGrid& terrain, const Vec2& playerPos, AllyBehavior behavior);
 int32 UpdateAutoCombat(Array<Ally>& allies, Array<Enemy>& enemies, const Vec2& playerPos, double& playerHp, Array<Vec2>* defeatedEnemyPositions = nullptr, bool canDamagePlayer = true, bool* playerWasHit = nullptr);
	Vec2 GetMovementInput();
	void UpdatePlayerPosition(Vec2& playerPos, const TerrainGrid& terrain);
}
