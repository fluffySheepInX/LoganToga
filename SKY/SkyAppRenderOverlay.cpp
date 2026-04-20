# include "SkyAppRenderOverlayInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		// Each overlay pass shares the same signature so the dispatch
		// list is just an array of function pointers. Adding a new
		// overlay = add an entry here.
		using OverlayPassFn = void(*)(SkyAppResources&, SkyAppState&, const SkyAppFrameState&);

		void Pass_DrawResourceLoadWarnings(SkyAppResources& r, SkyAppState&, const SkyAppFrameState&)         { OverlayDetail::DrawResourceLoadWarnings(r); }
		void Pass_DrawUiEditGridOverlay   (SkyAppResources&, SkyAppState& s, const SkyAppFrameState&)         { OverlayDetail::DrawUiEditGridOverlay(s); }
		void Pass_DrawGroundContactOverlays(SkyAppResources& r, SkyAppState& s, const SkyAppFrameState& f)    { OverlayDetail::DrawGroundContactOverlays(r, s, f); }
		void Pass_DrawBattleOverlays      (SkyAppResources&, SkyAppState& s, const SkyAppFrameState& f)       { OverlayDetail::DrawBattleOverlays(s, f); }
		void Pass_DrawResourceAreaOverlays(SkyAppResources&, SkyAppState& s, const SkyAppFrameState&)         { OverlayDetail::DrawResourceAreaOverlays(s); }
		void Pass_DrawResourcePanelOverlay(SkyAppResources&, SkyAppState& s, const SkyAppFrameState& f)       { OverlayDetail::DrawResourcePanelOverlay(s, f); }
		void Pass_DrawBaseStatusOverlays  (SkyAppResources&, SkyAppState& s, const SkyAppFrameState& f)       { OverlayDetail::DrawBaseStatusOverlays(s, f); }
		void Pass_DrawModelHeightOverlay  (SkyAppResources& r, SkyAppState& s, const SkyAppFrameState& f)     { OverlayDetail::DrawModelHeightOverlay(r, s, f); }
		void Pass_DrawSelectionDragOverlay(SkyAppResources&, SkyAppState& s, const SkyAppFrameState& f)       { OverlayDetail::DrawSelectionDragOverlay(s, f); }
		void Pass_DrawMatchResultOverlay  (SkyAppResources&, SkyAppState& s, const SkyAppFrameState&)         { OverlayDetail::DrawMatchResultOverlay(s); }

		constexpr OverlayPassFn OverlayPasses[] = {
			Pass_DrawResourceLoadWarnings,
			Pass_DrawUiEditGridOverlay,
			Pass_DrawGroundContactOverlays,
			Pass_DrawBattleOverlays,
			Pass_DrawResourceAreaOverlays,
			Pass_DrawResourcePanelOverlay,
			Pass_DrawBaseStatusOverlays,
			Pass_DrawModelHeightOverlay,
			Pass_DrawSelectionDragOverlay,
			Pass_DrawMatchResultOverlay,
		};
	}

	void DrawOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		Graphics3D::Flush();
		resources.renderTexture.resolve();
		Shader::LinearToScreen(resources.renderTexture);

		for (const OverlayPassFn pass : OverlayPasses)
		{
			pass(resources, state, frame);
		}
	}
}
