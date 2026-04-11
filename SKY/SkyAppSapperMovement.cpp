# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	void ApplySapperSuppression(SpawnedSapper& sapper, const double durationSeconds, const double moveSpeedMultiplier, const double attackDamageMultiplier, const double attackIntervalMultiplier)
	{
		const Vec3 currentPosition = GetSpawnedSapperBasePosition(sapper);
		const double remainingDistance = currentPosition.distanceFrom(sapper.targetPosition);
		sapper.suppressedUntil = Max(sapper.suppressedUntil, (Scene::Time() + Max(0.0, durationSeconds)));
		sapper.suppressedMoveSpeedMultiplier = Clamp(moveSpeedMultiplier, SapperInternal::MinimumSuppressionMultiplier, 1.0);
		sapper.suppressedAttackDamageMultiplier = Clamp(attackDamageMultiplier, SapperInternal::MinimumSuppressionMultiplier, 1.0);
		sapper.suppressedAttackIntervalMultiplier = Clamp(attackIntervalMultiplier, 1.0, SapperInternal::MaximumSuppressionAttackIntervalMultiplier);

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

		sapper.moveDuration = Max(SapperInternal::MinimumSapperMoveDuration, (remainingDistance / GetEffectiveSapperMoveSpeed(sapper)));
	}

 void UpdateSpawnedSappers(Array<SpawnedSapper>& spawnedSappers, const MapData& mapData, const ModelHeightSettings& modelHeightSettings)
	{
		for (auto& sapper : spawnedSappers)
		{
         if (IsSapperRetreatOrdered(sapper))
			{
				if (sapper.retreatReturnAt <= Scene::Time())
				{
					sapper.position = sapper.retreatReturnPosition;
					sapper.startPosition = sapper.retreatReturnPosition;
					sapper.targetPosition = sapper.retreatReturnPosition;
					sapper.destinationPosition = sapper.retreatReturnPosition;
					sapper.moveStartedAt = Scene::Time();
					sapper.moveDuration = 0.0;
					sapper.hitPoints = sapper.maxHitPoints;
					sapper.suppressedUntil = -1000.0;
					sapper.retreatDisappearAt = -1000.0;
					sapper.retreatReturnAt = -1000.0;
				}

				continue;
			}

			if (sapper.moveDuration <= 0.0)
			{
				continue;
			}

			sapper.position = GetSpawnedSapperBasePosition(sapper);
			sapper.facingYaw = SapperInternal::ToSapperYaw((sapper.targetPosition - sapper.position), sapper.facingYaw);

			if (sapper.moveDuration <= (Scene::Time() - sapper.moveStartedAt))
			{
				sapper.position = sapper.targetPosition;
				sapper.startPosition = sapper.targetPosition;
				sapper.moveDuration = 0.0;

				if (0.05 < sapper.position.distanceFrom(sapper.destinationPosition))
				{
                    SetSpawnedSapperTarget(sapper, sapper.destinationPosition, mapData, modelHeightSettings);
				}
			}
		}
	}
}
