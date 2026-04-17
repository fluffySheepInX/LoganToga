# include "SkyAppRenderOverlayInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	void DrawOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		Graphics3D::Flush();
		resources.renderTexture.resolve();
		Shader::LinearToScreen(resources.renderTexture);

      OverlayDetail::DrawResourceLoadWarnings(resources);
      OverlayDetail::DrawUiEditGridOverlay(state);
		OverlayDetail::DrawGroundContactOverlays(resources, state, frame);
		OverlayDetail::DrawBattleOverlays(state, frame);
		OverlayDetail::DrawResourceAreaOverlays(state);
		OverlayDetail::DrawResourcePanelOverlay(state, frame);
       OverlayDetail::DrawBaseStatusOverlays(state, frame);
        OverlayDetail::DrawModelHeightOverlay(resources, state, frame);
		OverlayDetail::DrawSelectionDragOverlay(state, frame);
		OverlayDetail::DrawMatchResultOverlay(state);
	}
}
