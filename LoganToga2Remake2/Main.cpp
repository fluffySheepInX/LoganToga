#include "GameData.h"
#include "BattleScene.h"
#include "BalanceEditScene.h"
#include "BonusRoomScene.h"
#include "GameSettings.h"
#include "MapEditScene.h"
#include "RewardScene.h"
#include "TitleScene.h"
#include "WindowChromeAddon.h"

void Main()
{
	const PersistentGameSettings settings = GameSettings::GetGameSettings();
	ApplyDisplaySettings(settings.displaySettings);
	Window::SetTitle(U"LoganToga2Remake2");
	s3d::Addon::Register<WindowChromeAddon>(WindowChromeAddon::AddonName);
	WindowChromeAddon::Configure(U"LoganToga2Remake2");
	s3d::GlobalAudio::SetVolume(settings.masterVolume);
	s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::BgmBus, settings.bgmVolume);
	s3d::GlobalAudio::BusSetVolume(WindowChromeAddon::SeBus, settings.seVolume);
	Window::SetFullscreen(settings.fullscreen);
	//Scene::SetBackground(ColorF{ 0.11, 0.13, 0.16 });
	Scene::SetBackground(ColorF{ U"#011B05" });
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	App manager;
	manager.get()->displaySettings = settings.displaySettings;
	manager.add<TitleScene>(U"Title");
	manager.add<BattleScene>(U"Battle");
	manager.add<BalanceEditScene>(U"BalanceEdit");
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
