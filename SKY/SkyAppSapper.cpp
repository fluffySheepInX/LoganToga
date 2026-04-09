# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace SapperInternal
	{
       [[nodiscard]] double GetSapperMoveProgressT(const SpawnedSapper& sapper, const double linearT)
		{
			switch (sapper.movementType)
			{
			case MovementType::Tank:
				return EaseInOutCubic(linearT);

			case MovementType::Infantry:
			default:
				return linearT;
			}
		}

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

		[[nodiscard]] Optional<double> GetPlacedModelObstacleRadius(const PlacedModel& placedModel)
		{
			switch (placedModel.type)
			{
			case PlaceableModelType::Rock:
				return RockObstacleRadius;

			default:
				return none;
			}
		}

		void StopSapperAtPosition(SpawnedSapper& sapper, const Vec3& position)
		{
			const Vec3 groundedPosition{ position.x, 0.0, position.z };
			sapper.position = groundedPosition;
			sapper.startPosition = groundedPosition;
			sapper.targetPosition = groundedPosition;
           sapper.destinationPosition = groundedPosition;
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

	void ApplyUnitParameters(SpawnedSapper& sapper, const UnitParameters& parameters)
	{
		sapper.movementType = parameters.movementType;
		sapper.maxHitPoints = Max(1.0, parameters.maxHitPoints);
		sapper.hitPoints = sapper.maxHitPoints;
		sapper.moveSpeed = Max(SapperInternal::MinimumSapperMoveSpeed, parameters.moveSpeed);
		sapper.attackRange = Max(0.5, parameters.attackRange);
		sapper.baseAttackDamage = Max(0.0, parameters.attackDamage);
		sapper.baseAttackInterval = Max(0.05, parameters.attackInterval);
	}

	double GetEffectiveSapperMoveSpeed(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedMoveSpeedMultiplier, SapperInternal::MinimumSuppressionMultiplier, 1.0)
			: 1.0);
		return Max(SapperInternal::MinimumSapperMoveSpeed, (sapper.moveSpeed * multiplier));
	}

	double GetEffectiveSapperAttackDamage(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedAttackDamageMultiplier, SapperInternal::MinimumSuppressionMultiplier, 1.0)
			: 1.0);
		return Max(0.0, (sapper.baseAttackDamage * multiplier));
	}

	double GetEffectiveSapperAttackInterval(const SpawnedSapper& sapper)
	{
		const double multiplier = (IsSapperSuppressed(sapper)
			? Clamp(sapper.suppressedAttackIntervalMultiplier, 1.0, SapperInternal::MaximumSuppressionAttackIntervalMultiplier)
			: 1.0);
		return Max(0.05, (sapper.baseAttackInterval * multiplier));
	}

	ColorF GetSpawnedSapperTint(const SpawnedSapper& sapper, const ColorF& baseColor)
	{
		ColorF tint = baseColor;

		switch (sapper.unitType)
		{
		case SapperUnitType::ArcaneInfantry:
			tint = ColorF{
				Clamp(baseColor.r * 0.62 + 0.18, 0.0, 1.0),
				Clamp(baseColor.g * 0.82 + 0.10, 0.0, 1.0),
				Clamp(baseColor.b * 1.30 + 0.18, 0.0, 1.0),
				baseColor.a,
			};
			break;

		case SapperUnitType::Infantry:
		default:
			break;
		}

		if (IsSapperSuppressed(sapper))
		{
			const double pulse = (0.35 + 0.25 * Periodic::Sine0_1(1.2s));
			tint = tint.lerp(ColorF{ 0.52, 0.84, 1.0, tint.a }, pulse);
		}

		return tint;
	}

	Vec3 GetSpawnedSapperBasePosition(const SpawnedSapper& sapper)
	{
		if (sapper.moveDuration <= 0.0)
		{
			return sapper.position;
		}

		const double elapsed = (Scene::Time() - sapper.moveStartedAt);
       const double moveT = SapperInternal::GetSapperMoveProgressT(sapper, Min(elapsed / sapper.moveDuration, 1.0));
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

    void SpawnSapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, const MapData& mapData, const SapperUnitType unitType)
	{
		const size_t sapperIndex = spawnedSappers.size();
		const Vec3 startPosition = spawnPosition.movedBy(2.4, 0, 2.2);
		const Vec3 targetPosition = GetSapperPopTargetPosition(rallyPoint, sapperIndex);
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = startPosition,
			.position = startPosition,
           .targetPosition = startPosition,
			.destinationPosition = startPosition,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
          .moveDuration = 0.0,
			.facingYaw = SapperInternal::ToSapperYaw((targetPosition - startPosition), BirdDisplayYaw),
			.team = UnitTeam::Player,
			.unitType = unitType,
           .movementType = MovementType::Infantry,
		};
       ApplyUnitParameters(spawnedSappers.back(), MakeDefaultUnitParameters(UnitTeam::Player, unitType));
       SetSpawnedSapperTarget(spawnedSappers.back(), targetPosition, mapData);
	}

   void SpawnEnemySapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& position, const double facingYaw, const SapperUnitType unitType)
	{
		const double spawnTime = Scene::Time();
		spawnedSappers << SpawnedSapper{
			.startPosition = position,
			.position = position,
			.targetPosition = position,
         .destinationPosition = position,
			.spawnedAt = spawnTime,
			.moveStartedAt = spawnTime,
			.moveDuration = 0.0,
			.facingYaw = facingYaw,
			.team = UnitTeam::Enemy,
           .unitType = unitType,
           .movementType = MovementType::Infantry,
		};
      ApplyUnitParameters(spawnedSappers.back(), MakeDefaultUnitParameters(UnitTeam::Enemy, unitType));
	}
}
