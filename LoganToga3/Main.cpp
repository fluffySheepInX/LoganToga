# include <Siv3D.hpp> // Siv3D v0.6.16
# include "App/AppFrameEnd.h"
# include "App/AppInitialization.h"
# include "App/AppSceneSharedData.h"
# include "Data/MusicSettings.h"
# include "Scenes/TitleScene.h"
# include "Scenes/BattleScene.h"

void Main()
{
    LT3::InitializeGaussianAddon();
	auto shared = std::make_shared<LT3::AppSharedData>();
	const int32 argc = System::GetArgc();
	char** argv = System::GetArgv();
	for (int32 i = 1; i < argc; ++i)
	{
		const String arg = Unicode::Widen(argv[i]);
		if (arg == U"--mod")
		{
			shared->modMode = true;
			continue;
		}

		if (arg == U"--quick-battle")
		{
			shared->quickBattleRequested = true;
			if ((i + 1) < argc)
			{
				shared->quickBattleArgument = Unicode::Widen(argv[i + 1]);
				++i;
			}
			continue;
		}
	}

	const bool skipTitleToBattle = shared->quickBattleRequested;
	Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
	LT3::LoadMusicSettingsToml(shared->musicSettings, shared->musicEditor.statusText);
	LT3::AppSceneManager manager{ shared };
	manager.add<LT3::TitleScene>(LT3::AppSceneState::Title);
	manager.add<LT3::BattleScene>(LT3::AppSceneState::Battle);
	manager.init(skipTitleToBattle ? LT3::AppSceneState::Battle : LT3::AppSceneState::Title, 0);

	while (System::Update())
	{
		{
			const double scale = GaussianFSAddon::GetSCALE();
			const Vec2 offset = GaussianFSAddon::GetOFFSET();
			const Transformer2D screenScaling{ Mat3x2::Scale(scale).translated(offset), TransformCursor::Yes };

			if (!manager.update())
			{
				break;
			}
		}

        if (!LT3::ProcessGaussianAddonFrameEnd()) break;


	}
}
