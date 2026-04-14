# include "MapEditorUpdateInternal.hpp"

namespace
{
	[[nodiscard]] bool HandleRoadRotationDrag(MapEditorState& state, MapData& mapData)
	{
		if ((not state.roadRotateDrag) || (not MouseL.pressed()))
		{
			return false;
		}

		if (MapEditorDetail::IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex)
			&& state.hoveredGroundPosition
			&& (mapData.placedModels[*state.selectedPlacedModelIndex].type == PlaceableModelType::Road)
			&& (*state.selectedPlacedModelIndex == state.roadRotateDrag->placedModelIndex))
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			placedModel.yaw = MapEditorDetail::ComputeWallYaw(placedModel.position, *state.hoveredGroundPosition, placedModel.yaw);
			MapEditorDetail::SetStatusMessage(state, U"Road を回転");
			return true;
		}

		state.roadRotateDrag.reset();
		return false;
	}

	[[nodiscard]] bool HandleRoadResizeDrag(MapEditorState& state, MapData& mapData)
	{
		if ((not state.roadResizeDrag) || (not MouseL.pressed()))
		{
			return false;
		}

		if (MapEditorDetail::IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex)
			&& state.hoveredGroundPosition
			&& (mapData.placedModels[*state.selectedPlacedModelIndex].type == PlaceableModelType::Road)
			&& (*state.selectedPlacedModelIndex == state.roadResizeDrag->placedModelIndex))
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			MapEditorDetail::ResizeRoadFromCorner(placedModel, state.roadResizeDrag->draggedCornerIndex, *state.hoveredGroundPosition, state.roadResizeDrag->fixedCornerPosition);
			MapEditorDetail::SetStatusMessage(state, U"Road を引き延ばし");
			return true;
		}

		state.roadResizeDrag.reset();
		return false;
	}

	[[nodiscard]] bool HandleSelectionMove(MapEditorState& state, MapData& mapData)
	{
		if (MouseL.down())
		{
			return false;
		}

		if (MouseL.pressed() && state.selectedNavPointIndex && state.hoveredGroundPosition)
		{
			NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
			navPoint.position = *state.hoveredGroundPosition;
			MapEditorDetail::SetStatusMessage(state, U"NavPoint {} を移動"_fmt(*state.selectedNavPointIndex));
		}
		else if (MouseL.pressed() && state.selectedPlacedModelIndex && state.hoveredGroundPosition)
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			placedModel.position = *state.hoveredGroundPosition;
			MapEditorDetail::SetStatusMessage(state, U"{} を移動"_fmt(ToString(placedModel.type)));
		}
		else if (MouseL.pressed() && state.selectedResourceAreaIndex && state.hoveredGroundPosition)
		{
			ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
			resourceArea.position = *state.hoveredGroundPosition;
			MapEditorDetail::SetStatusMessage(state, U"{} を移動"_fmt(ToString(resourceArea.type)));
		}

		return true;
	}

	[[nodiscard]] bool TryBeginRoadHandleDrag(MapEditorState& state, const MapData& mapData)
	{
		if (not MapEditorDetail::IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			return false;
		}

		if ((not state.hoveredGroundPosition)
			|| (mapData.placedModels[*state.selectedPlacedModelIndex].type != PlaceableModelType::Road))
		{
			return false;
		}

		const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
		if (MapEditorDetail::HitTestRoadRotationHandle(placedModel, state.hoveredGroundPosition))
		{
			state.roadRotateDrag = RoadRotateDragState{
				.placedModelIndex = *state.selectedPlacedModelIndex,
			};
			MapEditorDetail::SetStatusMessage(state, U"Road をドラッグして回転");
			return true;
		}

		if (const auto cornerIndex = MapEditorDetail::HitTestRoadCornerHandle(placedModel, state.hoveredGroundPosition))
		{
			const Array<Vec3> corners = MapEditorDetail::GetRoadCorners(placedModel);
			state.roadResizeDrag = RoadResizeDragState{
				.placedModelIndex = *state.selectedPlacedModelIndex,
				.draggedCornerIndex = *cornerIndex,
				.fixedCornerPosition = corners[static_cast<size_t>((*cornerIndex + 2) % static_cast<int32>(corners.size()))],
			};
			MapEditorDetail::SetStatusMessage(state, U"Road の角をドラッグして引き延ばし");
			return true;
		}

		return false;
	}
}

namespace MapEditorUpdateDetail
{
	bool HandleSelectionMode(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera)
	{
		if (not state.selectionMode)
		{
			return false;
		}

		if (HandleRoadRotationDrag(state, mapData))
		{
			return true;
		}

		if (HandleRoadResizeDrag(state, mapData))
		{
			return true;
		}

		if (HandleSelectionMove(state, mapData))
		{
			return true;
		}

		if (TryBeginRoadHandleDrag(state, mapData))
		{
			return true;
		}

     if (state.showNavPoints)
		{
          if (const auto selectedIndex = MapEditorDetail::HitTestNavPoint(mapData.navPoints, camera))
			{
				SelectNavPoint(state, *selectedIndex);
				MapEditorDetail::SetStatusMessage(state, U"NavPoint {} を選択"_fmt(*selectedIndex));
				return true;
			}
		}

		if (const auto selectedIndex = MapEditorDetail::HitTestPlacedModel(mapData.placedModels, camera, state.hoveredGroundPosition))
		{
			SelectPlacedModel(state, *selectedIndex);
			MapEditorDetail::SetStatusMessage(state, U"{} を選択"_fmt(ToString(mapData.placedModels[*selectedIndex].type)));
			return true;
		}

		if (const auto selectedIndex = MapEditorDetail::HitTestResourceArea(mapData.resourceAreas, state.hoveredGroundPosition))
		{
			SelectResourceArea(state, *selectedIndex);
			MapEditorDetail::SetStatusMessage(state, U"{} を選択"_fmt(ToString(mapData.resourceAreas[*selectedIndex].type)));
			return true;
		}

		ClearSelection(state);
		MapEditorDetail::SetStatusMessage(state, U"選択解除");
		return true;
	}
}
