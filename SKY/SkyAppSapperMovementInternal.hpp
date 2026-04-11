# pragma once
# include "SkyAppSapperInternal.hpp"

namespace SkyAppSupport
{
	namespace SapperMovementDetail
	{
		[[nodiscard]] Vec2 ToHorizontal(const Vec3& value);
		[[nodiscard]] bool IsWallObstacle(const PlacedModel& placedModel);
		[[nodiscard]] double GetObstacleCoreRadius(const PlacedModel& placedModel);
		[[nodiscard]] bool IsObstacleModel(const PlacedModel& placedModel);
		[[nodiscard]] Vec2 GetClosestPointOnObstacle(const PlacedModel& placedModel, const Vec2& point);
       [[nodiscard]] double GetScaledSapperBodyRadius(const MainSupport::SpawnedSapper& sapper, const MainSupport::ModelHeightSettings& modelHeightSettings);
		[[nodiscard]] bool IsPathSegmentBlocked(const Vec3& start, const Vec3& goal, const Array<PlacedModel>& placedModels, double sapperBodyRadius);
		[[nodiscard]] Vec3 ClampReachablePointOnSegment(const Vec3& start, const Vec3& goal, const Array<PlacedModel>& placedModels, double sapperBodyRadius);
	}
}
