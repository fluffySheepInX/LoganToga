# include "MapEditorInternal.hpp"
# include "MapEditorUpdateInternal.hpp"
# include "MapEditorPanelInternal.hpp"
# include "MapEditorSceneInternal.hpp"
# include "SkyAppUiInternal.hpp"

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

void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, const FilePathView path, const Rect& panelRect)
{
	using namespace MapEditorDetail;
	static const Font font{ 16 };
	SkyAppSupport::UiInternal::DrawNinePatchPanelFrame(panelRect, U"Map Editor", ColorF{ 0.98, 0.95 }, SkyAppSupport::UiInternal::DefaultPanelFrameColor, SkyAppSupport::UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::MapEditor);
	DrawMapEditorToolSection(state, panelRect, font);
	DrawMapEditorCommandSection(state, mapData, path, panelRect);
	DrawMapEditorInfoSection(state, mapData, panelRect, font);
	DrawMapEditorSelectionDetailSection(state, mapData, panelRect, font);
	DrawMapEditorStatusMessage(state, panelRect, font);
}

void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData)
{
	using namespace MapEditorSceneDetail;
	if (not state.enabled)
	{
		return;
	}

	for (const auto& terrainCell : mapData.terrainCells)
	{
		DrawTerrainCell(terrainCell);
	}

	DrawNavLinks(state, mapData);
	DrawNavPoints(state, mapData);
	DrawBaseMarkers(mapData);
	DrawResourceAreas(mapData);
	DrawSelectedPlacedModelHighlight(state, mapData);
	DrawSelectedResourceAreaHighlight(state, mapData);

	if (not state.hoveredGroundPosition)
	{
		return;
	}

	const Vec3 hoverPosition = *state.hoveredGroundPosition;
	if (state.selectionMode)
	{
		DrawSelectionModePreview(state, mapData, hoverPosition);
		return;
	}

	DrawToolHoverPreview(state, mapData, hoverPosition);
}
