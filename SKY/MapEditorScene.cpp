# include "MapEditorSceneInternal.hpp"

using namespace MapEditorSceneDetail;

void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData)
{
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
