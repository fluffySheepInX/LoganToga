# include "MapEditorInternal.hpp"

using namespace MapEditorDetail;

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, const bool canHandleSceneInput)
{
	if (not state.enabled)
	{
		state.hoveredGroundPosition.reset();
     state.pendingWallPlacementStartPosition.reset();
     state.pendingRoadPlacementStartPosition.reset();
        state.roadResizeDrag.reset();
        state.roadRotateDrag.reset();
     state.lastTerrainPaintCell.reset();
		return;
	}

	state.hoveredGroundPosition = GetGroundIntersection(camera);
	if (not IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		state.selectedPlacedModelIndex.reset();
       state.roadResizeDrag.reset();
       state.roadRotateDrag.reset();
	}
	if (not IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
	{
		state.selectedResourceAreaIndex.reset();
	}
	if (not IsValidNavPointIndex(mapData, state.selectedNavPointIndex))
	{
		state.selectedNavPointIndex.reset();
	}
	if (not IsValidNavPointIndex(mapData, state.pendingNavLinkStartIndex))
	{
		state.pendingNavLinkStartIndex.reset();
	}

	if (not canHandleSceneInput)
	{
		return;
	}

	if (not MouseL.pressed())
	{
		state.lastTerrainPaintCell.reset();
       state.roadResizeDrag.reset();
       state.roadRotateDrag.reset();
	}

	if (state.selectedTool != MapEditorTool::PlaceWall)
	{
		state.pendingWallPlacementStartPosition.reset();
	}

	if (state.selectedTool != MapEditorTool::PlaceRoad)
	{
		state.pendingRoadPlacementStartPosition.reset();
	}

	if (state.selectionMode)
	{
     if (state.roadRotateDrag && MouseL.pressed())
		{
			if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex)
				&& state.hoveredGroundPosition
				&& (mapData.placedModels[*state.selectedPlacedModelIndex].type == PlaceableModelType::Road)
				&& (*state.selectedPlacedModelIndex == state.roadRotateDrag->placedModelIndex))
			{
				PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
				placedModel.yaw = ComputeWallYaw(placedModel.position, *state.hoveredGroundPosition, placedModel.yaw);
				SetStatusMessage(state, U"Road を回転");
				return;
			}

			state.roadRotateDrag.reset();
		}

      if (state.roadResizeDrag && MouseL.pressed())
		{
			if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex)
				&& state.hoveredGroundPosition
				&& (mapData.placedModels[*state.selectedPlacedModelIndex].type == PlaceableModelType::Road)
				&& (*state.selectedPlacedModelIndex == state.roadResizeDrag->placedModelIndex))
			{
				PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
				ResizeRoadFromCorner(placedModel, state.roadResizeDrag->draggedCornerIndex, *state.hoveredGroundPosition, state.roadResizeDrag->fixedCornerPosition);
				SetStatusMessage(state, U"Road を引き延ばし");
				return;
			}

			state.roadResizeDrag.reset();
		}

		if (not MouseL.down())
		{
            if (MouseL.pressed() && state.selectedNavPointIndex && state.hoveredGroundPosition)
			{
				NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
				navPoint.position = *state.hoveredGroundPosition;
				SetStatusMessage(state, U"NavPoint {} を移動"_fmt(*state.selectedNavPointIndex));
				return;
			}

          if (MouseL.pressed() && state.selectedPlacedModelIndex && state.hoveredGroundPosition)
			{
				PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
				placedModel.position = *state.hoveredGroundPosition;
				SetStatusMessage(state, U"{} を移動"_fmt(ToString(placedModel.type)));
				return;
			}

         if (MouseL.pressed() && state.selectedResourceAreaIndex && state.hoveredGroundPosition)
			{
				ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
				resourceArea.position = *state.hoveredGroundPosition;
				SetStatusMessage(state, U"{} を移動"_fmt(ToString(resourceArea.type)));
				return;
			}

			return;
		}

		if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex)
			&& state.hoveredGroundPosition
			&& (mapData.placedModels[*state.selectedPlacedModelIndex].type == PlaceableModelType::Road))
		{
			const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
         if (HitTestRoadRotationHandle(placedModel, state.hoveredGroundPosition))
			{
				state.roadRotateDrag = RoadRotateDragState{
					.placedModelIndex = *state.selectedPlacedModelIndex,
				};
				SetStatusMessage(state, U"Road をドラッグして回転");
				return;
			}

			if (const auto cornerIndex = HitTestRoadCornerHandle(placedModel, state.hoveredGroundPosition))
			{
				const Array<Vec3> corners = GetRoadCorners(placedModel);
				state.roadResizeDrag = RoadResizeDragState{
					.placedModelIndex = *state.selectedPlacedModelIndex,
					.draggedCornerIndex = *cornerIndex,
					.fixedCornerPosition = corners[static_cast<size_t>((*cornerIndex + 2) % static_cast<int32>(corners.size()))],
				};
				SetStatusMessage(state, U"Road の角をドラッグして引き延ばし");
				return;
			}
		}

		if (const auto selectedIndex = HitTestNavPoint(mapData.navPoints, camera))
		{
			state.selectedNavPointIndex = *selectedIndex;
			state.selectedPlacedModelIndex.reset();
			state.selectedResourceAreaIndex.reset();
			SetStatusMessage(state, U"NavPoint {} を選択"_fmt(*selectedIndex));
			return;
		}

		if (const auto selectedIndex = HitTestPlacedModel(mapData.placedModels, camera, state.hoveredGroundPosition))
		{
			state.selectedPlacedModelIndex = *selectedIndex;
			state.selectedResourceAreaIndex.reset();
			state.selectedNavPointIndex.reset();
			SetStatusMessage(state, U"{} を選択"_fmt(ToString(mapData.placedModels[*selectedIndex].type)));
			return;
		}

		if (const auto selectedIndex = HitTestResourceArea(mapData.resourceAreas, state.hoveredGroundPosition))
		{
			state.selectedResourceAreaIndex = *selectedIndex;
			state.selectedPlacedModelIndex.reset();
		   state.selectedNavPointIndex.reset();
			SetStatusMessage(state, U"{} を選択"_fmt(ToString(mapData.resourceAreas[*selectedIndex].type)));
			return;
		}

		state.selectedPlacedModelIndex.reset();
	 state.selectedResourceAreaIndex.reset();
		state.selectedNavPointIndex.reset();
     state.pendingRoadPlacementStartPosition.reset();
	   state.pendingWallPlacementStartPosition.reset();
		SetStatusMessage(state, U"選択解除");
		return;
	}

	if ((ToTerrainCellType(state.selectedTool) || (state.selectedTool == MapEditorTool::EraseTerrain))
		&& state.hoveredGroundPosition && MouseL.pressed())
	{
		const Point cell = ToTerrainCell(*state.hoveredGroundPosition);
		if (state.lastTerrainPaintCell && (*state.lastTerrainPaintCell == cell))
		{
			return;
		}

		state.lastTerrainPaintCell = cell;
		if (const auto terrainType = ToTerrainCellType(state.selectedTool))
		{
			SetTerrainCell(mapData.terrainCells, cell, *terrainType, state.selectedTerrainColor);
			SetStatusMessage(state, U"{} を塗布"_fmt(ToLabel(state.selectedTool)));
		}
		else if (RemoveTerrainCell(mapData.terrainCells, cell))
		{
			SetStatusMessage(state, U"地表セルを削除");
		}

		return;
	}

   if (state.selectedTool == MapEditorTool::PlaceRoad)
	{
        if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingRoadPlacementStartPosition = *state.hoveredGroundPosition;
			return;
		}

		if (state.pendingRoadPlacementStartPosition && MouseL.up())
		{
			const Vec3 roadPosition = *state.pendingRoadPlacementStartPosition;
			const Vec3 roadDirectionTarget = state.hoveredGroundPosition.value_or(roadPosition);
            mapData.placedModels << BuildRoadFromStartAndEnd(roadPosition, roadDirectionTarget, 8.0, 4.0, 0.0);
			state.pendingRoadPlacementStartPosition.reset();
			state.selectedPlacedModelIndex = (mapData.placedModels.size() - 1);
			state.selectedResourceAreaIndex.reset();
			state.selectedNavPointIndex.reset();
			SetStatusMessage(state, U"Road を配置");
			return;
		}

		return;
	}

	if (state.selectedTool == MapEditorTool::PlaceWall)
	{
		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingWallPlacementStartPosition = *state.hoveredGroundPosition;
			return;
		}

		if (state.pendingWallPlacementStartPosition && MouseL.up())
		{
			const Vec3 wallPosition = *state.pendingWallPlacementStartPosition;
			const Vec3 wallEndPosition = state.hoveredGroundPosition.value_or(wallPosition);
          mapData.placedModels << BuildWallFromStartAndEnd(wallPosition, wallEndPosition, 10.0, 0.0);
			state.pendingWallPlacementStartPosition.reset();
			SetStatusMessage(state, U"Wall を配置");
			return;
		}

		return;
	}

	if (not state.hoveredGroundPosition || not MouseL.down())
	{
		return;
	}

	const Vec3 position = *state.hoveredGroundPosition;

	if (state.selectedTool == MapEditorTool::SetPlayerBasePosition)
	{
		mapData.playerBasePosition = position;
		SetStatusMessage(state, U"自軍拠点を更新");
		return;
	}

	if (state.selectedTool == MapEditorTool::SetEnemyBasePosition)
	{
		mapData.enemyBasePosition = position;
		SetStatusMessage(state, U"敵拠点を更新");
		return;
	}

	if (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
	{
		mapData.sapperRallyPoint = position;
		SetStatusMessage(state, U"集結位置を更新");
		return;
	}

	if (const auto resourceType = ToResourceType(state.selectedTool))
	{
     mapData.resourceAreas << ResourceArea{ .type = *resourceType, .position = position, .radius = MainSupport::ResourceAreaDefaultRadius };
		SetStatusMessage(state, U"{}を追加"_fmt(ToLabel(state.selectedTool)));
		return;
	}

	if (state.selectedTool == MapEditorTool::PlaceNavPoint)
	{
		mapData.navPoints << NavPoint{ .position = position, .radius = 1.4 };
		state.selectedNavPointIndex = (mapData.navPoints.size() - 1);
		state.selectedPlacedModelIndex.reset();
		state.selectedResourceAreaIndex.reset();
		SetStatusMessage(state, U"NavPoint を追加");
		return;
	}

	if (state.selectedTool == MapEditorTool::LinkNavPoints)
	{
		if (const auto hitNavPointIndex = HitTestNavPoint(mapData.navPoints, camera))
		{
			if (not state.pendingNavLinkStartIndex)
			{
				state.pendingNavLinkStartIndex = *hitNavPointIndex;
				SetStatusMessage(state, U"NavLink 開始点を選択");
				return;
			}

			if (*state.pendingNavLinkStartIndex == *hitNavPointIndex)
			{
				state.pendingNavLinkStartIndex.reset();
				SetStatusMessage(state, U"NavLink 選択を解除");
				return;
			}

			const bool added = ToggleNavLink(mapData, *state.pendingNavLinkStartIndex, *hitNavPointIndex);
			state.pendingNavLinkStartIndex.reset();
			SetStatusMessage(state, added ? U"NavLink を追加" : U"NavLink を削除");
			return;
		}

		state.pendingNavLinkStartIndex.reset();
		return;
	}

	if (const auto modelType = ToPlaceableModelType(state.selectedTool))
	{
        mapData.placedModels << PlacedModel{ .type = *modelType, .position = position, .yaw = 0.0, .wallLength = 10.0 };
		SetStatusMessage(state, U"モデルを配置");
	}
}
