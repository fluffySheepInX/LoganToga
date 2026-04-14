# include "MapEditorUpdateInternal.hpp"

namespace MapEditorUpdateDetail
{
	bool HandleRoadPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceRoad)
		{
			return false;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingRoadPlacementStartPosition = *state.hoveredGroundPosition;
			return true;
		}

		if (state.pendingRoadPlacementStartPosition && MouseL.up())
		{
			const Vec3 roadPosition = *state.pendingRoadPlacementStartPosition;
			const Vec3 roadDirectionTarget = state.hoveredGroundPosition.value_or(roadPosition);
			mapData.placedModels << MapEditorDetail::BuildRoadFromStartAndEnd(roadPosition, roadDirectionTarget, 8.0, 4.0, 0.0);
			state.pendingRoadPlacementStartPosition.reset();
			SelectPlacedModel(state, mapData.placedModels.size() - 1);
			MapEditorDetail::SetStatusMessage(state, U"Road を配置");
			return true;
		}

		return true;
	}

	bool HandleTireTrackPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceTireTrackDecal)
		{
			return false;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingTireTrackPlacementStartPosition = *state.hoveredGroundPosition;
			return true;
		}

		if (state.pendingTireTrackPlacementStartPosition && MouseL.up())
		{
			const Vec3 decalStart = *state.pendingTireTrackPlacementStartPosition;
			const Vec3 decalEnd = state.hoveredGroundPosition.value_or(decalStart);
			mapData.placedModels << MapEditorDetail::BuildTireTrackDecalFromStartAndEnd(decalStart, decalEnd, 6.0, 2.0, 0.0);
			state.pendingTireTrackPlacementStartPosition.reset();
			SelectPlacedModel(state, mapData.placedModels.size() - 1);
			MapEditorDetail::SetStatusMessage(state, U"タイヤ跡デカールを配置");
			return true;
		}

		return true;
	}

	bool HandleWallPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceWall)
		{
			return false;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingWallPlacementStartPosition = *state.hoveredGroundPosition;
			return true;
		}

		if (state.pendingWallPlacementStartPosition && MouseL.up())
		{
			const Vec3 wallPosition = *state.pendingWallPlacementStartPosition;
			const Vec3 wallEndPosition = state.hoveredGroundPosition.value_or(wallPosition);
			mapData.placedModels << MapEditorDetail::BuildWallFromStartAndEnd(wallPosition, wallEndPosition, 10.0, 0.0);
			state.pendingWallPlacementStartPosition.reset();
			MapEditorDetail::SetStatusMessage(state, U"Wall を配置");
			return true;
		}

		return true;
	}

	void HandleGroundPlacement(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera)
	{
		if (not state.hoveredGroundPosition || not MouseL.down())
		{
			return;
		}

		const Vec3 position = *state.hoveredGroundPosition;

		if (state.selectedTool == MapEditorTool::SetPlayerBasePosition)
		{
			mapData.playerBasePosition = position;
			MapEditorDetail::SetStatusMessage(state, U"自軍拠点を更新");
			return;
		}

		if (state.selectedTool == MapEditorTool::SetEnemyBasePosition)
		{
			mapData.enemyBasePosition = position;
			MapEditorDetail::SetStatusMessage(state, U"敵拠点を更新");
			return;
		}

		if (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
		{
			mapData.sapperRallyPoint = position;
			MapEditorDetail::SetStatusMessage(state, U"集結位置を更新");
			return;
		}

		if (const auto resourceType = MapEditorDetail::ToResourceType(state.selectedTool))
		{
			mapData.resourceAreas << ResourceArea{ .type = *resourceType, .position = position, .radius = MainSupport::ResourceAreaDefaultRadius };
			MapEditorDetail::SetStatusMessage(state, U"{}を追加"_fmt(MapEditorDetail::ToLabel(state.selectedTool)));
			return;
		}

		if (state.selectedTool == MapEditorTool::PlaceNavPoint)
		{
			mapData.navPoints << NavPoint{ .position = position, .radius = 1.4 };
			SelectNavPoint(state, mapData.navPoints.size() - 1);
			MapEditorDetail::SetStatusMessage(state, U"NavPoint を追加");
			return;
		}

		if (state.selectedTool == MapEditorTool::LinkNavPoints)
		{
			if (const auto hitNavPointIndex = MapEditorDetail::HitTestNavPoint(mapData.navPoints, camera))
			{
				if (not state.pendingNavLinkStartIndex)
				{
					state.pendingNavLinkStartIndex = *hitNavPointIndex;
					MapEditorDetail::SetStatusMessage(state, U"NavLink 開始点を選択");
					return;
				}

				if (*state.pendingNavLinkStartIndex == *hitNavPointIndex)
				{
					state.pendingNavLinkStartIndex.reset();
					MapEditorDetail::SetStatusMessage(state, U"NavLink 選択を解除");
					return;
				}

				const bool added = MapEditorDetail::ToggleNavLink(mapData, *state.pendingNavLinkStartIndex, *hitNavPointIndex);
				state.pendingNavLinkStartIndex.reset();
				MapEditorDetail::SetStatusMessage(state, added ? U"NavLink を追加" : U"NavLink を削除");
				return;
			}

			state.pendingNavLinkStartIndex.reset();
			return;
		}

		if (const auto modelType = MapEditorDetail::ToPlaceableModelType(state.selectedTool))
		{
			mapData.placedModels << PlacedModel{ .type = *modelType, .position = position, .yaw = 0.0, .wallLength = 10.0 };
			MapEditorDetail::SetStatusMessage(state, U"モデルを配置");
		}
	}
}
