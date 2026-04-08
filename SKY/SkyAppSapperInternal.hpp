# pragma once
# include "SkyAppSupport.hpp"

namespace SkyAppSupport
{
	namespace SapperInternal
	{
		inline constexpr double InitialSapperMoveDuration = 0.45;
		inline constexpr double DefaultSapperMoveSpeed = 6.0;
		inline constexpr double MinimumSapperMoveSpeed = 0.5;
		inline constexpr double MinimumSuppressionMultiplier = 0.1;
		inline constexpr double MaximumSuppressionAttackIntervalMultiplier = 10.0;
		inline constexpr double MinimumSapperMoveDuration = 0.08;
		inline constexpr double MinimumSapperTurnDistanceSq = 0.0001;
		inline constexpr double SapperBodyRadius = 1.2;
		inline constexpr double RockObstacleRadius = 2.6;
		inline constexpr double ObstacleAvoidancePadding = 0.3;

		[[nodiscard]] double ToSapperYaw(const Vec3& direction, double fallbackYaw);
		[[nodiscard]] Vec3 ToHorizontalDirectionOrFallback(const Vec3& direction, const Vec3& fallback);
		[[nodiscard]] double GetSapperCombatStopDistance(const MainSupport::SpawnedSapper& attacker, const MainSupport::SpawnedSapper& defender);
		[[nodiscard]] Optional<double> GetPlacedModelObstacleRadius(const PlacedModel& placedModel);
		void StopSapperAtPosition(MainSupport::SpawnedSapper& sapper, const Vec3& position);
		[[nodiscard]] Optional<size_t> FindNearestSapperInRange(const MainSupport::SpawnedSapper& source, const Array<MainSupport::SpawnedSapper>& candidates);
	}
}
