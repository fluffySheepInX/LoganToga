# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	void DrawSettingsHud(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.showUI)
		{
			return;
		}

		DrawSkySettingsPanel(state.sky, state.skyTime, state.skySettingsExpanded, frame.panels);

		DrawCameraSettingsPanel(state.camera,
			state.cameraSettings,
			state.cameraSettingsExpanded,
			resources.GetUnitRenderModel(UnitRenderModel::Bird),
			resources.GetUnitRenderModel(UnitRenderModel::Ashigaru),
			state.cameraSaveMessage,
			frame.panels);
		DrawTerrainVisualSettingsPanel(state.terrainVisualSettings, state.uiEditMode, state.terrainVisualSettingsExpanded, frame.panels);
	}
}
