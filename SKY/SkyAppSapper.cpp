# include "SkyAppSupport.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		constexpr double InitialSapperMoveDuration = 0.45;
     constexpr double DefaultSapperMoveSpeed = 6.0;
		constexpr double MinimumSapperMoveSpeed = 0.5;
		constexpr double MinimumSuppressionMultiplier = 0.1;
		constexpr double MaximumSuppressionAttackIntervalMultiplier = 10.0;
		constexpr double MinimumSapperMoveDuration = 0.08;
		constexpr double MinimumSapperTurnDistanceSq = 0.0001;
		constexpr double SapperBodyRadius = 1.2;

		[[nodiscard]] double ToSapperYaw(const Vec3& direction, const double fallbackYaw)
		{
			const Vec2 horizontalDirection{ direction.x, direction.z };

			if (horizontalDirection.lengthSq() <= MinimumSapperTurnDistanceSq)
			{
				return fallbackYaw;
			}

			return (BirdDisplayYaw + SapperFacingYawOffset + Math::Atan2(direction.x, direction.z));
		}

		[[nodiscard]] Vec3 ToHorizontalDirectionOrFallback(const Vec3& direction, const Vec3& fallback)
		{
			Vec3 horizontalDirection{ direction.x, 0.0, direction.z };

			if (horizontalDirection.lengthSq() <= MinimumSapperTurnDistanceSq)
			{
				horizontalDirection = Vec3{ fallback.x, 0.0, fallback.z };
			}

			if (horizontalDirection.lengthSq() <= MinimumSapperTurnDistanceSq)
			{
				return Vec3{ 0, 0, -1 };
			}

			return horizontalDirection.normalized();
		}

		[[nodiscard]] double GetSapperCombatStopDistance(const SpawnedSapper& attacker, const SpawnedSapper& defender)
		{
			return (attacker.attackRange + SapperBodyRadius + SapperBodyRadius);
		}

		void StopSapperAtPosition(SpawnedSapper& sapper, const Vec3& position)
		{
			const Vec3 groundedPosition{ position.x, 0.0, position.z };
			sapper.position = groundedPosition;
			sapper.startPosition = groundedPosition;
			sapper.targetPosition = groundedPosition;
			sapper.moveStartedAt = Scene::Time();
			sapper.moveDuration = 0.0;
		}

		[[nodiscard]] Optional<size_t> FindNearestSapperInRange(const SpawnedSapper& source, const Array<SpawnedSapper>& candidates)
		{
			double nearestDistanceSq = Math::Inf;
			Optional<size_t> nearestIndex;
			const Vec3 sourcePosition = GetSpawnedSapperBasePosition(source);

			for (size_t i = 0; i < candidates.size(); ++i)
			{
				if (candidates[i].hitPoints <= 0.0)
				{
					continue;
				}

				const double distanceSq = sourcePosition.distanceFromSq(GetSpawnedSapperBasePosition(candidates[i]));
				const double combatStopDistance = GetSapperCombatStopDistance(source, candidates[i]);

				if ((distanceSq <= Square(combatStopDistance)) && (distanceSq < nearestDistanceSq))
				{
					nearestDistanceSq = distanceSq;
					nearestIndex = i;
				}
			}

			return nearestIndex;
		}
	}

	bool IsSapperSuppressed(const SpawnedSapper& sapper)
	{
		return (Scene::Time() < sapper.suppressedUntil);
	}

	double GetEffectiveSapperMoveSpeed(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedMoveSpeedMultiplier, MinimumSuppressionMultiplier, 1.0)
			: 1.0);
		return Max(MinimumSapperMoveSpeed, (sapper.moveSpeed * multiplier));
	}

	double GetEffectiveSapperAttackDamage(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedAttackDamageMultiplier, MinimumSuppressionMultiplier, 1.0)
			: 1.0);
		return Max(0.0, (sapper.baseAttackDamage * multiplier));
	}

	double GetEffectiveSapperAttackInterval(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedAttackIntervalMultiplier, 1.0, MaximumSuppressionAttackIntervalMultiplier)
			: 1.0);
		return Max(0.05, (sapper.baseAttackInterval * multiplier));
	}

		ColorF GetSpawnedSapperTint(const SpawnedSapper& sapper, const ColorF& baseColor)
		{
			switch (sapper.unitType)
			{
			case SapperUnitType::ArcaneInfantry:
				return ColorF{
					Clamp(baseColor.r * 0.62 + 0.18, 0.0, 1.0),
					Clamp(baseColor.g * 0.82 + 0.10, 0.0, 1.0),
					Clamp(baseColor.b * 1.30 + 0.18, 0.0, 1.0),
					baseColor.a,
				};

			case SapperUnitType::Infantry:
			default:
				return baseColor;
			}
		}

	void ApplySapperSuppression(SpawnedSapper& sapper, const double durationSeconds, const double moveSpeedMultiplier, const double attackDamageMultiplier, const double attackIntervalMultiplier)
	{
      const Vec3 currentPosition = GetSpawnedSapperBasePosition(sapper);
		const double remainingDistance = currentPosition.distanceFrom(sapper.targetPosition);
		sapper.suppressedUntil = Max(sapper.suppressedUntil, (Scene::Time() + Max(0.0, durationSeconds)));
		sapper.suppressedMoveSpeedMultiplier = Clamp(moveSpeedMultiplier, MinimumSuppressionMultiplier, 1.0);
		sapper.suppressedAttackDamageMultiplier = Clamp(attackDamageMultiplier, MinimumSuppressionMultiplier, 1.0);
		sapper.suppressedAttackIntervalMultiplier = Clamp(attackIntervalMultiplier, 1.0, MaximumSuppressionAttackIntervalMultiplier);

		if (sapper.moveDuration <= 0.0)
		{
			return;
		}

		sapper.position = currentPosition;
		sapper.startPosition = currentPosition;
		sapper.moveStartedAt = Scene::Time();

		if (remainingDistance <= 0.05)
		{
			sapper.position = sapper.targetPosition;
			sapper.startPosition = sapper.targetPosition;
			sapper.moveDuration = 0.0;
			return;
		}

		sapper.moveDuration = Max(MinimumSapperMoveDuration, (remainingDistance / GetEffectiveSapperMoveSpeed(sapper)));
	}

	Vec3 GetSpawnedSapperBasePosition(const SpawnedSapper& sapper)
	{
		if (sapper.moveDuration <= 0.0)
		{
			return sapper.position;
		}

		const double elapsed = (Scene::Time() - sapper.moveStartedAt);
		const double moveT = EaseInOutCubic(Min(elapsed / sapper.moveDuration, 1.0));
		return sapper.startPosition.lerp(sapper.targetPosition, moveT);
	}

	double GetSpawnedSapperBounceOffset(const SpawnedSapper& sapper)
	{
		const double elapsed = (Scene::Time() - sapper.spawnedAt);
		return (0.18 * Max(0.0, 1.0 - elapsed / 0.5) * Abs(Math::Sin(elapsed * 18.0)));
	}

	Vec3 GetSpawnedSapperRenderPosition(const SpawnedSapper& sapper)
	{
		return GetSpawnedSapperBasePosition(sapper).movedBy(0, GetSpawnedSapperBounceOffset(sapper), 0);
	}

	double GetSpawnedSapperYaw(const SpawnedSapper& sapper)
	{
		return sapper.facingYaw;
	}

	void UpdateSpawnedSappers(Array<SpawnedSapper>& spawnedSappers)
	{
		for (auto& sapper : spawnedSappers)
		{
			if (sapper.moveDuration <= 0.0)
			{
				continue;
			}

			sapper.position = GetSpawnedSapperBasePosition(sapper);
			sapper.facingYaw = ToSapperYaw((sapper.targetPosition - sapper.position), sapper.facingYaw);

			if (sapper.moveDuration <= (Scene::Time() - sapper.moveStartedAt))
			{
				sapper.position = sapper.targetPosition;
				sapper.startPosition = sapper.targetPosition;
				sapper.moveDuration = 0.0;
			}
		}
	}

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
				const double stopDistance = GetSapperCombatStopDistance(sapper, enemySapper);

				if ((Square(stopDistance) < distanceSq) || (distanceSq >= nearestDistanceSq))
				{
					continue;
				}

				const Vec3 separationDirection = ToHorizontalDirectionOrFallback((sapperPosition - enemyPosition), (sapperPosition - sapper.targetPosition));
				nearestDistanceSq = distanceSq;
				stopPosition = (enemyPosition + separationDirection * stopDistance);
				targetPosition = enemyPosition;
			}

			if (stopPosition && targetPosition)
			{
				sapper.facingYaw = ToSapperYaw((*targetPosition - sapperPosition), sapper.facingYaw);
				StopSapperAtPosition(sapper, *stopPosition);
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

			const Vec3 separationDirection = ToHorizontalDirectionOrFallback((sapperPosition - enemyBasePosition), (sapperPosition - sapper.targetPosition));
			sapper.facingYaw = ToSapperYaw((enemyBasePosition - sapperPosition), sapper.facingYaw);
			StopSapperAtPosition(sapper, (enemyBasePosition + separationDirection * stopDistance));
		}
	}

	void SetSpawnedSapperTarget(SpawnedSapper& sapper, const Vec3& targetPosition)
	{
		const Vec3 currentPosition = GetSpawnedSapperBasePosition(sapper);
		const Vec3 groundedTargetPosition{ targetPosition.x, 0.0, targetPosition.z };
		const double distance = currentPosition.distanceFrom(groundedTargetPosition);
		sapper.startPosition = currentPosition;
		sapper.position = currentPosition;
		sapper.targetPosition = groundedTargetPosition;
		sapper.facingYaw = ToSapperYaw((groundedTargetPosition - currentPosition), sapper.facingYaw);

		if (distance <= 0.05)
		{
			sapper.position = groundedTargetPosition;
			sapper.startPosition = groundedTargetPosition;
			sapper.moveDuration = 0.0;
			return;
		}

		sapper.moveStartedAt = Scene::Time();
     sapper.moveDuration = Max(MinimumSapperMoveDuration, (distance / GetEffectiveSapperMoveSpeed(sapper)));
	}

	void UpdateAutoCombat(Array<SpawnedSapper>& attackers, Array<SpawnedSapper>& defenders)
	{
		for (auto& attacker : attackers)
		{
			if (attacker.hitPoints <= 0.0)
			{
				continue;
			}

			const Optional<size_t> targetIndex = FindNearestSapperInRange(attacker, defenders);

			if (not targetIndex)
			{
				continue;
			}

			SpawnedSapper& target = defenders[*targetIndex];
			attacker.facingYaw = ToSapperYaw((GetSpawnedSapperBasePosition(target) - GetSpawnedSapperBasePosition(attacker)), attacker.facingYaw);

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

   void SpawnSapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, const SapperUnitType unitType)
	{
		const size_t sapperIndex = spawnedSappers.size();
		const Vec3 startPosition = spawnPosition.movedBy(2.4, 0, 2.2);
		const Vec3 targetPosition = GetSapperPopTargetPosition(rallyPoint, sapperIndex);
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = startPosition,
			.position = startPosition,
			.targetPosition = targetPosition,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
			.moveDuration = InitialSapperMoveDuration,
			.facingYaw = ToSapperYaw((targetPosition - startPosition), BirdDisplayYaw),
			.team = UnitTeam::Player,
          .unitType = unitType,
			.maxHitPoints = 100.0,
			.hitPoints = 100.0,
         .moveSpeed = DefaultSapperMoveSpeed,
			.attackRange = 3.2,
           .baseAttackDamage = 12.0,
			.baseAttackInterval = 0.8,
		};
	}

	void SpawnEnemySapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& position, const double facingYaw)
	{
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = position,
			.position = position,
			.targetPosition = position,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
			.moveDuration = 0.0,
			.facingYaw = facingYaw,
			.team = UnitTeam::Enemy,
          .unitType = SapperUnitType::Infantry,
			.maxHitPoints = 120.0,
			.hitPoints = 120.0,
         .moveSpeed = DefaultSapperMoveSpeed,
			.attackRange = 3.0,
           .baseAttackDamage = 10.0,
			.baseAttackInterval = 0.95,
		};
	}
}
