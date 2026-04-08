# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	void ResolveSapperSpacingAgainstUnits(Array<SpawnedSapper>& spawnedSappers, const Array<SpawnedSapper>& enemySappers)
	{
		for (auto& sapper : spawnedSappers)
		{
			if (sapper.hitPoints <= 0.0)
			{
				continue;
			}

			const Vec3 sapperPosition = GetSpawnedSapperBasePosition(sapper);
			double nearestDistanceSq = Math::Inf;
			Optional<Vec3> stopPosition;
			Optional<Vec3> targetPosition;

			for (const auto& enemySapper : enemySappers)
			{
				if (enemySapper.hitPoints <= 0.0)
				{
					continue;
				}

				const Vec3 enemyPosition = GetSpawnedSapperBasePosition(enemySapper);
				const double distanceSq = sapperPosition.distanceFromSq(enemyPosition);
				const double stopDistance = SapperInternal::GetSapperCombatStopDistance(sapper, enemySapper);

				if ((Square(stopDistance) < distanceSq) || (distanceSq >= nearestDistanceSq))
				{
					continue;
				}

				const Vec3 separationDirection = SapperInternal::ToHorizontalDirectionOrFallback((sapperPosition - enemyPosition), (sapperPosition - sapper.targetPosition));
				nearestDistanceSq = distanceSq;
				stopPosition = (enemyPosition + separationDirection * stopDistance);
				targetPosition = enemyPosition;
			}

			if (stopPosition && targetPosition)
			{
				sapper.facingYaw = SapperInternal::ToSapperYaw((*targetPosition - sapperPosition), sapper.facingYaw);
				SapperInternal::StopSapperAtPosition(sapper, *stopPosition);
			}
		}
	}

	void ResolveSapperSpacingAgainstBase(Array<SpawnedSapper>& spawnedSappers, const Vec3& enemyBasePosition)
	{
		for (auto& sapper : spawnedSappers)
		{
			if (sapper.hitPoints <= 0.0)
			{
				continue;
			}

			const Vec3 sapperPosition = GetSpawnedSapperBasePosition(sapper);
			const double stopDistance = (sapper.attackRange + BaseCombatRadius);
			const double distanceSq = sapperPosition.distanceFromSq(enemyBasePosition);

			if (Square(stopDistance) < distanceSq)
			{
				continue;
			}

			const Vec3 separationDirection = SapperInternal::ToHorizontalDirectionOrFallback((sapperPosition - enemyBasePosition), (sapperPosition - sapper.targetPosition));
			sapper.facingYaw = SapperInternal::ToSapperYaw((enemyBasePosition - sapperPosition), sapper.facingYaw);
			SapperInternal::StopSapperAtPosition(sapper, (enemyBasePosition + separationDirection * stopDistance));
		}
	}

	void UpdateAutoCombat(Array<SpawnedSapper>& attackers, Array<SpawnedSapper>& defenders)
	{
		for (auto& attacker : attackers)
		{
			if (attacker.hitPoints <= 0.0)
			{
				continue;
			}

			const Optional<size_t> targetIndex = SapperInternal::FindNearestSapperInRange(attacker, defenders);

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
