# pragma once
# include <Siv3D.hpp>
# include "MapData.hpp"

enum class MapEditorTool
{
	SetSapperRallyPoint,
	PlaceMill,
	PlaceTree,
	PlacePine,
};

struct MapEditorState
{
	bool enabled = false;
	MapEditorTool selectedTool = MapEditorTool::SetSapperRallyPoint;
	Optional<Vec3> hoveredGroundPosition;
	String statusMessage;
	double statusMessageUntil = 0.0;
};

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const DebugCamera3D& camera, bool canHandleSceneInput);
void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData);
void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, FilePathView path, const Rect& panelRect);
