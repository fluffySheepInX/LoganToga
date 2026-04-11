# include "SkyAppSapperInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace SapperInternal
	{
       struct FootprintSegment
		{
			Vec2 start;
			Vec2 end;
			double radius = 0.0;
		};

		struct ClosestSegmentPoints
		{
			Vec2 pointA;
			Vec2 pointB;
			double centerDistance = 0.0;
		};

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

		[[nodiscard]] Vec2 ToFootprintForward(const double facingYaw)
		{
			const double angle = (facingYaw - BirdDisplayYaw - SapperFacingYawOffset);
			return Vec2{ Math::Sin(angle), Math::Cos(angle) };
		}

     [[nodiscard]] FootprintSegment GetFootprintSegment(const SpawnedSapper& sapper, const ModelHeightSettings& modelHeightSettings)
		{
			const Vec3 basePosition = GetSpawnedSapperBasePosition(sapper);
			const Vec2 center{ basePosition.x, basePosition.z };
         const double scale = Max(ModelScaleMin, GetSpawnedSapperModelScale(modelHeightSettings, sapper));
			const double radius = Max(0.1, (sapper.footprintRadius * scale));
			const double halfLength = ((sapper.footprintType == UnitFootprintType::Capsule) ? Max(0.0, (sapper.footprintHalfLength * scale)) : 0.0);
			const Vec2 forward = ToFootprintForward(sapper.facingYaw);
			return FootprintSegment{
				.start = (center - forward * halfLength),
				.end = (center + forward * halfLength),
				.radius = radius,
			};
		}

		[[nodiscard]] ClosestSegmentPoints GetClosestPointsOnSegments(const Vec2& a0, const Vec2& a1, const Vec2& b0, const Vec2& b1)
		{
			constexpr double Epsilon = 0.000001;
			const Vec2 segmentA = (a1 - a0);
			const Vec2 segmentB = (b1 - b0);
			const Vec2 delta = (a0 - b0);
			const double lengthSqA = segmentA.lengthSq();
			const double lengthSqB = segmentB.lengthSq();
			double s = 0.0;
			double t = 0.0;

			if ((lengthSqA <= Epsilon) && (lengthSqB <= Epsilon))
			{
				return ClosestSegmentPoints{ a0, b0, a0.distanceFrom(b0) };
			}

			if (lengthSqA <= Epsilon)
			{
				t = Clamp(segmentB.dot(delta) / Max(lengthSqB, Epsilon), 0.0, 1.0);
			}
			else
			{
				const double c = segmentA.dot(delta);

				if (lengthSqB <= Epsilon)
				{
					s = Clamp(-c / lengthSqA, 0.0, 1.0);
				}
				else
				{
					const double b = segmentA.dot(segmentB);
					const double f = segmentB.dot(delta);
					const double denominator = (lengthSqA * lengthSqB - b * b);

					if (Abs(denominator) > Epsilon)
					{
						s = Clamp((b * f - c * lengthSqB) / denominator, 0.0, 1.0);
					}

					t = (b * s + f) / lengthSqB;

					if (t < 0.0)
					{
						t = 0.0;
						s = Clamp(-c / lengthSqA, 0.0, 1.0);
					}
					else if (1.0 < t)
					{
						t = 1.0;
						s = Clamp((b - c) / lengthSqA, 0.0, 1.0);
					}
				}
			}

			const Vec2 pointA = (a0 + segmentA * s);
			const Vec2 pointB = (b0 + segmentB * t);
			return ClosestSegmentPoints{ pointA, pointB, pointA.distanceFrom(pointB) };
		}

		[[nodiscard]] Vec2 ClosestPointOnSegment(const Vec2& point, const Vec2& segmentStart, const Vec2& segmentEnd)
		{
			const Vec2 segment = (segmentEnd - segmentStart);
			const double lengthSq = segment.lengthSq();

			if (lengthSq <= 0.000001)
			{
				return segmentStart;
			}

			const double t = Clamp(segment.dot(point - segmentStart) / lengthSq, 0.0, 1.0);
			return (segmentStart + segment * t);
		}

		[[nodiscard]] Vec2 GetSeparationDirection(const Vec2& fromPoint, const Vec2& toPoint, const Vec3& fallback)
		{
			const Vec2 direction = (toPoint - fromPoint);

			if (direction.lengthSq() <= MinimumSapperTurnDistanceSq)
			{
				const Vec3 fallbackDirection = ToHorizontalDirectionOrFallback(fallback, Vec3{ 0, 0, -1 });
				return Vec2{ fallbackDirection.x, fallbackDirection.z };
			}

			return direction.normalized();
		}

      [[nodiscard]] double GetSapperCombatStopDistance(const SpawnedSapper& attacker, const SpawnedSapper& defender, const ModelHeightSettings& modelHeightSettings)
		{
            return (Max(0.0, attacker.stopDistance)
				+ Max(0.1, (attacker.footprintRadius * Max(ModelScaleMin, GetSpawnedSapperModelScale(modelHeightSettings, attacker))))
				+ Max(0.1, (defender.footprintRadius * Max(ModelScaleMin, GetSpawnedSapperModelScale(modelHeightSettings, defender)))));
		}

       [[nodiscard]] double GetSapperCombatSurfaceDistance(const SpawnedSapper& attacker, const SpawnedSapper& defender, const ModelHeightSettings& modelHeightSettings)
		{
         const FootprintSegment attackerSegment = GetFootprintSegment(attacker, modelHeightSettings);
			const FootprintSegment defenderSegment = GetFootprintSegment(defender, modelHeightSettings);
			const ClosestSegmentPoints closestPoints = GetClosestPointsOnSegments(attackerSegment.start, attackerSegment.end, defenderSegment.start, defenderSegment.end);
			return (closestPoints.centerDistance - attackerSegment.radius - defenderSegment.radius);
		}

        [[nodiscard]] Vec3 GetSapperCombatStopPosition(const SpawnedSapper& attacker, const SpawnedSapper& defender, const ModelHeightSettings& modelHeightSettings)
		{
         const FootprintSegment attackerSegment = GetFootprintSegment(attacker, modelHeightSettings);
			const FootprintSegment defenderSegment = GetFootprintSegment(defender, modelHeightSettings);
			const ClosestSegmentPoints closestPoints = GetClosestPointsOnSegments(attackerSegment.start, attackerSegment.end, defenderSegment.start, defenderSegment.end);
			const double surfaceDistance = (closestPoints.centerDistance - attackerSegment.radius - defenderSegment.radius);
			const Vec2 separationDirection = GetSeparationDirection(closestPoints.pointB, closestPoints.pointA, (GetSpawnedSapperBasePosition(attacker) - GetSpawnedSapperBasePosition(defender)));
         const double stopDistanceDelta = (Max(0.0, attacker.stopDistance) - surfaceDistance);
			const Vec3 attackerPosition = GetSpawnedSapperBasePosition(attacker);
         return attackerPosition.movedBy((separationDirection.x * stopDistanceDelta), 0, (separationDirection.y * stopDistanceDelta));
		}

       [[nodiscard]] double GetSapperBaseCombatSurfaceDistance(const SpawnedSapper& attacker, const Vec3& basePosition, const double baseRadius, const ModelHeightSettings& modelHeightSettings)
		{
         const FootprintSegment attackerSegment = GetFootprintSegment(attacker, modelHeightSettings);
			const Vec2 baseCenter{ basePosition.x, basePosition.z };
			const Vec2 closestPoint = ClosestPointOnSegment(baseCenter, attackerSegment.start, attackerSegment.end);
			return (closestPoint.distanceFrom(baseCenter) - attackerSegment.radius - baseRadius);
		}

        [[nodiscard]] Vec3 GetSapperBaseCombatStopPosition(const SpawnedSapper& attacker, const Vec3& basePosition, const double baseRadius, const ModelHeightSettings& modelHeightSettings)
		{
         const FootprintSegment attackerSegment = GetFootprintSegment(attacker, modelHeightSettings);
			const Vec2 baseCenter{ basePosition.x, basePosition.z };
			const Vec2 closestPoint = ClosestPointOnSegment(baseCenter, attackerSegment.start, attackerSegment.end);
			const double surfaceDistance = (closestPoint.distanceFrom(baseCenter) - attackerSegment.radius - baseRadius);
			const Vec2 separationDirection = GetSeparationDirection(baseCenter, closestPoint, (GetSpawnedSapperBasePosition(attacker) - basePosition));
         const double stopDistanceDelta = (Max(0.0, attacker.stopDistance) - surfaceDistance);
			const Vec3 attackerPosition = GetSpawnedSapperBasePosition(attacker);
         return attackerPosition.movedBy((separationDirection.x * stopDistanceDelta), 0, (separationDirection.y * stopDistanceDelta));
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

        [[nodiscard]] Optional<size_t> FindNearestSapperInRange(const SpawnedSapper& source, const Array<SpawnedSapper>& candidates, const ModelHeightSettings& modelHeightSettings)
		{
			double nearestDistanceSq = Math::Inf;
			Optional<size_t> nearestIndex;
			const Vec3 sourcePosition = GetSpawnedSapperBasePosition(source);

			for (size_t i = 0; i < candidates.size(); ++i)
			{
             if (not IsSpawnedSapperCombatActive(candidates[i]))
				{
					continue;
				}

                const double surfaceDistance = GetSapperCombatSurfaceDistance(source, candidates[i], modelHeightSettings);

             if ((surfaceDistance <= source.attackRange) && (surfaceDistance < nearestDistanceSq))
				{
                 nearestDistanceSq = surfaceDistance;
					nearestIndex = i;
				}
			}

			return nearestIndex;
		}
	}

}
