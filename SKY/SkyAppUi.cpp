# include "SkyAppLoopInternal.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	void DrawHudUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
     if (HandleEscMenu(resources, state, frame))
			{
				return;
			}

			DrawSettingsHud(resources, state, frame);
			DrawContextHud(state, frame);
			DrawHudModeToggles(state, frame);
			DrawHudFooter(state, frame);
	}
}
