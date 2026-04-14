# pragma once
# include "MapEditorInternal.hpp"

namespace MapEditorUpdateDetail
{
	void ResetPendingPlacementStates(MapEditorState& state);
	void ResetInactiveEditorState(MapEditorState& state);
	void ValidateSelections(MapEditorState& state, const MapData& mapData);
	void ResetDragStateWhenMouseReleased(MapEditorState& state);
	void ResetPendingStatesForSelectedTool(MapEditorState& state);
	void ClearSelection(MapEditorState& state);
	void SelectNavPoint(MapEditorState& state, size_t selectedIndex);
	void SelectPlacedModel(MapEditorState& state, size_t selectedIndex);
	void SelectResourceArea(MapEditorState& state, size_t selectedIndex);
	[[nodiscard]] bool HandleSelectionMode(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera);
	[[nodiscard]] bool HandleTerrainEditing(MapEditorState& state, MapData& mapData);
	[[nodiscard]] bool HandleRoadPlacement(MapEditorState& state, MapData& mapData);
	[[nodiscard]] bool HandleTireTrackPlacement(MapEditorState& state, MapData& mapData);
	[[nodiscard]] bool HandleWallPlacement(MapEditorState& state, MapData& mapData);
	void HandleGroundPlacement(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera);
}
