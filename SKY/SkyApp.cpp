# include <Siv3D.hpp>
# include "SkyApp.hpp"
# include "SkyAppLoop.hpp"

void RunSkyApp()
{
	Window::Resize(1280, 720);
  System::SetTerminationTriggers(UserAction::CloseButtonClicked);
	SkyAppFlow::SkyAppResources resources;
	SkyAppFlow::SkyAppState state;
	SkyAppFlow::InitializeSkyAppState(state);

	while (System::Update())
	{
		SkyAppFlow::RunSkyAppFrame(resources, state);
	}
}
