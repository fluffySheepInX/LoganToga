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

	namespace
	{
		void ResizeBattleWindow(SkyAppResources& resources, SkyAppState& state, const Size& size)
		{
			Window::Resize(size);
			resources.renderTexture = MSRenderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
			state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.camera.getEyePosition(), state.camera.getFocusPosition() };
		}
	}

	bool HandleEscMenu(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.showEscMenu)
		{
			return false;
		}

		switch (DrawEscMenu(frame.panels.escMenu))
		{
		case EscMenuAction::Restart:
			ResetMatch(state);
			state.restartMessage.show(U"試合をリスタート");
			state.showEscMenu = false;
			break;

		case EscMenuAction::Title:
			state.requestTitleScene = true;
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1280x720:
			ResizeBattleWindow(resources, state, Size{ 1280, 720 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1600x900:
			ResizeBattleWindow(resources, state, Size{ 1600, 900 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1920x1080:
			ResizeBattleWindow(resources, state, Size{ 1920, 1080 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::None:
		default:
			break;
		}

		return true;
	}
}
