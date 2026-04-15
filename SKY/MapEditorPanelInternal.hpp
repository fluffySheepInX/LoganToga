# pragma once
# include "MapEditorInternal.hpp"

namespace MapEditorDetail
{
	[[nodiscard]] const Array<Color>& TerrainColorPalette();
	[[nodiscard]] bool IsTerrainPaintTool(MapEditorTool tool);
	[[nodiscard]] StringView ToLabel(MapEditorToolCategory category);
	[[nodiscard]] MapEditorToolCategory ToCategory(MapEditorTool tool);
	[[nodiscard]] Array<MapEditorTool> GetToolsForCategory(MapEditorToolCategory category);
	[[nodiscard]] StringView GetOperationHint(const MapEditorState& state);
	void ResetToolInteractionState(MapEditorState& state);
	void DrawMapEditorPanelSection(const Rect& rect);
  void DrawMillPlacementOwnerSelector(MapEditorState& state, const Rect& rect, const Font& font);
	void DrawMapEditorToolSection(MapEditorState& state, const Rect& panelRect, const Font& font);
	void DrawMapEditorCommandSection(MapEditorState& state, MapData& mapData, FilePathView path, const Rect& panelRect);
	void DrawMapEditorInfoSection(const MapEditorState& state, const MapData& mapData, const Rect& panelRect, const Font& font);
	void DrawMapEditorSelectionDetailSection(MapEditorState& state, MapData& mapData, const Rect& panelRect, const Font& font);
	void DrawMapEditorStatusMessage(const MapEditorState& state, const Rect& panelRect, const Font& font);
}
