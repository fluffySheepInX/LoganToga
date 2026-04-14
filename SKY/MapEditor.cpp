# include "MapEditorInternal.hpp"
# include "MapEditorUpdateInternal.hpp"

using namespace MapEditorUpdateDetail;

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, const bool canHandleSceneInput)
{
	if (not state.enabled)
	{
        ResetInactiveEditorState(state);
		return;
	}

   state.hoveredGroundPosition = MapEditorDetail::GetGroundIntersection(camera);
	ValidateSelections(state, mapData);

	if (not canHandleSceneInput)
	{
     if (not MouseL.pressed())
		{
			state.pendingTerrainPaintRangeStartCell.reset();
		}

		return;
	}

	if (not MouseL.pressed())
	{
     ResetDragStateWhenMouseReleased(state);
	}

 ResetPendingStatesForSelectedTool(state);

    if (HandleSelectionMode(state, mapData, camera))
	{
		return;
	}

  if (HandleTerrainEditing(state, mapData))
	{
		return;
	}

  if (HandleRoadPlacement(state, mapData))
	{
		return;
	}

   if (HandleTireTrackPlacement(state, mapData))
	{
		return;
	}

 if (HandleWallPlacement(state, mapData))
	{
		return;
	}

   HandleGroundPlacement(state, mapData, camera);
}
