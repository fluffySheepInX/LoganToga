# pragma once
# include "MapEditor.hpp"

namespace MapEditorDetail
{
    [[nodiscard]] Optional<Vec3> GetGroundIntersection(const MainSupport::AppCamera3D& camera);
	void SetStatusMessage(MapEditorState& state, const String& message);
	[[nodiscard]] bool DrawEditorButton(const Rect& rect, StringView label, bool selected = false);
	[[nodiscard]] double GetPlacedModelSelectionRadius(const PlacedModel& placedModel);
  [[nodiscard]] double GetNavPointSelectionRadius(const NavPoint& navPoint);
  [[nodiscard]] double ComputeWallYaw(const Vec3& startPosition, const Vec3& endPosition, double fallbackYaw = 0.0);
  [[nodiscard]] double ComputeWallLength(const Vec3& startPosition, const Vec3& endPosition, double fallbackLength = 10.0);
  [[nodiscard]] PlacedModel BuildWallFromStartAndEnd(const Vec3& startPosition, const Vec3& endPosition, double fallbackLength = 10.0, double fallbackYaw = 0.0);
  [[nodiscard]] PlacedModel BuildRoadFromStartAndEnd(const Vec3& startPosition, const Vec3& endPosition, double fallbackLength = 8.0, double fallbackWidth = 4.0, double fallbackYaw = 0.0);
	[[nodiscard]] bool IsValidPlacedModelIndex(const MapData& mapData, const Optional<size_t>& index);
    [[nodiscard]] bool IsValidResourceAreaIndex(const MapData& mapData, const Optional<size_t>& index);
	[[nodiscard]] bool IsValidNavPointIndex(const MapData& mapData, const Optional<size_t>& index);
  [[nodiscard]] Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera, const Optional<Vec3>& hoveredGroundPosition);
	[[nodiscard]] Optional<size_t> HitTestResourceArea(const Array<ResourceArea>& resourceAreas, const Optional<Vec3>& hoveredGroundPosition);
	[[nodiscard]] Optional<size_t> HitTestNavPoint(const Array<NavPoint>& navPoints, const MainSupport::AppCamera3D& camera);
   [[nodiscard]] Array<Vec3> GetRoadCorners(const PlacedModel& placedModel);
 [[nodiscard]] Vec3 GetRoadRotationHandlePosition(const PlacedModel& placedModel);
	[[nodiscard]] Optional<int32> HitTestRoadCornerHandle(const PlacedModel& placedModel, const Optional<Vec3>& hoveredGroundPosition);
    [[nodiscard]] bool HitTestRoadRotationHandle(const PlacedModel& placedModel, const Optional<Vec3>& hoveredGroundPosition);
	void ResizeRoadFromCorner(PlacedModel& placedModel, int32 draggedCornerIndex, const Vec3& draggedPosition, const Vec3& fixedCornerPosition);
	[[nodiscard]] size_t CountNavLinksForPoint(const MapData& mapData, size_t navPointIndex);
	void RemoveNavPointAt(MapData& mapData, size_t navPointIndex);
	[[nodiscard]] bool ToggleNavLink(MapData& mapData, size_t firstIndex, size_t secondIndex);
	[[nodiscard]] StringView ToLabel(MapEditorTool tool);
	[[nodiscard]] Optional<PlaceableModelType> ToPlaceableModelType(MapEditorTool tool);
   [[nodiscard]] Optional<TerrainCellType> ToTerrainCellType(MapEditorTool tool);
	[[nodiscard]] Optional<MainSupport::ResourceType> ToResourceType(MapEditorTool tool);
	[[nodiscard]] ColorF GetResourceAreaColor(MainSupport::ResourceType type);
}
