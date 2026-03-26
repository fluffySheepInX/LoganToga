# include "AppBootstrap.h"
# include "AppState.h"
# include "FormationScene.h"
# include "GameScene.h"
# include "TitleScene.h"

void Main()
{
	ConfigureAddons();

	App manager;
	manager.add<TitleScene>(U"Title");
	manager.add<FormationScene>(U"Formation");
	manager.add<GameScene>(U"Game");

	while (System::Update())
	{
		if (UpdateAddons())
		{
			break;
		}

		if (not manager.update())
		{
			break;
		}
	}
}
