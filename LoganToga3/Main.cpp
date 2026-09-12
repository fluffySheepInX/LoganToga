# include <Siv3D.hpp> // Siv3D v0.6.16
# include "App/AppFrameEnd.h"
# include "App/AppInitialization.h"
# include "App/AppSceneSharedData.h"
# include "Data/Localization.h"
# include "Data/MusicSettings.h"
# include "Data/ModPolicy.h"
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
			if (((i + 1) < argc) && !String{ Unicode::Widen(argv[i + 1]) }.starts_with(U"--"))
			{
				shared->requestedModId = Unicode::Widen(argv[i + 1]);
				++i;
			}
			continue;
		}

		if (arg == U"--quick-battle")
		{
			shared->quickBattleRequested = true;
			if (((i + 1) < argc) && !String{ Unicode::Widen(argv[i + 1]) }.starts_with(U"--"))
			{
				shared->quickBattleArgument = Unicode::Widen(argv[i + 1]);
				++i;
			}
			continue;
		}
	}

	const String requestedModId = !shared->requestedModId.isEmpty()
		? shared->requestedModId
		: U"000-default-game";
	if (!LT3::TryLoadModContext(requestedModId, shared->activeMod, shared->startupErrorText))
	{
		shared->quickBattleRequested = false;
	}
	else
	{
		shared->definitions = LT3::CreateAppDefinitionState(shared->activeMod);
	}
	if (shared->quickBattleRequested && shared->startupErrorText.isEmpty())
	{
		const LT3::ModPolicy::QuickBattleTarget target = LT3::ModPolicy::ParseQuickBattleTarget(
			LT3::ModPolicy::TextView{ shared->quickBattleArgument.data(), shared->quickBattleArgument.size() });
		if (target.kind != LT3::ModPolicy::QuickBattleTargetKind::Skirmish)
		{
			shared->startupErrorText = U"Unsupported quick battle target: {}"_fmt(shared->quickBattleArgument);
			shared->quickBattleRequested = false;
		}
		else if (!LT3::TryLoadSkirmishBattleRequest(shared->activeMod,
			String{ target.battleId.data, target.battleId.size }, shared->quickBattleRequest, shared->startupErrorText))
		{
			shared->quickBattleRequested = false;
		}
	}

	const bool skipTitleToBattle = shared->quickBattleRequested && shared->quickBattleRequest.valid;
	Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
	LT3::LoadLocalizationCatalogs(LT3::DefaultLocale, shared->localizedTexts, shared->defaultTexts);
	LT3::LoadMusicSettingsToml(shared->musicSettings, shared->musicEditor.statusText);
	LT3::AppSceneManager manager{ shared };
	manager.add<LT3::TitleScene>(LT3::AppSceneState::Title);
	manager.add<LT3::BattleScene>(LT3::AppSceneState::Battle);
	manager.init(skipTitleToBattle ? LT3::AppSceneState::Battle : LT3::AppSceneState::Title, 0);

	while (System::Update())
	{
		if (!manager.update())
		{
			break;
		}

        if (!LT3::ProcessGaussianAddonFrameEnd()) break;


	}
}
