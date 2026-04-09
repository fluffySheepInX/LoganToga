# include "SkyApp.hpp"
# include "SkyAppInternal.hpp"

void RunSkyApp()
{
	Window::Resize(1280, 720);
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

    SkyAppInternal::App manager;
	SkyAppInternal::AddTitleScene(manager);
	SkyAppInternal::AddCampaignEditorScene(manager);
	SkyAppInternal::AddBattleScene(manager);
	manager.init(U"Battle", 0);

	while (System::Update())
	{
		if (not manager.update())
		{
			break;
		}
	}
}
