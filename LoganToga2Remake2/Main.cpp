#include "GameData.h"
#include "BattleScene.h"
#include "TitleScene.h"

void Main()
{
	Window::Resize(1280, 720);
	Window::SetTitle(U"LoganToga2Remake2");
	Scene::SetBackground(ColorF{ 0.11, 0.13, 0.16 });
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	App manager;
	manager.add<TitleScene>(U"Title");
	manager.add<BattleScene>(U"Battle");

	while (System::Update())
	{
		if (!manager.update())
		{
			break;
		}
	}
}
