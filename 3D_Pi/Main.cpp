# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Addons/Pi3D/Pi3D.hpp"
# include "UI/RectUI.hpp"
# include "Road/RoadEditor.hpp"
# include "Ground/TextureEditor.hpp"

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
	RoadEditor roadEditor{ U"texture/road.jpg" };
	TextureEditor textureEditor;

   struct CameraPreset
	{
		String label;
		Vec3 eye;
		Vec3 focus;
	};

	DebugCamera3D camera{ Scene::Size(), 40_deg, Vec3{ 0, 3, -16 } };

	const Array<CameraPreset> cameraPresets = {
		{ U"Low", Vec3{ 0, 3, -16 }, Vec3{ 0, 0, 0 } },
		{ U"Medium", Vec3{ 0, 10, -18 }, Vec3{ 0, 0, 0 } },
		{ U"Top", Vec3{ 0, 24, -0.6 }, Vec3{ 0, 0, 0 } },
	};
	size_t activeCameraPresetIndex = 0;
	const auto applyCameraPreset = [&](const size_t index)
	{
		if (index < cameraPresets.size())
		{
			activeCameraPresetIndex = index;
			const Vec3 currentFocus = camera.getFocusPosition();
			const Vec3 presetOffset = cameraPresets[index].eye - cameraPresets[index].focus;
			camera.setView(currentFocus + presetOffset, currentFocus);
		}
	};
	const auto getCameraPresetPanelRect = [&]()
	{
		return RectF{ (Scene::Width() * 0.5) - 206, Scene::Height() - 72, 412, 48 };
	};
	const auto isCursorOnCameraPresetUI = [&]()
	{
		return getCameraPresetPanelRect().mouseOver();
	};
   const auto updateCameraWheelFocus = [&]() -> bool
	{
     if (roadEditor.wantsMouseCapture() || isCursorOnCameraPresetUI())
		{
         return false;
		}

		const double wheel = Mouse::Wheel();
		if (wheel == 0.0)
		{
         return false;
		}

          const Vec3 focus = camera.getFocusPosition();
		const Vec3 eye = camera.getEyePosition();
       const Vec3 eyeOffset = eye - focus;
		const double distance = eyeOffset.length();
		if (distance <= 0.001)
		{
         return false;
		}

      const double zoomScale = Math::Pow(1.16, wheel);
		const double targetDistance = Clamp(distance * zoomScale, 2.0, 160.0);
         camera.setView(focus + eyeOffset * (targetDistance / distance), focus);
      return true;
	};
	const auto drawScene = [&]()
	{
		Pi3D::EnvironmentRef().drawGround(groundPlane, groundTexture);
		textureEditor.draw3D();
		roadEditor.draw3D();
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
     if (not updateCameraWheelFocus())
		{
            if (not isCursorOnCameraPresetUI())
			{
				camera.update(6.0);
			}
		}
     textureEditor.update(camera);
		roadEditor.update(camera);
		Graphics3D::SetCameraTransform(camera);

#pragma region Addon
		Pi3D::Update();
		Pi3D::Begin3D(drawScene);
		Pi3D::End3D(drawScene);
		Pi3D::DrawUI();

		const RectF cameraPresetPanel = getCameraPresetPanelRect();
		ui::Panel(cameraPresetPanel);
		const double buttonGap = 8.0;
		const double innerPadding = 8.0;
		const double buttonWidth = (cameraPresetPanel.w - (innerPadding * 2.0) - (buttonGap * 2.0)) / 3.0;
		for (size_t i = 0; i < cameraPresets.size(); ++i)
		{
			const RectF buttonRect{
				cameraPresetPanel.x + innerPadding + (i * (buttonWidth + buttonGap)),
				cameraPresetPanel.y + 8,
				buttonWidth,
				32
			};

			if (ui::Button(ui::DefaultFont(), cameraPresets[i].label, buttonRect))
			{
				applyCameraPreset(i);
			}

			if (i == activeCameraPresetIndex)
			{
				buttonRect.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
			}
		}

		roadEditor.drawUI();
		textureEditor.drawUI();

		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion
	}
}


