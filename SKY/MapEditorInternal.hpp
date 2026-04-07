# pragma once
# include "MapEditor.hpp"

namespace MapEditorDetail
{
    [[nodiscard]] Optional<Vec3> GetGroundIntersection(const MainSupport::AppCamera3D& camera);
	void SetStatusMessage(MapEditorState& state, const String& message);
	[[nodiscard]] bool DrawEditorButton(const Rect& rect, StringView label, bool selected = false);
	[[nodiscard]] double GetPlacedModelSelectionRadius(const PlacedModel& placedModel);
	[[nodiscard]] bool IsValidPlacedModelIndex(const MapData& mapData, const Optional<size_t>& index);
 [[nodiscard]] Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera);
	[[nodiscard]] StringView ToLabel(MapEditorTool tool);
	[[nodiscard]] Optional<PlaceableModelType> ToPlaceableModelType(MapEditorTool tool);
	[[nodiscard]] Optional<MainSupport::ResourceType> ToResourceType(MapEditorTool tool);
	[[nodiscard]] ColorF GetResourceAreaColor(MainSupport::ResourceType type);
	[[nodiscard]] int32 FindResourceAreaIndex(const Array<ResourceArea>& resourceAreas, MainSupport::ResourceType type);
}
