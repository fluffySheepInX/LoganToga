# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Addons/Pi3D/Pi3D.hpp"

void Main()
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
#pragma endregion

	Pi3D::RegisterAddon();

	const Mesh groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) };
	const Texture groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB };

	const Model blacksmithModel{ U"example/obj/blacksmith.obj" };
	const Model millModel{ U"example/obj/mill.obj" };
	const Model treeModel{ U"example/obj/tree.obj" };
	const Model pineModel{ U"example/obj/pine.obj" };
	const Model siv3dkunModel{ U"example/obj/siv3d-kun.obj" };

	Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(siv3dkunModel, TextureDesc::MippedSRGB);

	DebugCamera3D camera{ Scene::Size(), 40_deg, Vec3{ 0, 3, -16 } };
	const auto drawScene = [&]()
	{
		Pi3D::EnvironmentRef().drawGround(groundPlane, groundTexture);
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());
		blacksmithModel.draw(Vec3{ 8, 0, 4 });
		millModel.draw(Vec3{ -8, 0, 4 });

		{
			const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(Vec3{ 16, 0, 4 });
			pineModel.draw(Vec3{ 16, 0, 0 });
		}

		siv3dkunModel.draw(Vec3{ 2, 0, -2 }, Quaternion::RotateY(180_deg));
		Pi3D::EnvironmentRef().draw3D();
	};

	while (System::Update())
	{
		camera.update(4.0);
		Graphics3D::SetCameraTransform(camera);

#pragma region Addon
		Pi3D::Update();
		Pi3D::Begin3D(drawScene);
		Pi3D::End3D(drawScene);
		Pi3D::DrawUI();

		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion
	}
}


