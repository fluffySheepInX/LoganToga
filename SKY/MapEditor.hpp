# pragma once
# include <Siv3D.hpp>
# include "MapData.hpp"

enum class MapEditorTool
{
    SetPlayerBasePosition,
	SetEnemyBasePosition,
	SetSapperRallyPoint,
  SetBudgetArea,
	SetGunpowderArea,
	SetManaArea,
	PlaceMill,
	PlaceTree,
	PlacePine,
    PlaceRock,
  PlaceWall,
	PlaceNavPoint,
	LinkNavPoints,
};

struct MapEditorState
{
	bool enabled = false;
	MapEditorTool selectedTool = MapEditorTool::SetSapperRallyPoint;
   bool selectionMode = false;
	Optional<size_t> selectedPlacedModelIndex;
   Optional<size_t> selectedResourceAreaIndex;
   Optional<size_t> selectedNavPointIndex;
	Optional<size_t> pendingNavLinkStartIndex;
   Optional<Vec3> pendingWallPlacementStartPosition;
	Optional<Vec3> hoveredGroundPosition;
	String statusMessage;
	double statusMessageUntil = 0.0;
};

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, bool canHandleSceneInput);
void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData);
void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, FilePathView path, const Rect& panelRect);
