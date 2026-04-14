# pragma once
# include "MapEditorInternal.hpp"

namespace MapEditorSceneDetail
{
	struct RoadOutlineHighlightState
	{
		Optional<int32> hoveredCorner;
		bool hoveredRotationHandle = false;
	};

	void DrawTerrainCell(const TerrainCell& terrainCell, double yOffset = 0.01, double height = 0.02);
	void DrawTerrainCellPreview(const Point& cell, const ColorF& color);
	void DrawTerrainCellPreviewRange(const Point& startCell, const Point& endCell, const ColorF& color);
	void DrawTireTrackOutline(const PlacedModel& placedModel, const ColorF& outlineColor);
	void DrawResourceAreaRing(const Vec3& position, double radius, const ColorF& ringColor);
	void DrawWallPreview(const PlacedModel& placedModel, const ColorF& wallColor);
	void DrawRoadOutline(const PlacedModel& placedModel, const ColorF& outlineColor, const Optional<int32>& highlightedCorner = none, bool highlightedRotationHandle = false);
	[[nodiscard]] RoadOutlineHighlightState GetRoadOutlineHighlightState(const MapEditorState& state, const PlacedModel& placedModel);
	void DrawNavLinks(const MapEditorState& state, const MapData& mapData);
	void DrawNavPoints(const MapEditorState& state, const MapData& mapData);
	void DrawBaseMarkers(const MapData& mapData);
	void DrawResourceAreas(const MapData& mapData);
	void DrawSelectedPlacedModelHighlight(const MapEditorState& state, const MapData& mapData);
	void DrawSelectedResourceAreaHighlight(const MapEditorState& state, const MapData& mapData);
	void DrawSelectionModePreview(const MapEditorState& state, const MapData& mapData, const Vec3& hoverPosition);
	void DrawToolHoverPreview(const MapEditorState& state, const MapData& mapData, const Vec3& hoverPosition);
}
