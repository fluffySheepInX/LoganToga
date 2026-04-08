# include "MapEditorInternal.hpp"

using namespace MapEditorDetail;

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, const bool canHandleSceneInput)
{
	if (not state.enabled)
	{
		state.hoveredGroundPosition.reset();
		return;
	}

	state.hoveredGroundPosition = GetGroundIntersection(camera);
	if (not IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		state.selectedPlacedModelIndex.reset();
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

	if (state.selectionMode)
	{
		if (not MouseL.down())
		{
			return;
		}

		if (const auto selectedIndex = HitTestNavPoint(mapData.navPoints, camera))
		{
			state.selectedNavPointIndex = *selectedIndex;
			state.selectedPlacedModelIndex.reset();
			state.selectedResourceAreaIndex.reset();
			SetStatusMessage(state, U"NavPoint {} を選択"_fmt(*selectedIndex));
			return;
		}

		if (const auto selectedIndex = HitTestPlacedModel(mapData.placedModels, camera))
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

		if (state.selectedNavPointIndex && state.hoveredGroundPosition)
		{
			NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
			navPoint.position = *state.hoveredGroundPosition;
			SetStatusMessage(state, U"NavPoint {} を移動"_fmt(*state.selectedNavPointIndex));
			return;
		}

		if (state.selectedPlacedModelIndex && state.hoveredGroundPosition)
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			placedModel.position = *state.hoveredGroundPosition;
			SetStatusMessage(state, U"{} を移動"_fmt(ToString(placedModel.type)));
			return;
		}

		if (state.selectedResourceAreaIndex && state.hoveredGroundPosition)
		{
			ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
			resourceArea.position = *state.hoveredGroundPosition;
			SetStatusMessage(state, U"{} を移動"_fmt(ToString(resourceArea.type)));
			return;
		}

		state.selectedPlacedModelIndex.reset();
     state.selectedResourceAreaIndex.reset();
		state.selectedNavPointIndex.reset();
		SetStatusMessage(state, U"選択解除");
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
		mapData.placedModels << PlacedModel{ .type = *modelType, .position = position };
		SetStatusMessage(state, U"モデルを配置");
	}
}
