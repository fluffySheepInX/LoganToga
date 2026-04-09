# include "SkyAppSapperMovementInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace
{
	[[nodiscard]] double DistanceSqPointToSegment(const Vec2& point, const Vec2& start, const Vec2& goal)
	{
		const Vec2 segment = (goal - start);
		const double segmentLengthSq = segment.lengthSq();

		if (segmentLengthSq <= SkyAppSupport::SapperInternal::MinimumSapperTurnDistanceSq)
		{
			return point.distanceFromSq(start);
		}

		const double t = Clamp(((point - start).dot(segment) / segmentLengthSq), 0.0, 1.0);
		return point.distanceFromSq(start + (segment * t));
	}

	[[nodiscard]] Vec2 GetWallDirection(const PlacedModel& placedModel)
	{
		return Vec2{ Math::Sin(placedModel.yaw), Math::Cos(placedModel.yaw) };
	}

	[[nodiscard]] double GetWallLength(const PlacedModel& placedModel)
	{
		return Clamp(placedModel.wallLength, 2.0, 80.0);
	}

	[[nodiscard]] std::pair<Vec2, Vec2> GetWallSegment(const PlacedModel& placedModel)
	{
		const Vec2 center = SkyAppSupport::SapperMovementDetail::ToHorizontal(placedModel.position);
		const Vec2 halfDirection = (GetWallDirection(placedModel) * (GetWallLength(placedModel) * 0.5));
		return { (center - halfDirection), (center + halfDirection) };
	}

	[[nodiscard]] double Cross(const Vec2& a, const Vec2& b)
	{
		return ((a.x * b.y) - (a.y * b.x));
	}

	[[nodiscard]] bool IntersectsSegment2D(const Vec2& a0, const Vec2& a1, const Vec2& b0, const Vec2& b1)
	{
		const Vec2 ab = (a1 - a0);
		const Vec2 ac = (b0 - a0);
		const Vec2 ad = (b1 - a0);
		const Vec2 cd = (b1 - b0);
		const Vec2 ca = (a0 - b0);
		const Vec2 cb = (a1 - b0);
		const double cross1 = Cross(ab, ac);
		const double cross2 = Cross(ab, ad);
		const double cross3 = Cross(cd, ca);
		const double cross4 = Cross(cd, cb);
		return (((cross1 <= 0.0) && (0.0 <= cross2)) || ((cross2 <= 0.0) && (0.0 <= cross1)))
			&& (((cross3 <= 0.0) && (0.0 <= cross4)) || ((cross4 <= 0.0) && (0.0 <= cross3)));
	}

	[[nodiscard]] double DistanceSqSegmentToSegment(const Vec2& a0, const Vec2& a1, const Vec2& b0, const Vec2& b1)
	{
		if (IntersectsSegment2D(a0, a1, b0, b1))
		{
			return 0.0;
		}

		return Min({
			DistanceSqPointToSegment(a0, b0, b1),
			DistanceSqPointToSegment(a1, b0, b1),
			DistanceSqPointToSegment(b0, a0, a1),
			DistanceSqPointToSegment(b1, a0, a1),
		});
	}
}

namespace SkyAppSupport
{
	namespace SapperMovementDetail
	{
		Vec2 ToHorizontal(const Vec3& value)
		{
			return Vec2{ value.x, value.z };
		}

		bool IsWallObstacle(const PlacedModel& placedModel)
		{
			return (placedModel.type == PlaceableModelType::Wall);
		}

		double GetObstacleCoreRadius(const PlacedModel& placedModel)
		{
			if (IsWallObstacle(placedModel))
			{
				return 0.55;
			}

			return SapperInternal::GetPlacedModelObstacleRadius(placedModel).value_or(0.0);
		}

		bool IsObstacleModel(const PlacedModel& placedModel)
		{
			return IsWallObstacle(placedModel) || SapperInternal::GetPlacedModelObstacleRadius(placedModel).has_value();
		}

		Vec2 GetClosestPointOnObstacle(const PlacedModel& placedModel, const Vec2& point)
		{
			if (IsWallObstacle(placedModel))
			{
				const auto [start, goal] = GetWallSegment(placedModel);
				const Vec2 segment = (goal - start);
				const double segmentLengthSq = segment.lengthSq();

				if (segmentLengthSq <= SapperInternal::MinimumSapperTurnDistanceSq)
				{
					return start;
				}

				const double t = Clamp(((point - start).dot(segment) / segmentLengthSq), 0.0, 1.0);
				return (start + (segment * t));
			}

			return ToHorizontal(placedModel.position);
		}

		bool IsPathSegmentBlocked(const Vec3& start, const Vec3& goal, const Array<PlacedModel>& placedModels)
		{
			const Vec2 start2 = ToHorizontal(start);
			const Vec2 goal2 = ToHorizontal(goal);

			if ((goal2 - start2).lengthSq() <= SapperInternal::MinimumSapperTurnDistanceSq)
			{
				return false;
			}

			for (const auto& placedModel : placedModels)
			{
				if (not IsObstacleModel(placedModel))
				{
					continue;
				}

				const double requiredDistance = (GetObstacleCoreRadius(placedModel) + SapperInternal::SapperBodyRadius + SapperInternal::ObstacleAvoidancePadding);
				if (IsWallObstacle(placedModel))
				{
					const auto [wallStart, wallGoal] = GetWallSegment(placedModel);
					if (DistanceSqSegmentToSegment(start2, goal2, wallStart, wallGoal) < Square(requiredDistance))
					{
						return true;
					}
					continue;
				}

				const Vec2 obstacle2 = ToHorizontal(placedModel.position);
				if (DistanceSqPointToSegment(obstacle2, start2, goal2) < Square(requiredDistance))
				{
					return true;
				}
			}

			return false;
		}

		Vec3 ClampReachablePointOnSegment(const Vec3& start, const Vec3& goal, const Array<PlacedModel>& placedModels)
		{
			if ((goal - start).lengthSq() <= 0.0001)
			{
				return Vec3{ start.x, 0.0, start.z };
			}

			double low = 0.0;
			double high = 1.0;
			for (int32 i = 0; i < 18; ++i)
			{
				const double mid = ((low + high) * 0.5);
				const Vec3 candidate = start.lerp(goal, mid);
				if (IsPathSegmentBlocked(start, candidate, placedModels))
				{
					high = mid;
				}
				else
				{
					low = mid;
				}
			}

			const Vec3 reachablePoint = start.lerp(goal, low);
			return Vec3{ reachablePoint.x, 0.0, reachablePoint.z };
		}
	}

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
				if (not SapperMovementDetail::IsObstacleModel(placedModel))
				{
					continue;
				}

				const double requiredDistance = (SapperMovementDetail::GetObstacleCoreRadius(placedModel) + SapperInternal::SapperBodyRadius + SapperInternal::ObstacleAvoidancePadding);
				const Vec2 adjustedPosition2 = SapperMovementDetail::ToHorizontal(adjustedPosition);
				const Vec2 closestObstaclePoint = SapperMovementDetail::GetClosestPointOnObstacle(placedModel, adjustedPosition2);
				const Vec2 toObstacle2 = (closestObstaclePoint - adjustedPosition2);
				const double distanceSq = toObstacle2.lengthSq();

				if (distanceSq < Square(requiredDistance))
				{
					const Vec3 obstaclePosition{ closestObstaclePoint.x, 0.0, closestObstaclePoint.y };
					const Vec3 pushDirection = SapperInternal::ToHorizontalDirectionOrFallback((adjustedPosition - obstaclePosition), (adjustedPosition - desiredTarget));
					adjustedPosition = obstaclePosition + (pushDirection * requiredDistance);
					adjusted = true;
					continue;
				}

				if (SapperMovementDetail::IsWallObstacle(placedModel))
				{
					continue;
				}

				const Vec3 obstaclePosition = placedModel.position;
				const Vec3 toObstacle = (obstaclePosition - adjustedPosition);
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
}
