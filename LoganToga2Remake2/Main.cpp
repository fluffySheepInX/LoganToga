#include "GameData.h"
#include "BattleScene.h"
#include "BonusRoomScene.h"
#include "MapEditScene.h"
#include "RewardScene.h"
#include "TitleScene.h"

void Main()
{
	ApplyDisplaySettings(DisplaySettings{});
	Window::SetTitle(U"LoganToga2Remake2");
	Scene::SetBackground(ColorF{ 0.11, 0.13, 0.16 });
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	App manager;
	manager.add<TitleScene>(U"Title");
	manager.add<BattleScene>(U"Battle");
	manager.add<BonusRoomScene>(U"BonusRoom");
	manager.add<MapEditScene>(U"MapEdit");
	manager.add<RewardScene>(U"Reward");

	while (System::Update())
	{
		if (!manager.update())
		{
			break;
		}
	}
}
