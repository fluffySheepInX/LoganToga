# pragma once
# include "MainContext.hpp"
# include "MapData.hpp"

namespace MainSupport
{
    [[nodiscard]] Optional<Vec3> GetWheelZoomFocusPosition(const DebugCamera3D& camera, const Vec3& playerBasePosition);
	[[nodiscard]] Vec3 GetSapperPopTargetPosition(const Vec3& rallyPoint, size_t sapperIndex);
	void DrawPlacedModel(const PlacedModel& placedModel, const Model& millModel, const Model& treeModel, const Model& pineModel);
	void DrawSelectionIndicator(const DebugCamera3D& camera, const Vec3& position);
	void DrawGroundContactOverlay(const DebugCamera3D& camera, const Vec3& position);
}
