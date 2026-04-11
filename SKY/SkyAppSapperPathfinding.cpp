# include "SkyAppSapperMovementInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace
{
	struct RoutePlan
	{
		Vec3 nextTarget{ 0, 0, 0 };
		Vec3 resolvedDestination{ 0, 0, 0 };
	};

	struct NavRouteSearch
	{
		Array<double> bestDistances;
		Array<Optional<size_t>> previousIndices;
	};

 [[nodiscard]] NavRouteSearch BuildNavRouteSearch(const Vec3& start, const MapData& mapData, const double sapperBodyRadius)
	{
		NavRouteSearch result{
			.bestDistances = Array<double>(mapData.navPoints.size(), Math::Inf),
			.previousIndices = Array<Optional<size_t>>(mapData.navPoints.size()),
		};
		Array<bool> visited(mapData.navPoints.size(), false);

		for (size_t i = 0; i < mapData.navPoints.size(); ++i)
		{
          if (not SkyAppSupport::SapperMovementDetail::IsPathSegmentBlocked(start, mapData.navPoints[i].position, mapData.placedModels, sapperBodyRadius))
			{
				result.bestDistances[i] = start.distanceFrom(mapData.navPoints[i].position);
			}
		}

		for (;;)
		{
			double nearestDistance = Math::Inf;
			Optional<size_t> currentIndex;

			for (size_t i = 0; i < result.bestDistances.size(); ++i)
			{
				if ((not visited[i]) && (result.bestDistances[i] < nearestDistance))
				{
					nearestDistance = result.bestDistances[i];
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
                     || SkyAppSupport::SapperMovementDetail::IsPathSegmentBlocked(mapData.navPoints[fromIndex].position, mapData.navPoints[toIndex].position, mapData.placedModels, sapperBodyRadius))
						{
							return;
						}

						const double segmentDistance = mapData.navPoints[fromIndex].position.distanceFrom(mapData.navPoints[toIndex].position);
						const double candidateDistance = (result.bestDistances[fromIndex] + (segmentDistance * Max(0.1, navLink.costMultiplier)));

						if (candidateDistance < result.bestDistances[toIndex])
						{
							result.bestDistances[toIndex] = candidateDistance;
							result.previousIndices[toIndex] = fromIndex;
						}
					};

				relax(navLink.fromIndex, navLink.toIndex);
				if (navLink.bidirectional)
				{
					relax(navLink.toIndex, navLink.fromIndex);
				}
			}
		}

		return result;
	}

	[[nodiscard]] size_t ResolveFirstStepIndex(const Array<Optional<size_t>>& previousIndices, size_t goalIndex)
	{
		size_t nextIndex = goalIndex;
		while (previousIndices[nextIndex])
		{
			nextIndex = *previousIndices[nextIndex];
		}

		return nextIndex;
	}

  [[nodiscard]] RoutePlan BuildRoutePlan(const SpawnedSapper& sapper, const Vec3& start, const Vec3& destination, const MapData& mapData, const ModelHeightSettings& modelHeightSettings)
	{
		const Vec3 groundedDestination{ destination.x, 0.0, destination.z };
		const double sapperBodyRadius = SkyAppSupport::SapperMovementDetail::GetScaledSapperBodyRadius(sapper, modelHeightSettings);

        if (not SkyAppSupport::SapperMovementDetail::IsPathSegmentBlocked(start, groundedDestination, mapData.placedModels, sapperBodyRadius))
		{
			return RoutePlan{ .nextTarget = groundedDestination, .resolvedDestination = groundedDestination };
		}

		if (mapData.navPoints.isEmpty())
		{
            const Vec3 reachablePoint = SkyAppSupport::SapperMovementDetail::ClampReachablePointOnSegment(start, groundedDestination, mapData.placedModels, sapperBodyRadius);
			return RoutePlan{ .nextTarget = reachablePoint, .resolvedDestination = reachablePoint };
		}

      const NavRouteSearch navRouteSearch = BuildNavRouteSearch(start, mapData, sapperBodyRadius);
		const Vec3 directFallback = SkyAppSupport::SapperMovementDetail::ClampReachablePointOnSegment(start, groundedDestination, mapData.placedModels, sapperBodyRadius);
		double bestGapDistanceSq = groundedDestination.distanceFromSq(directFallback);
		double bestTravelDistance = start.distanceFrom(directFallback);
		RoutePlan bestFallbackPlan{ .nextTarget = directFallback, .resolvedDestination = directFallback };
		Optional<size_t> bestGoalIndex;
		double bestGoalRouteDistance = Math::Inf;

		for (size_t i = 0; i < mapData.navPoints.size(); ++i)
		{
			if (navRouteSearch.bestDistances[i] == Math::Inf)
			{
				continue;
			}

            if (not SkyAppSupport::SapperMovementDetail::IsPathSegmentBlocked(mapData.navPoints[i].position, groundedDestination, mapData.placedModels, sapperBodyRadius))
			{
				const double candidateRouteDistance = (navRouteSearch.bestDistances[i] + mapData.navPoints[i].position.distanceFrom(groundedDestination));
				if (candidateRouteDistance < bestGoalRouteDistance)
				{
					bestGoalRouteDistance = candidateRouteDistance;
					bestGoalIndex = i;
				}
			}

           const Vec3 candidateReachablePoint = SkyAppSupport::SapperMovementDetail::ClampReachablePointOnSegment(mapData.navPoints[i].position, groundedDestination, mapData.placedModels, sapperBodyRadius);
			const double candidateGapDistanceSq = groundedDestination.distanceFromSq(candidateReachablePoint);
			const double candidateTravelDistance = (navRouteSearch.bestDistances[i] + mapData.navPoints[i].position.distanceFrom(candidateReachablePoint));

			if ((candidateGapDistanceSq < bestGapDistanceSq)
				|| ((Abs(candidateGapDistanceSq - bestGapDistanceSq) <= 0.01) && (candidateTravelDistance < bestTravelDistance)))
			{
				bestGapDistanceSq = candidateGapDistanceSq;
				bestTravelDistance = candidateTravelDistance;
				bestFallbackPlan = RoutePlan{
					.nextTarget = mapData.navPoints[ResolveFirstStepIndex(navRouteSearch.previousIndices, i)].position,
					.resolvedDestination = candidateReachablePoint,
				};
			}
		}

		if (bestGoalIndex)
		{
			return RoutePlan{
				.nextTarget = mapData.navPoints[ResolveFirstStepIndex(navRouteSearch.previousIndices, *bestGoalIndex)].position,
				.resolvedDestination = groundedDestination,
			};
		}

		if ((bestFallbackPlan.nextTarget.distanceFromSq(start) <= 0.01)
			&& (bestFallbackPlan.resolvedDestination.distanceFromSq(start) <= 0.01))
		{
			const Vec3 nearestReachablePoint = start;
			return RoutePlan{ .nextTarget = nearestReachablePoint, .resolvedDestination = nearestReachablePoint };
		}

		return bestFallbackPlan;
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
  void SetSpawnedSapperTarget(SpawnedSapper& sapper, const Vec3& targetPosition, const MapData& mapData, const ModelHeightSettings& modelHeightSettings)
	{
		const Vec3 currentPosition = GetSpawnedSapperBasePosition(sapper);
		const Vec3 groundedTargetPosition{ targetPosition.x, 0.0, targetPosition.z };
       const RoutePlan routePlan = BuildRoutePlan(sapper, currentPosition, groundedTargetPosition, mapData, modelHeightSettings);
		sapper.destinationPosition = routePlan.resolvedDestination;

		if (currentPosition.distanceFrom(routePlan.resolvedDestination) <= 0.05)
		{
			sapper.position = routePlan.resolvedDestination;
			sapper.startPosition = routePlan.resolvedDestination;
			sapper.targetPosition = routePlan.resolvedDestination;
			sapper.moveDuration = 0.0;
			return;
		}

		StartSapperMoveSegment(sapper, currentPosition, routePlan.nextTarget);
	}
}
