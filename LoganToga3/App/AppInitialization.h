#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"
# include "../Data/MusicSettings.h"
# include "../Systems/EditorInputSystem.h"
# include "App/AppStateData.h"
# include "App/AppSceneSharedData.h"

namespace LT3
{
	inline void InitializeGaussianAddon()
	{
		Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
		GaussianFSAddon::Condition({ 1600, 900 });
		GaussianFSAddon::SetLangSet({
			{ U"Japan",     U"日本語" },
			{ U"English",   U"English" },
			{ U"Deutsch",   U"Deutsch" },
			{ U"Test",      U"TestLang" },
			});
		GaussianFSAddon::SetLang(U"Japan");
		GaussianFSAddon::SetSceneSet({
			{ U"1600*900", U"1600", U"900" },
			{ U"1200*600", U"1200", U"600" },
			});
		GaussianFSAddon::SetScene(U"1600*900");
	}

	inline void InitializeApp(AppState& app)
	{
		Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
		InitializeAppUiState(app.ui, false);
		InitializeAppRuntimeState(app.runtime, app.definitions);
		SyncBattleWorldMapFromEditor(app.ui.mapEditor, app.runtime.world, app.definitions.defs);
	}

	inline void InitializeAppSharedData(AppSharedData& data)
	{
		Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
		LoadMusicSettingsToml(data.musicSettings, data.musicEditor.statusText);
	}
}
