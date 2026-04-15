# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		[[nodiscard]] double GetSapperMoveProgressT(const SpawnedSapper& sapper, const double linearT)
		{
			switch (sapper.movementType)
			{
			case MovementType::Tank:
             return EaseOutCubic(linearT);

			case MovementType::Infantry:
			default:
				return linearT;
			}
		}
	}

	bool IsSapperSuppressed(const SpawnedSapper& sapper)
	{
		return (Scene::Time() < sapper.suppressedUntil);
	}

	void ApplyUnitParameters(SpawnedSapper& sapper, const UnitParameters& parameters)
	{
      sapper.aiRole = parameters.aiRole;
		sapper.movementType = parameters.movementType;
		sapper.maxHitPoints = Max(1.0, parameters.maxHitPoints);
		sapper.hitPoints = sapper.maxHitPoints;
		sapper.moveSpeed = Max(SapperInternal::MinimumSapperMoveSpeed, parameters.moveSpeed);
		sapper.attackRange = Max(0.5, parameters.attackRange);
		sapper.stopDistance = Clamp(parameters.stopDistance, 0.0, 24.0);
		sapper.baseAttackDamage = Max(0.0, parameters.attackDamage);
		sapper.baseAttackInterval = Max(0.05, parameters.attackInterval);
        sapper.visionRange = Clamp(parameters.visionRange, 0.5, 40.0);
		sapper.footprintType = parameters.footprintType;
		sapper.footprintRadius = Clamp(parameters.footprintRadius, 0.1, 4.0);
		sapper.footprintHalfLength = ((parameters.footprintType == UnitFootprintType::Capsule) ? Clamp(parameters.footprintHalfLength, 0.0, 6.0) : 0.0);
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
		ColorF tint = ApplyUnitTint(sapper.unitType, baseColor);

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
		const double moveT = GetSapperMoveProgressT(sapper, Min(elapsed / sapper.moveDuration, 1.0));
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
}
