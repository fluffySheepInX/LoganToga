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
  PaintGrass,
	PaintDirt,
	PaintSand,
	PaintRock,
	EraseTerrain,
	PlaceMill,
	PlaceTree,
	PlacePine,
  PlaceGrassPatch,
    PlaceRock,
  PlaceWall,
  PlaceRoad,
	PlaceNavPoint,
	LinkNavPoints,
};

struct RoadResizeDragState
{
	size_t placedModelIndex = 0;
	int32 draggedCornerIndex = 0;
	Vec3 fixedCornerPosition{ 0, 0, 0 };
};

struct RoadRotateDragState
{
	size_t placedModelIndex = 0;
};

struct MapEditorState
{
	bool enabled = false;
	MapEditorTool selectedTool = MapEditorTool::SetSapperRallyPoint;
   bool selectionMode = false;
  bool showNavPoints = true;
	bool showNavLinks = true;
	Optional<size_t> selectedPlacedModelIndex;
   Optional<size_t> selectedResourceAreaIndex;
   Optional<size_t> selectedNavPointIndex;
	Optional<size_t> pendingNavLinkStartIndex;
   Optional<Vec3> pendingWallPlacementStartPosition;
    Optional<Vec3> pendingRoadPlacementStartPosition;
   Optional<RoadResizeDragState> roadResizeDrag;
   Optional<RoadRotateDragState> roadRotateDrag;
	Optional<Vec3> hoveredGroundPosition;
   Optional<Point> lastTerrainPaintCell;
	Color selectedTerrainColor{ 255, 255, 255 };
	String statusMessage;
	double statusMessageUntil = 0.0;
};

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, bool canHandleSceneInput);
void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData);
void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, FilePathView path, const Rect& panelRect);
