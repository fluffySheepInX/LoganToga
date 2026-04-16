# include "MapEditorPanelInternal.hpp"
# include "SkyAppUiInternal.hpp"

using namespace MapEditorDetail;

void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, const FilePathView path, const Rect& panelRect)
{
	static const Font font{ 16 };
 SkyAppSupport::UiInternal::DrawNinePatchPanelFrame(panelRect, U"Map Editor", ColorF{ 0.98, 0.95 }, SkyAppSupport::UiInternal::DefaultPanelFrameColor, SkyAppSupport::UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::MapEditor);
	DrawMapEditorToolSection(state, panelRect, font);
	DrawMapEditorCommandSection(state, mapData, path, panelRect);
	DrawMapEditorInfoSection(state, mapData, panelRect, font);
	DrawMapEditorSelectionDetailSection(state, mapData, panelRect, font);
	DrawMapEditorStatusMessage(state, panelRect, font);
}
