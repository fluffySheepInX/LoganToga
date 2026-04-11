# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
   void ResolveSapperSpacingAgainstUnits(Array<SpawnedSapper>& spawnedSappers, const Array<SpawnedSapper>& enemySappers, const ModelHeightSettings& modelHeightSettings)
	{
		for (auto& sapper : spawnedSappers)
		{
            if (not IsSpawnedSapperCombatActive(sapper))
			{
				continue;
			}

			const Vec3 sapperPosition = GetSpawnedSapperBasePosition(sapper);
			double nearestDistanceSq = Math::Inf;
			Optional<Vec3> stopPosition;
			Optional<Vec3> targetPosition;

			for (const auto& enemySapper : enemySappers)
			{
               if (not IsSpawnedSapperCombatActive(enemySapper))
				{
					continue;
				}

				const Vec3 enemyPosition = GetSpawnedSapperBasePosition(enemySapper);
                const double surfaceDistance = SapperInternal::GetSapperCombatSurfaceDistance(sapper, enemySapper, modelHeightSettings);

               if ((sapper.attackRange < surfaceDistance) || (surfaceDistance >= nearestDistanceSq))
				{
					continue;
				}

               const double facingYaw = SapperInternal::ToSapperYaw((enemyPosition - sapperPosition), sapper.facingYaw);
				SpawnedSapper orientedSapper = sapper;
				orientedSapper.facingYaw = facingYaw;
				nearestDistanceSq = surfaceDistance;
                stopPosition = SapperInternal::GetSapperCombatStopPosition(orientedSapper, enemySapper, modelHeightSettings);
				targetPosition = enemyPosition;
			}

			if (stopPosition && targetPosition)
			{
				sapper.facingYaw = SapperInternal::ToSapperYaw((*targetPosition - sapperPosition), sapper.facingYaw);
				SapperInternal::StopSapperAtPosition(sapper, *stopPosition);
			}
		}
	}

   void ResolveSapperSpacingAgainstBase(Array<SpawnedSapper>& spawnedSappers, const Vec3& enemyBasePosition, const ModelHeightSettings& modelHeightSettings)
	{
		for (auto& sapper : spawnedSappers)
		{
            if (not IsSpawnedSapperCombatActive(sapper))
			{
				continue;
			}

			if ((sapper.team == UnitTeam::Enemy) && (sapper.aiRole != UnitAiRole::AssaultBase))
			{
				continue;
			}

			const Vec3 sapperPosition = GetSpawnedSapperBasePosition(sapper);
         const double surfaceDistance = SapperInternal::GetSapperBaseCombatSurfaceDistance(sapper, enemyBasePosition, BaseCombatRadius, modelHeightSettings);

          if (sapper.attackRange < surfaceDistance)
			{
				continue;
			}

           const double facingYaw = SapperInternal::ToSapperYaw((enemyBasePosition - sapperPosition), sapper.facingYaw);
			SpawnedSapper orientedSapper = sapper;
			orientedSapper.facingYaw = facingYaw;
			sapper.facingYaw = facingYaw;
         SapperInternal::StopSapperAtPosition(sapper, SapperInternal::GetSapperBaseCombatStopPosition(orientedSapper, enemyBasePosition, BaseCombatRadius, modelHeightSettings));
		}
	}

 void UpdateAutoCombat(Array<SpawnedSapper>& attackers, Array<SpawnedSapper>& defenders, const ModelHeightSettings& modelHeightSettings)
	{
		for (auto& attacker : attackers)
		{
          if (not IsSpawnedSapperCombatActive(attacker))
			{
				continue;
			}

         const Optional<size_t> targetIndex = SapperInternal::FindNearestSapperInRange(attacker, defenders, modelHeightSettings);

			if (not targetIndex)
			{
				continue;
			}

			SpawnedSapper& target = defenders[*targetIndex];
			attacker.facingYaw = SapperInternal::ToSapperYaw((GetSpawnedSapperBasePosition(target) - GetSpawnedSapperBasePosition(attacker)), attacker.facingYaw);

			if ((Scene::Time() - attacker.lastAttackAt) < GetEffectiveSapperAttackInterval(attacker))
			{
				continue;
			}

			attacker.lastAttackAt = Scene::Time();
			target.hitPoints = Max(0.0, (target.hitPoints - GetEffectiveSapperAttackDamage(attacker)));
		}
	}

	void RemoveDefeatedSappers(Array<SpawnedSapper>& spawnedSappers)
	{
		spawnedSappers.remove_if([](const SpawnedSapper& sapper)
			{
				return (sapper.hitPoints <= 0.0);
			});
	}
}
