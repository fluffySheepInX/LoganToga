# include "MapEditorUpdateInternal.hpp"

namespace MapEditorUpdateDetail
{
	void ResetPendingPlacementStates(MapEditorState& state)
	{
		state.pendingWallPlacementStartPosition.reset();
		state.pendingRoadPlacementStartPosition.reset();
		state.pendingTireTrackPlacementStartPosition.reset();
	}

	void ResetInactiveEditorState(MapEditorState& state)
	{
		state.hoveredGroundPosition.reset();
		ResetPendingPlacementStates(state);
		state.roadResizeDrag.reset();
		state.roadRotateDrag.reset();
		state.lastTerrainPaintCell.reset();
		state.pendingTerrainPaintRangeStartCell.reset();
	}

	void ValidateSelections(MapEditorState& state, const MapData& mapData)
	{
		if (not MapEditorDetail::IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			state.selectedPlacedModelIndex.reset();
			state.roadResizeDrag.reset();
			state.roadRotateDrag.reset();
		}

		if (not MapEditorDetail::IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
		{
			state.selectedResourceAreaIndex.reset();
		}

		if (not MapEditorDetail::IsValidNavPointIndex(mapData, state.selectedNavPointIndex))
		{
			state.selectedNavPointIndex.reset();
		}

		if (not MapEditorDetail::IsValidNavPointIndex(mapData, state.pendingNavLinkStartIndex))
		{
			state.pendingNavLinkStartIndex.reset();
		}
	}

	void ResetDragStateWhenMouseReleased(MapEditorState& state)
	{
		state.lastTerrainPaintCell.reset();
		state.roadResizeDrag.reset();
		state.roadRotateDrag.reset();
	}

	void ResetPendingStatesForSelectedTool(MapEditorState& state)
	{
		if (state.selectedTool != MapEditorTool::PlaceWall)
		{
			state.pendingWallPlacementStartPosition.reset();
		}

		if (state.selectedTool != MapEditorTool::PlaceRoad)
		{
			state.pendingRoadPlacementStartPosition.reset();
		}

		if (state.selectedTool != MapEditorTool::PlaceTireTrackDecal)
		{
			state.pendingTireTrackPlacementStartPosition.reset();
		}

		if ((not MapEditorDetail::ToTerrainCellType(state.selectedTool)) && (state.selectedTool != MapEditorTool::EraseTerrain))
		{
			state.pendingTerrainPaintRangeStartCell.reset();
		}
	}

	void ClearSelection(MapEditorState& state)
	{
		state.selectedPlacedModelIndex.reset();
		state.selectedResourceAreaIndex.reset();
		state.selectedNavPointIndex.reset();
		ResetPendingPlacementStates(state);
	}

	void SelectNavPoint(MapEditorState& state, const size_t selectedIndex)
	{
		state.selectedNavPointIndex = selectedIndex;
		state.selectedPlacedModelIndex.reset();
		state.selectedResourceAreaIndex.reset();
	}

	void SelectPlacedModel(MapEditorState& state, const size_t selectedIndex)
	{
		state.selectedPlacedModelIndex = selectedIndex;
		state.selectedResourceAreaIndex.reset();
		state.selectedNavPointIndex.reset();
	}

	void SelectResourceArea(MapEditorState& state, const size_t selectedIndex)
	{
		state.selectedResourceAreaIndex = selectedIndex;
		state.selectedPlacedModelIndex.reset();
		state.selectedNavPointIndex.reset();
	}
}
