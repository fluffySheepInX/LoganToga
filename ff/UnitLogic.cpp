# include "UnitLogic.h"
# include "IsoMap.h"
# include <array>

namespace ff
{
	namespace
	{
           void UpdateCooldown(double& cooldown)
			{
				cooldown = Max(0.0, (cooldown - Scene::DeltaTime()));
			}

		void MoveUnitTowards(Vec2& pos, const Vec2& targetPos, const TerrainGrid& terrain, const double speed, const double stopDistance)
		{
			const Vec2 direction = (targetPos - pos);
			const double distance = direction.length();

			if (distance <= stopDistance)
			{
				return;
			}

			const Vec2 movement = (direction / distance) * speed * Scene::DeltaTime();
			const Vec2 nextPos = (pos + movement);

			if (not IsWaterTile(terrain[ToTileIndex(nextPos)]))
			{
				pos = nextPos;
			}
		}

			Optional<size_t> FindClosestEnemyIndexInRange(const Array<Enemy>& enemies, const Vec2& fromPos, const double range)
			{
				Optional<size_t> result;
				double bestDistance = range;

				for (size_t i = 0; i < enemies.size(); ++i)
				{
					const double distance = fromPos.distanceFrom(enemies[i].pos);

					if (distance <= bestDistance)
					{
						bestDistance = distance;
						result = i;
					}
				}

				return result;
			}

			Optional<size_t> FindClosestAllyIndexInRange(const Array<Ally>& allies, const Vec2& fromPos, const double range)
			{
				Optional<size_t> result;
				double bestDistance = range;

				for (size_t i = 0; i < allies.size(); ++i)
				{
					const double distance = fromPos.distanceFrom(allies[i].pos);

					if (distance <= bestDistance)
					{
						bestDistance = distance;
						result = i;
					}
				}

				return result;
			}
	}

	Optional<Vec2> FindClosestEnemyPos(const Array<Enemy>& enemies, const Vec2& fromPos)
	{
		if (enemies.isEmpty())
		{
			return none;
		}

		const Enemy* closest = &enemies.front();
		double closestDistance = fromPos.distanceFrom(closest->pos);

		for (const auto& enemy : enemies)
		{
			const double distance = fromPos.distanceFrom(enemy.pos);

			if (distance < closestDistance)
			{
				closest = &enemy;
				closestDistance = distance;
			}
		}

		return closest->pos;
	}

 int32 UpdateAutoCombat(Array<Ally>& allies, Array<Enemy>& enemies, const Vec2& playerPos, double& playerHp, const double allyAttackIntervalMultiplier, Array<Enemy>* defeatedEnemies, const bool canDamagePlayer, bool* playerWasHit, CombatTelemetry* combatTelemetry)
	{
		Array<double> enemyDamage(enemies.size(), 0.0);
		Array<double> allyDamage(allies.size(), 0.0);
		bool playerCanTakeDamage = canDamagePlayer;

		if (combatTelemetry)
		{
			combatTelemetry->allyDamageDealt.assign(allies.size(), 0.0);
			combatTelemetry->allySurvived.assign(allies.size(), false);
		}

		if (playerWasHit)
		{
			*playerWasHit = false;
		}

		for (auto& ally : allies)
		{
			UpdateCooldown(ally.attackCooldown);
		}

		for (auto& enemy : enemies)
		{
			UpdateCooldown(enemy.attackCooldown);
		}

		for (size_t i = 0; i < allies.size(); ++i)
		{
			auto& ally = allies[i];

			if (ally.attackCooldown > 0.0)
			{
				continue;
			}

          const bool commandBuffActive = IsWithinPlayerCommandRange(ally.pos, playerPos);
			const double allyDamage = GetAllyAttackDamage(ally.behavior) * (commandBuffActive ? PlayerCommandDamageMultiplier : 1.0);
            const double allyAttackInterval = GetAllyAttackInterval(ally.behavior)
				* (commandBuffActive ? PlayerCommandAttackIntervalMultiplier : 1.0)
				* allyAttackIntervalMultiplier;

			  if (const auto targetIndex = FindClosestEnemyIndexInRange(enemies, ally.pos, GetAllyAttackRange(ally.behavior)))
			{
                enemyDamage[*targetIndex] += allyDamage;
               if (combatTelemetry)
				{
					combatTelemetry->allyDamageDealt[i] += allyDamage;
				}
				ally.attackCooldown = allyAttackInterval;
			}
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			auto& enemy = enemies[i];

			if (enemy.attackCooldown > 0.0)
			{
				continue;
			}

          if (const auto targetIndex = FindClosestAllyIndexInRange(allies, enemy.pos, GetEnemyAttackRange(enemy.kind)))
			{
              allyDamage[*targetIndex] += GetEnemyAttackDamage(enemy.kind);
				enemy.attackCooldown = GetEnemyAttackInterval(enemy.kind);
				continue;
			}

            if (enemy.pos.distanceFrom(playerPos) <= GetEnemyAttackRange(enemy.kind))
			{
                if (playerCanTakeDamage)
				{
                    playerHp = Max(0.0, (playerHp - GetEnemyAttackDamage(enemy.kind)));
					playerCanTakeDamage = false;

					if (playerWasHit)
					{
						*playerWasHit = true;
					}
				}

             enemy.attackCooldown = GetEnemyAttackInterval(enemy.kind);
			}
		}

		for (size_t i = 0; i < enemies.size(); ++i)
		{
			enemies[i].hp -= enemyDamage[i];
		}

		for (size_t i = 0; i < allies.size(); ++i)
		{
			allies[i].hp -= allyDamage[i];
		}

      Array<Enemy> survivingEnemies;
		survivingEnemies.reserve(enemies.size());

		for (const auto& enemy : enemies)
			{
                 if (enemy.hp > 0.0)
				{
					survivingEnemies << enemy;
				}
             else if (defeatedEnemies)
				{
                   defeatedEnemies->push_back(enemy);
				}
			}

       Array<Ally> survivingAllies;
		survivingAllies.reserve(allies.size());

     for (size_t i = 0; i < allies.size(); ++i)
			{
              const auto& ally = allies[i];
				const bool survived = (ally.hp > 0.0);

				if (combatTelemetry)
				{
					combatTelemetry->allySurvived[i] = survived;
				}

                if (ally.hp > 0.0)
				{
					survivingAllies << ally;
				}
			}

      const int32 defeatedEnemyCount = static_cast<int32>(enemies.size() - survivingEnemies.size());
        enemies = std::move(survivingEnemies);
		allies = std::move(survivingAllies);
       return defeatedEnemyCount;
	}

    bool SpawnEnemy(Array<Enemy>& enemies, const Array<Point>& spawnTiles, const EnemyKind kind, const double hpMultiplier, const double speedMultiplier, const double attackIntervalMultiplier)
	{
		if (spawnTiles.isEmpty() || (enemies.size() >= MaxEnemyCount))
		{
          return false;
		}

		const Point spawnTile = spawnTiles[Random<size_t>(spawnTiles.size() - 1)];
     const double maxHp = (GetEnemyMaxHp(kind) * Max(0.1, hpMultiplier));
		enemies << Enemy{
			Vec2{ static_cast<double>(spawnTile.x), static_cast<double>(spawnTile.y) },
			kind,
			maxHp,
			maxHp,
			Max(0.1, speedMultiplier),
			Max(0.1, attackIntervalMultiplier)
		};
		return true;
	}

	void UpdateEnemies(Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos)
	{
		for (auto& enemy : enemies)
		{
         MoveUnitTowards(enemy.pos, playerPos, terrain, (GetEnemySpeed(enemy.kind) * enemy.speedMultiplier), EnemyStopDistance);
		}
	}

  void UpdateAllies(Array<Ally>& allies, const Array<Enemy>& enemies, const TerrainGrid& terrain, const Vec2& playerPos, const double speedMultiplier)
	{
		for (auto& ally : allies)
		{
			switch (ally.behavior)
			{
			case AllyBehavior::ChaseEnemies:
				if (const auto target = FindClosestEnemyPos(enemies, ally.pos))
				{
                   MoveUnitTowards(ally.pos, *target, terrain, (AllySpeed * speedMultiplier), AllyStopDistance);
				}
				break;

			case AllyBehavior::HoldPosition:
				break;

			case AllyBehavior::GuardPlayer:
              MoveUnitTowards(ally.pos, playerPos, terrain, (AllySpeed * speedMultiplier), 1.1);
				break;

			case AllyBehavior::OrbitPlayer:
				ally.orbitAngle += (1.6 * Scene::DeltaTime());
				MoveUnitTowards(ally.pos,
					(playerPos + Circular{ AllyOrbitRadius, ally.orbitAngle }.fastToVec2()),
					terrain,
                  (AllySpeed * speedMultiplier),
					0.25);
				break;

			case AllyBehavior::FixedTurret:
				break;
			}
		}
	}

   bool SpawnAlly(Array<Ally>& allies, const TerrainGrid& terrain, const Vec2& playerPos, const AllyBehavior behavior)
	{
		static constexpr std::array<Point, 9> SpawnOffsets = {
			Point{ -1, 0 },
			Point{ 1, 0 },
			Point{ 0, -1 },
			Point{ 0, 1 },
			Point{ -1, -1 },
			Point{ 1, -1 },
			Point{ -1, 1 },
			Point{ 1, 1 },
			Point{ 0, 0 },
		};

		const Point playerTile = ToTileIndex(playerPos);

		for (const auto& offset : SpawnOffsets)
		{
			const Point tile{ (playerTile.x + offset.x), (playerTile.y + offset.y) };

			if ((not InRange(tile.x, 0, (MapSize.x - 1)))
				|| (not InRange(tile.y, 0, (MapSize.y - 1)))
				|| IsWaterTile(terrain[tile]))
			{
				continue;
			}

			bool occupied = false;

			for (const auto& ally : allies)
			{
				if (ToTileIndex(ally.pos) == tile)
				{
					occupied = true;
					break;
				}
			}

			if (occupied)
			{
				continue;
			}

         allies << Ally{ Vec2{ static_cast<double>(tile.x), static_cast<double>(tile.y) }, behavior, Random(0.0, Math::TwoPi), GetAllyMaxHp(behavior) };
         return true;
		}

		return false;
	}

	Vec2 GetMovementInput()
	{
		Vec2 movement{ 0, 0 };

		if (KeyW.pressed())
		{
			movement += Vec2{ -1, -1 };
		}

		if (KeyS.pressed())
		{
			movement += Vec2{ 1, 1 };
		}

		if (KeyA.pressed())
		{
			movement += Vec2{ -1, 1 };
		}

		if (KeyD.pressed())
		{
			movement += Vec2{ 1, -1 };
		}

		return movement;
	}

  void UpdatePlayerPosition(Vec2& playerPos, const TerrainGrid& terrain, const double speedMultiplier)
	{
		Vec2 movement = GetMovementInput();

		if (movement == Vec2{ 0, 0 })
		{
			return;
		}

        movement = movement.normalized() * (PlayerSpeed * speedMultiplier) * Scene::DeltaTime();
		Vec2 nextPos = (playerPos + movement);
		nextPos.x = Clamp(nextPos.x, 1.0, (MapSize.x - 2.0));
		nextPos.y = Clamp(nextPos.y, 1.0, (MapSize.y - 2.0));

		if (not IsWaterTile(terrain[ToTileIndex(nextPos)]))
		{
			playerPos = nextPos;
		}
	}
}
