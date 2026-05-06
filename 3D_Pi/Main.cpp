# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Addons/Pi3D/Pi3D.hpp"
# include "Application/CameraController.hpp"
# include "Application/SceneAssets.hpp"
# include "UI/RectUI.hpp"
# include "Road/RoadEditor.hpp"
# include "Ground/TextureEditor.hpp"
# include "Procedural/StairGenerator.hpp"

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

  app::SceneAssets sceneAssets;
	RoadEditor roadEditor{ U"texture/road.jpg" };
	TextureEditor textureEditor;
	app::CameraController cameraController{ Scene::Size() };
	procedural::StairGenerator stairGenerator;
	const auto isCursorOnGeneratorUI = [&]()
	{
       return stairGenerator.wantsMouseCapture();
	};
	const auto isCursorOnMainUI = [&]()
	{
      return cameraController.isCursorOnUI() || isCursorOnGeneratorUI() || sceneAssets.wantsMouseCapture();
	};
    const auto getWheelZoomFocusPosition = [&]() -> Optional<Vec3>
	{
     const Ray centerRay = cameraController.camera().screenToRay(Scene::CenterF());
		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

		if (const auto distance = centerRay.intersects(groundPlane))
		{
			return centerRay.point_at(*distance);
		}

		return none;
	};
	const auto drawScene = [&]()
	{
      Pi3D::EnvironmentRef().drawGround(sceneAssets.groundPlane(), sceneAssets.groundTexture());
		textureEditor.draw3D();
		roadEditor.draw3D();
     stairGenerator.draw3D(not cameraController.isUIHidden());
      sceneAssets.drawStaticScene(Pi3D::LightingRef().getEffectiveSunDirection());
		Pi3D::EnvironmentRef().draw3D();
	};

	while (System::Update())
	{
       stairGenerator.setUIHidden(cameraController.isUIHidden());
		cameraController.update({
			.isDragPanBlocked = [&]() { return roadEditor.wantsMouseCapture() || isCursorOnMainUI(); },
			.isWheelZoomBlocked = [&]() { return roadEditor.wantsMouseCapture() || textureEditor.wantsMouseWheelCapture() || Pi3D::WantsMouseWheelCapture() || isCursorOnMainUI(); },
			.isFreeCameraBlocked = [&]() { return isCursorOnMainUI(); },
			.getWheelZoomFocus = [&](const DebugCamera3D&) { return getWheelZoomFocusPosition(); }
		});
		stairGenerator.update(cameraController.camera(), isCursorOnMainUI());
		textureEditor.update(cameraController.camera());
		roadEditor.update(cameraController.camera());
      sceneAssets.updateEditor();
		Graphics3D::SetCameraTransform(cameraController.camera());

#pragma region Addon
		Pi3D::Update();
		Pi3D::Begin3D(drawScene);
     Pi3D::End3D(drawScene);
		if (not cameraController.isUIHidden())
		{
            Pi3D::DrawUI();
			const auto cameraEvents = cameraController.drawUI();
			stairGenerator.drawUI();
			if (cameraEvents.previewStarted)
			{
				stairGenerator.cancelTargetSelection();
			}

			roadEditor.drawUI();
			textureEditor.drawUI();
           sceneAssets.drawEditorUI();
		}

		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion
	}
}


