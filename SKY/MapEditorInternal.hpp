# pragma once
# include "MapEditor.hpp"

namespace MapEditorDetail
{
    [[nodiscard]] Optional<Vec3> GetGroundIntersection(const MainSupport::AppCamera3D& camera);
	void SetStatusMessage(MapEditorState& state, const String& message);
	[[nodiscard]] bool DrawEditorButton(const Rect& rect, StringView label, bool selected = false);
	[[nodiscard]] double GetPlacedModelSelectionRadius(const PlacedModel& placedModel);
  [[nodiscard]] double GetNavPointSelectionRadius(const NavPoint& navPoint);
	[[nodiscard]] bool IsValidPlacedModelIndex(const MapData& mapData, const Optional<size_t>& index);
    [[nodiscard]] bool IsValidResourceAreaIndex(const MapData& mapData, const Optional<size_t>& index);
	[[nodiscard]] bool IsValidNavPointIndex(const MapData& mapData, const Optional<size_t>& index);
	[[nodiscard]] Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera);
	[[nodiscard]] Optional<size_t> HitTestResourceArea(const Array<ResourceArea>& resourceAreas, const Optional<Vec3>& hoveredGroundPosition);
	[[nodiscard]] Optional<size_t> HitTestNavPoint(const Array<NavPoint>& navPoints, const MainSupport::AppCamera3D& camera);
	[[nodiscard]] size_t CountNavLinksForPoint(const MapData& mapData, size_t navPointIndex);
	void RemoveNavPointAt(MapData& mapData, size_t navPointIndex);
	[[nodiscard]] bool ToggleNavLink(MapData& mapData, size_t firstIndex, size_t secondIndex);
	[[nodiscard]] StringView ToLabel(MapEditorTool tool);
	[[nodiscard]] Optional<PlaceableModelType> ToPlaceableModelType(MapEditorTool tool);
	[[nodiscard]] Optional<MainSupport::ResourceType> ToResourceType(MapEditorTool tool);
	[[nodiscard]] ColorF GetResourceAreaColor(MainSupport::ResourceType type);
}
