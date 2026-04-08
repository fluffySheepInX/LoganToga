# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace
{
	[[nodiscard]] Vec2 ToHorizontal(const Vec3& value)
	{
		return Vec2{ value.x, value.z };
	}

	[[nodiscard]] bool IsPathSegmentBlocked(const Vec3& start, const Vec3& goal, const Array<PlacedModel>& placedModels)
	{
		const Vec2 start2 = ToHorizontal(start);
		const Vec2 goal2 = ToHorizontal(goal);
		const Vec2 segment = (goal2 - start2);
		const double segmentLengthSq = segment.lengthSq();

		if (segmentLengthSq <= SkyAppSupport::SapperInternal::MinimumSapperTurnDistanceSq)
		{
			return false;
		}

		for (const auto& placedModel : placedModels)
		{
			const Optional<double> obstacleRadius = SkyAppSupport::SapperInternal::GetPlacedModelObstacleRadius(placedModel);
			if (not obstacleRadius)
			{
				continue;
			}

			const double requiredDistance = (*obstacleRadius + SkyAppSupport::SapperInternal::SapperBodyRadius + SkyAppSupport::SapperInternal::ObstacleAvoidancePadding);
			const Vec2 obstacle2 = ToHorizontal(placedModel.position);
			const double t = Clamp(((obstacle2 - start2).dot(segment) / segmentLengthSq), 0.0, 1.0);
			const Vec2 closestPoint = (start2 + (segment * t));
			if (closestPoint.distanceFromSq(obstacle2) < Square(requiredDistance))
			{
				return true;
			}
		}

		return false;
	}

	[[nodiscard]] Optional<Vec3> FindNextRouteTarget(const Vec3& start, const Vec3& destination, const MapData& mapData)
	{
		const Vec3 groundedDestination{ destination.x, 0.0, destination.z };

		if (not IsPathSegmentBlocked(start, groundedDestination, mapData.placedModels))
		{
			return groundedDestination;
		}

		if (mapData.navPoints.isEmpty())
		{
			return none;
		}

		Array<double> bestDistances(mapData.navPoints.size(), Math::Inf);
		Array<Optional<size_t>> previousIndices(mapData.navPoints.size());
		Array<bool> visited(mapData.navPoints.size(), false);

		for (size_t i = 0; i < mapData.navPoints.size(); ++i)
		{
			if (not IsPathSegmentBlocked(start, mapData.navPoints[i].position, mapData.placedModels))
			{
				bestDistances[i] = start.distanceFrom(mapData.navPoints[i].position);
			}
		}

		for (;;)
		{
			double nearestDistance = Math::Inf;
			Optional<size_t> currentIndex;

			for (size_t i = 0; i < bestDistances.size(); ++i)
			{
				if ((not visited[i]) && (bestDistances[i] < nearestDistance))
				{
					nearestDistance = bestDistances[i];
					currentIndex = i;
				}
			}

			if (not currentIndex)
			{
				break;
			}

			visited[*currentIndex] = true;

			for (const auto& navLink : mapData.navLinks)
			{
				auto relax = [&](const size_t fromIndex, const size_t toIndex)
					{
						if ((*currentIndex != fromIndex)
							|| (mapData.navPoints.size() <= fromIndex)
							|| (mapData.navPoints.size() <= toIndex)
							|| visited[toIndex]
							|| IsPathSegmentBlocked(mapData.navPoints[fromIndex].position, mapData.navPoints[toIndex].position, mapData.placedModels))
						{
							return;
						}

						const double segmentDistance = mapData.navPoints[fromIndex].position.distanceFrom(mapData.navPoints[toIndex].position);
						const double candidateDistance = (bestDistances[fromIndex] + (segmentDistance * Max(0.1, navLink.costMultiplier)));

						if (candidateDistance < bestDistances[toIndex])
						{
							bestDistances[toIndex] = candidateDistance;
							previousIndices[toIndex] = fromIndex;
						}
					};

				relax(navLink.fromIndex, navLink.toIndex);
				if (navLink.bidirectional)
				{
					relax(navLink.toIndex, navLink.fromIndex);
				}
			}
		}

		double bestGoalDistance = Math::Inf;
		Optional<size_t> bestGoalIndex;

		for (size_t i = 0; i < mapData.navPoints.size(); ++i)
		{
			if ((bestDistances[i] == Math::Inf)
				|| IsPathSegmentBlocked(mapData.navPoints[i].position, groundedDestination, mapData.placedModels))
			{
				continue;
			}

			const double candidateDistance = (bestDistances[i] + mapData.navPoints[i].position.distanceFrom(groundedDestination));
			if (candidateDistance < bestGoalDistance)
			{
				bestGoalDistance = candidateDistance;
				bestGoalIndex = i;
			}
		}

		if (not bestGoalIndex)
		{
			return none;
		}

		size_t nextIndex = *bestGoalIndex;
		while (previousIndices[nextIndex])
		{
			nextIndex = *previousIndices[nextIndex];
		}

		return mapData.navPoints[nextIndex].position;
	}

	void StartSapperMoveSegment(SpawnedSapper& sapper, const Vec3& fromPosition, const Vec3& targetPosition)
	{
		const Vec3 groundedTargetPosition{ targetPosition.x, 0.0, targetPosition.z };
		const double distance = fromPosition.distanceFrom(groundedTargetPosition);
		sapper.startPosition = fromPosition;
		sapper.position = fromPosition;
		sapper.targetPosition = groundedTargetPosition;
		sapper.facingYaw = SkyAppSupport::SapperInternal::ToSapperYaw((groundedTargetPosition - fromPosition), sapper.facingYaw);

		if (distance <= 0.05)
		{
			sapper.position = groundedTargetPosition;
			sapper.startPosition = groundedTargetPosition;
			sapper.moveDuration = 0.0;
			return;
		}

		sapper.moveStartedAt = Scene::Time();
		sapper.moveDuration = Max(SkyAppSupport::SapperInternal::MinimumSapperMoveDuration, (distance / SkyAppSupport::GetEffectiveSapperMoveSpeed(sapper)));
	}
}

namespace SkyAppSupport
{
 void ResolveSapperSpacingAgainstObstacles(Array<SpawnedSapper>& spawnedSappers, const MapData& mapData)
	{
		for (auto& sapper : spawnedSappers)
		{
			if (sapper.hitPoints <= 0.0)
			{
				continue;
			}

           const Vec3 desiredTarget = sapper.targetPosition;
			const Vec3 finalDestination = sapper.destinationPosition;
			Vec3 adjustedPosition = GetSpawnedSapperBasePosition(sapper);
			bool adjusted = false;

            for (const auto& placedModel : mapData.placedModels)
			{
				const Optional<double> obstacleRadius = SapperInternal::GetPlacedModelObstacleRadius(placedModel);
				if (not obstacleRadius)
				{
					continue;
				}

				const double requiredDistance = (*obstacleRadius + SapperInternal::SapperBodyRadius + SapperInternal::ObstacleAvoidancePadding);
				const Vec3 obstaclePosition = placedModel.position;
				const Vec3 toObstacle = (obstaclePosition - adjustedPosition);
				const double distanceSq = (toObstacle.x * toObstacle.x + toObstacle.z * toObstacle.z);

				if (distanceSq < Square(requiredDistance))
				{
					const Vec3 pushDirection = SapperInternal::ToHorizontalDirectionOrFallback((adjustedPosition - obstaclePosition), (adjustedPosition - desiredTarget));
					adjustedPosition = obstaclePosition + (pushDirection * requiredDistance);
					adjusted = true;
					continue;
				}

				Vec3 moveDirection = (desiredTarget - adjustedPosition);
				const double moveLengthSq = (moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z);
				if (moveLengthSq <= SapperInternal::MinimumSapperTurnDistanceSq)
				{
					continue;
				}

				moveDirection = Vec3{ moveDirection.x, 0.0, moveDirection.z } / std::sqrt(moveLengthSq);
				const double projectedDistance = ((obstaclePosition.x - adjustedPosition.x) * moveDirection.x)
					+ ((obstaclePosition.z - adjustedPosition.z) * moveDirection.z);

				if ((projectedDistance <= 0.0) || (std::sqrt(moveLengthSq) <= projectedDistance))
				{
					continue;
				}

				const Vec3 closestPoint = adjustedPosition + (moveDirection * projectedDistance);
				const double lateralDistanceSq = ((obstaclePosition.x - closestPoint.x) * (obstaclePosition.x - closestPoint.x))
					+ ((obstaclePosition.z - closestPoint.z) * (obstaclePosition.z - closestPoint.z));

				if (Square(requiredDistance) < lateralDistanceSq)
				{
					continue;
				}

				const double sideSign = (((moveDirection.x * toObstacle.z) - (moveDirection.z * toObstacle.x)) < 0.0) ? 1.0 : -1.0;
				const Vec3 slideDirection{ moveDirection.z * sideSign, 0.0, -moveDirection.x * sideSign };
				const double lateralDistance = std::sqrt(Max(0.0, lateralDistanceSq));
				adjustedPosition += (slideDirection * (requiredDistance - lateralDistance + 0.2));
				adjusted = true;
			}

			if (adjusted)
			{
				sapper.position = Vec3{ adjustedPosition.x, 0.0, adjustedPosition.z };
				sapper.startPosition = sapper.position;
				sapper.moveStartedAt = Scene::Time();
				sapper.moveDuration = 0.0;
              SetSpawnedSapperTarget(sapper, finalDestination, mapData);
			}
		}
	}

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

 void UpdateSpawnedSappers(Array<SpawnedSapper>& spawnedSappers, const MapData& mapData)
	{
		for (auto& sapper : spawnedSappers)
		{
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
					SetSpawnedSapperTarget(sapper, sapper.destinationPosition, mapData);
				}
			}
		}
	}

  void SetSpawnedSapperTarget(SpawnedSapper& sapper, const Vec3& targetPosition, const MapData& mapData)
	{
		const Vec3 currentPosition = GetSpawnedSapperBasePosition(sapper);
		const Vec3 groundedTargetPosition{ targetPosition.x, 0.0, targetPosition.z };
       sapper.destinationPosition = groundedTargetPosition;

       if (currentPosition.distanceFrom(groundedTargetPosition) <= 0.05)
		{
			sapper.position = groundedTargetPosition;
			sapper.startPosition = groundedTargetPosition;
          sapper.targetPosition = groundedTargetPosition;
			sapper.moveDuration = 0.0;
			return;
		}

       const Vec3 nextTarget = FindNextRouteTarget(currentPosition, groundedTargetPosition, mapData).value_or(groundedTargetPosition);
		StartSapperMoveSegment(sapper, currentPosition, nextTarget);
	}
}
