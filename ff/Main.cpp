# include "AppBootstrap.h"
# include "AppState.h"
# include "DefinitionRepository.h"
# include "FormationScene.h"
# include "GameScene.h"
# include "TitleScene.h"
# include "UnitEditorScene.h"
# include "WaveEditorScene.h"

void Main()
{
	ConfigureAddons();

	ff::DefinitionRepository definitionRepository;
	ff::RegisterDefinitionRepository(definitionRepository);

	App manager;
  *manager.get() = LoadAppDataFromDisk();
	manager.add<TitleScene>(U"Title");
	manager.add<FormationScene>(U"Formation");
    manager.add<UnitEditorScene>(U"UnitEditor");
    manager.add<WaveEditorScene>(U"WaveEditor");
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

	ff::UnregisterDefinitionRepository(definitionRepository);
}
