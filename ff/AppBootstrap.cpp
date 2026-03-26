# include "AppBootstrap.h"
# include <Siv3D.hpp>
# include "libs/AddonEffFs.h"
# include "libs/AddonGaussian.h"

void ConfigureAddons()
{
#pragma region Addon
	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,900 });
	GaussianFSAddon::SetLangSet({
		{ U"Japan",     U"日本語" },
		{ U"English",   U"English" },
		{ U"Deutsch",   U"Deutsch" },
		{ U"Test",      U"TestLang" },
		});
	GaussianFSAddon::SetLang(U"Japan");
	GaussianFSAddon::SetSceneSet({
		{ U"1600*900", U"1600",U"900"},
		{ U"1200*600", U"1200",U"600"},
		});
	GaussianFSAddon::SetScene(U"1600*900");

	Addon::Register<AddonEffFs>(U"AddonEffFs");
	AddonEffFs::SetKind(AddonEffFs::EffectKind::Wind);
	AddonEffFs::SetColor(ColorF(0.9, 0.95, 1.0, 0.8));
	AddonEffFs::SetSpawnDensity(0.05, 0.25);
	//AddonEffFs::SetDirection(Vec2{ 0.5, 1.0 });       // 斜めの豪雨
	//AddonEffFs::AddImpulse(1.0, 5.0);                 // 強雨タイム
#pragma endregion
}

bool UpdateAddons()
{
#pragma region Addon
	if (GaussianFSAddon::TriggerOrDisplayLang()) return true;
	if (GaussianFSAddon::TriggerOrDisplaySceneSize()) return true;
	if (GaussianFSAddon::IsHide()) Window::Minimize();
	if (GaussianFSAddon::IsGameEnd()) return true;
	GaussianFSAddon::DragProcessWindow();
	AddonEffFs::SetDeltaTime(Scene::DeltaTime());
#pragma endregion

	return false;
}
