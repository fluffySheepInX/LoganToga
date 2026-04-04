# include <Siv3D.hpp>
# include "BirdModel.hpp"
# include "MainContext.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"
# include "MapData.hpp"
# include "MapEditor.hpp"
# include "SkyApp.hpp"

using namespace MainSupport;

namespace
{
	constexpr double MessageDisplaySeconds = 2.0;

	struct TimedMessage
	{
		String text;
		double until = 0.0;

		void show(const StringView message)
		{
			text = message;
			until = (Scene::Time() + MessageDisplaySeconds);
		}

		[[nodiscard]] bool isVisible() const
		{
			return (Scene::Time() < until);
		}
	};

	struct SkyAppPanels
	{
		Rect skySettings{ 20, 20, 480, 430 };
		Rect cameraSettings{ 520, 20, 360, 380 };
		Rect mapEditor{ (Scene::Width() - 360), 20, 340, 300 };
		Rect blacksmithMenu{ (Scene::Width() - 320), (Scene::Height() - 190), 300, 150 };
		Rect modelHeight{ 860, 20, 400, 300 };
		Rect uiToggle{ 20, (Scene::Height() - 100), 140, 36 };
		Rect mapModeToggle{ 180, (Scene::Height() - 100), 180, 36 };
		Rect modelHeightModeToggle{ 380, (Scene::Height() - 100), 220, 36 };
		Rect timeSlider{ 20, (Scene::Height() - 60), (Scene::Width() - 20), 36 };

		[[nodiscard]] bool isHoveringUi(const bool showUI, const bool isEditorMode, const bool showBlacksmithMenu, const bool modelHeightEditMode) const
		{
			return (showUI && (skySettings.mouseOver() || cameraSettings.mouseOver()))
				|| (isEditorMode && mapEditor.mouseOver())
				|| (showBlacksmithMenu && blacksmithMenu.mouseOver())
				|| (modelHeightEditMode && modelHeight.mouseOver())
				|| uiToggle.mouseOver()
				|| mapModeToggle.mouseOver()
				|| modelHeightModeToggle.mouseOver()
				|| timeSlider.mouseOver();
		}
	};

	void UpdateCameraWheelZoom(DebugCamera3D& camera, CameraSettings& cameraSettings)
	{
		const double wheel = Mouse::Wheel();

		if (wheel == 0.0)
		{
			return;
		}

		if (const auto zoomFocus = GetWheelZoomFocusPosition(camera))
		{
			const Vec3 focusPosition = *zoomFocus;
			const Vec3 eyeOffset = (cameraSettings.eye - focusPosition);
			const double currentDistance = eyeOffset.length();

			if (0.001 < currentDistance)
			{
				const double targetDistance = Clamp((currentDistance * Math::Pow(CameraZoomFactorPerWheelStep, wheel)), CameraZoomMinDistance, CameraZoomMaxDistance);
				cameraSettings.eye = (focusPosition + (eyeOffset * (targetDistance / currentDistance)));
				cameraSettings.focus = focusPosition;
				camera.setView(cameraSettings.eye, cameraSettings.focus);
			}
		}
	}

	void DrawSpawnedSappers(const Array<SpawnedSapper>& spawnedSappers)
	{
		for (const auto& sapper : spawnedSappers)
		{
			const double elapsed = (Scene::Time() - sapper.spawnedAt);
			const double popIn = Min(elapsed / 0.25, 1.0);
			const double moveT = EaseOutCubic(Min(elapsed / 0.45, 1.0));
			const Vec3 currentBasePosition = sapper.startPosition.lerp(sapper.targetPosition, moveT);
			const double bounce = (0.18 * Max(0.0, 1.0 - elapsed / 0.5) * Abs(Math::Sin(elapsed * 18.0)));
			const double radius = (0.22 + (0.68 * EaseOutBack(popIn)));
			Sphere{ currentBasePosition.movedBy(0, radius + bounce, 0), radius }.draw(ColorF{ 0.9, 0.94, 1.0 }.removeSRGBCurve());
		}
	}

	void UpdateSkyFromTime(Sky& sky, const double skyTime)
	{
		const double time0_2 = Math::Fraction(skyTime * 0.5) * 2.0;
		const double halfDay0_1 = Math::Fraction(skyTime);
		const double distanceFromNoon0_1 = Math::Saturate(1.0 - (Abs(0.5 - halfDay0_1) * 2.0));
		const bool night = (1.0 < time0_2);
		const double tf = EaseOutCubic(distanceFromNoon0_1);
		const double tc = EaseInOutCubic(distanceFromNoon0_1);
		const double starCenteredTime = Math::Fmod(time0_2 + 1.5, 2.0);

		{
			const Quaternion q = (Quaternion::RotateY(halfDay0_1 * 180_deg) * Quaternion::RotateX(50_deg));
			const Vec3 sunDirection = q * Vec3::Right();
			const ColorF sunColor{ 0.1 + Math::Pow(tf, 1.0 / 2.0) * (night ? 0.1 : 0.9) };

			Graphics3D::SetSunDirection(sunDirection);
			Graphics3D::SetSunColor(sunColor);
			Graphics3D::SetGlobalAmbientColor(ColorF{ sky.zenithColor });
		}

		if (night)
		{
			sky.zenithColor = ColorF{ 0.3, 0.05, 0.1 }.lerp(ColorF{ 0.1, 0.1, 0.15 }, tf);
			sky.horizonColor = ColorF{ 0.1, 0.1, 0.15 }.lerp(ColorF{ 0.1, 0.1, 0.2 }, tf);
		}
		else
		{
			sky.zenithColor = ColorF{ 0.4, 0.05, 0.1 }.lerp(ColorF{ 0.15, 0.24, 0.56 }, tf);
			sky.horizonColor = ColorF{ 0.2, 0.05, 0.15 }.lerp(ColorF{ 0.3, 0.4, 0.5 }, tf);
		}

		sky.starBrightness = Math::Saturate(1.0 - Pow(Abs(1.0 - starCenteredTime) * 1.8, 4));
		sky.fogHeightSky = (1.0 - tf);
		sky.cloudColor = ColorF{ 0.02 + (night ? 0.0 : (0.98 * tc)) };
		sky.sunEnabled = (not night);
		sky.cloudTime = skyTime * sky.cloudScale * 40.0;
		sky.starTime = skyTime;
	}

	void SpawnSapper(Array<SpawnedSapper>& spawnedSappers, const Vec3& rallyPoint)
	{
		const size_t sapperIndex = spawnedSappers.size();
		const Vec3 startPosition = BlacksmithPosition.movedBy(2.4, 0, 2.2);
		const Vec3 targetPosition = GetSapperPopTargetPosition(rallyPoint, sapperIndex);
		spawnedSappers << SpawnedSapper{
			.startPosition = startPosition,
			.position = targetPosition,
			.targetPosition = targetPosition,
			.spawnedAt = Scene::Time(),
		};
	}
}

void RunSkyApp()
{
	Window::Resize(1280, 720);
	const Mesh groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) };
	const Texture groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB };
	const Model blacksmithModel{ U"example/obj/blacksmith.obj" };
	const Model millModel{ U"example/obj/mill.obj" };
	const Model treeModel{ U"example/obj/tree.obj" };
	const Model pineModel{ U"example/obj/pine.obj" };
	BirdModel birdModel{ BirdModelPath, BirdDisplayHeight };
	BirdModel ashigaruModel{ AshigaruModelPath, BirdDisplayHeight };
	Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	CameraSettings cameraSettings = LoadCameraSettings();
	ModelHeightSettings modelHeightSettings = LoadModelHeightSettings();
	MapData mapData = LoadMapData(MapDataPath);
	DebugCamera3D camera{ Graphics3D::GetRenderTargetSize(), 40_deg, cameraSettings.eye, cameraSettings.focus };

	Sky sky;
	double skyTime = 0.5;
	bool showUI = true;
	AppMode appMode = AppMode::Play;
	MapEditorState mapEditor;
	bool showBlacksmithMenu = false;
	Array<SpawnedSapper> spawnedSappers;
   TimedMessage blacksmithMenuMessage;
	TimedMessage cameraSaveMessage;
	TimedMessage modelHeightMessage;
	bool modelHeightEditMode = false;

	while (System::Update())
	{
		birdModel.update(Scene::DeltaTime());
		ashigaruModel.update(Scene::DeltaTime());
		const bool isEditorMode = (appMode == AppMode::EditMap);
		mapEditor.enabled = isEditorMode;
        const SkyAppPanels panels;
		const bool isHoveringUI = panels.isHoveringUi(showUI, isEditorMode, showBlacksmithMenu, modelHeightEditMode);

		const Vec3 birdRenderPosition = BirdDisplayPosition.movedBy(0, modelHeightSettings.birdOffsetY, 0);
		const Vec3 ashigaruRenderPosition = AshigaruDisplayPosition.movedBy(0, modelHeightSettings.ashigaruOffsetY, 0);

		{
			const ScopedRenderTarget3D target{ renderTexture.clear(ColorF{ 0.0 }) };
			camera.setView(cameraSettings.eye, cameraSettings.focus);
			if (not isHoveringUI)
			{
				camera.update(4.0);
			}
			cameraSettings.eye = camera.getEyePosition();
			cameraSettings.focus = camera.getFocusPosition();

			if (not isHoveringUI)
			{
                UpdateCameraWheelZoom(camera, cameraSettings);
			}

			Graphics3D::SetCameraTransform(camera);
			UpdateMapEditor(mapEditor, mapData, camera, (isEditorMode && (not isHoveringUI)));

			const bool blacksmithHovered = BlacksmithInteractionSphere.intersects(camera.screenToRay(Cursor::PosF())).has_value();

			if ((not isEditorMode) && (not isHoveringUI) && blacksmithHovered && MouseL.down())
			{
				showBlacksmithMenu = not showBlacksmithMenu;
			}

			groundPlane.draw(groundTexture);
			Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());
			blacksmithModel.draw(BlacksmithPosition);
			if (birdModel.isLoaded())
			{
				birdModel.draw(birdRenderPosition, BirdDisplayYaw, ColorF{ 0.92, 0.95, 1.0 }.removeSRGBCurve());
			}
			if (ashigaruModel.isLoaded())
			{
				ashigaruModel.draw(ashigaruRenderPosition, BirdDisplayYaw, ColorF{ 0.95, 0.92, 0.90 }.removeSRGBCurve());
			}
			for (const auto& placedModel : mapData.placedModels)
			{
				DrawPlacedModel(placedModel, millModel, treeModel, pineModel);
			}

           DrawSpawnedSappers(spawnedSappers);

			DrawMapEditorScene(mapEditor, mapData);

			UpdateSkyFromTime(sky, skyTime);

			sky.draw();
		}

		{
			Graphics3D::Flush();
			renderTexture.resolve();
			Shader::LinearToScreen(renderTexture);

			if (const auto birdGroundPoint = birdModel.groundContactPoint(birdRenderPosition, BirdDisplayYaw))
			{
				DrawGroundContactOverlay(camera, *birdGroundPoint);
			}

			if (const auto ashigaruGroundPoint = ashigaruModel.groundContactPoint(ashigaruRenderPosition, BirdDisplayYaw))
			{
				DrawGroundContactOverlay(camera, *ashigaruGroundPoint);
			}

			if ((not isEditorMode) && showBlacksmithMenu)
			{
				DrawSelectionIndicator(camera, BlacksmithPosition);
			}

			if (modelHeightEditMode)
			{
              DrawModelHeightEditor(modelHeightSettings, modelHeightMessage.text, modelHeightMessage.until, panels.modelHeight, birdRenderPosition, ashigaruRenderPosition);
			}
		}

		if (showUI)
		{
         panels.skySettings.draw(ColorF{ 1.0, 0.92 });
			Rect{ 20, 20, 480, 76 }.draw();
			SimpleGUI::GetFont()(U"zenith:").draw(28, 24, ColorF{ 0.11 });
			Rect{ 100, 26, 28 }.draw(sky.zenithColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
			SimpleGUI::GetFont()(U"horizon:").draw(148, 24, ColorF{ 0.11 });
			Rect{ 230, 26, 28 }.draw(sky.horizonColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
			SimpleGUI::GetFont()(U"cloud:").draw(276, 24, ColorF{ 0.11 });
			Rect{ 340, 26, 28 }.draw(sky.cloudColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
			SimpleGUI::GetFont()(U"sun:").draw(386, 24, ColorF{ 0.11 });
			Rect{ 430, 26, 28 }.draw(Graphics3D::GetSunColor().gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
			SimpleGUI::GetFont()(U"sunDir: {:.2f}   cloudTime: {:.1f}"_fmt(Graphics3D::GetSunDirection(), sky.cloudTime)).draw(28, 60, ColorF{ 0.11 });

			SimpleGUI::Slider(U"cloudiness: {:.3f}"_fmt(sky.cloudiness), sky.cloudiness, Vec2{ 20, 100 }, 180, 300);
			SimpleGUI::Slider(U"cloudScale: {:.2f}"_fmt(sky.cloudScale), sky.cloudScale, 0.0, 2.0, Vec2{ 20, 140 }, 180, 300);
			SimpleGUI::Slider(U"cloudHeight: {:.0f}"_fmt(sky.cloudPlaneHeight), sky.cloudPlaneHeight, 20.0, 6000.0, Vec2{ 20, 180 }, 180, 300);
			SimpleGUI::Slider(U"orientation: {:.0f}"_fmt(Math::ToDegrees(sky.cloudOrientation)), sky.cloudOrientation, 0.0, Math::TwoPi, Vec2{ 20, 220 }, 180, 300);
			SimpleGUI::Slider(U"fogHeightSky: {:.2f}"_fmt(sky.fogHeightSky), sky.fogHeightSky, Vec2{ 20, 260 }, 180, 300, false);
			SimpleGUI::Slider(U"star: {:.2f}"_fmt(sky.starBrightness), sky.starBrightness, Vec2{ 20, 300 }, 180, 300, false);
			SimpleGUI::Slider(U"starF: {:.2f}"_fmt(sky.starBrightnessFactor), sky.starBrightnessFactor, Vec2{ 20, 340 }, 180, 300);
			SimpleGUI::Slider(U"starSat: {:.2f}"_fmt(sky.starSaturation), sky.starSaturation, 0.0, 1.0, Vec2{ 20, 380 }, 180, 300);
			SimpleGUI::CheckBox(sky.sunEnabled, U"sun", Vec2{ 20, 420 }, 120, false);
			SimpleGUI::CheckBox(sky.cloudsEnabled, U"clouds", Vec2{ 150, 420 }, 120);
			SimpleGUI::CheckBox(sky.cloudsLightingEnabled, U"cloudsLighting", Vec2{ 280, 420 }, 220);

          panels.cameraSettings.draw(ColorF{ 1.0, 0.92 });
			SimpleGUI::GetFont()(U"Camera eye").draw(540, 28, ColorF{ 0.11 });
			SimpleGUI::Slider(U"eyeX: {:.2f}"_fmt(cameraSettings.eye.x), cameraSettings.eye.x, -50.0, 50.0, Vec2{ 540, 60 }, 140, 180);
			SimpleGUI::Slider(U"eyeY: {:.2f}"_fmt(cameraSettings.eye.y), cameraSettings.eye.y, -10.0, 50.0, Vec2{ 540, 100 }, 140, 180);
			SimpleGUI::Slider(U"eyeZ: {:.2f}"_fmt(cameraSettings.eye.z), cameraSettings.eye.z, -50.0, 50.0, Vec2{ 540, 140 }, 140, 180);
			SimpleGUI::GetFont()(U"Camera focus").draw(540, 190, ColorF{ 0.11 });
			SimpleGUI::Slider(U"focusX: {:.2f}"_fmt(cameraSettings.focus.x), cameraSettings.focus.x, -50.0, 50.0, Vec2{ 540, 220 }, 140, 180);
			SimpleGUI::Slider(U"focusY: {:.2f}"_fmt(cameraSettings.focus.y), cameraSettings.focus.y, -10.0, 50.0, Vec2{ 540, 260 }, 140, 180);
			SimpleGUI::Slider(U"focusZ: {:.2f}"_fmt(cameraSettings.focus.z), cameraSettings.focus.z, -50.0, 50.0, Vec2{ 540, 300 }, 140, 180);

			if (DrawTextButton(Rect{ 540, 330, 150, 30 }, U"Save TOML"))
			{
              cameraSaveMessage.show(SaveCameraSettings(cameraSettings)
					? U"Saved: {}"_fmt(CameraSettingsPath)
					: U"Save failed");
			}

			if (DrawTextButton(Rect{ 710, 330, 150, 30 }, U"視点初期化"))
			{
				cameraSettings.eye = DefaultCameraEye;
				cameraSettings.focus = DefaultCameraFocus;
				camera.setView(cameraSettings.eye, cameraSettings.focus);
                cameraSaveMessage.show(U"Camera reset");
			}

         if (cameraSaveMessage.isVisible())
			{
             SimpleGUI::GetFont()(cameraSaveMessage.text).draw(540, 372, ColorF{ 0.11 });
			}

			if (not birdModel.isLoaded())
			{
				SimpleGUI::GetFont()(U"bird.glb load failed").draw(540, 396, ColorF{ 0.75, 0.2, 0.2 });
			}

			if (not ashigaruModel.isLoaded())
			{
				SimpleGUI::GetFont()(U"ashigaru_v2.1.glb load failed").draw(540, 420, ColorF{ 0.75, 0.2, 0.2 });
			}

			DrawAnimationClipSelector(birdModel, U"Bird Clips", 540, 420, 150);
			DrawAnimationClipSelector(ashigaruModel, U"Ashigaru Clips", 710, 420, 150);
		}

		if (isEditorMode)
		{
            DrawMapEditorPanel(mapEditor, mapData, MapDataPath, panels.mapEditor);
		}

		if ((not isEditorMode) && showBlacksmithMenu)
		{
         panels.blacksmithMenu.draw(ColorF{ 0.98, 0.95 });
			panels.blacksmithMenu.drawFrame(2, 0, ColorF{ 0.25 });
			SimpleGUI::GetFont()(U"ユニット生産メニュー").draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 12), ColorF{ 0.12 });

         const Rect produceSapperButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 46), (panels.blacksmithMenu.w - 32), 36 };
			const Rect tierUpgradeButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 92), (panels.blacksmithMenu.w - 32), 36 };

			if (DrawTextButton(produceSapperButton, U"工兵産出"))
			{
               SpawnSapper(spawnedSappers, mapData.sapperRallyPoint);
				blacksmithMenuMessage.show(U"工兵を産出");
			}

			if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード"))
			{
               blacksmithMenuMessage.show(U"ティアアップグレードを選択");
			}

         if (blacksmithMenuMessage.isVisible())
			{
               SimpleGUI::GetFont()(blacksmithMenuMessage.text).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y - 28), ColorF{ 0.12 });
			}
		}

      if (DrawTextButton(panels.mapModeToggle, isEditorMode ? U"Map Edit: ON" : U"Map Edit: OFF"))
		{
			appMode = isEditorMode ? AppMode::Play : AppMode::EditMap;
			showBlacksmithMenu = false;
			mapEditor.hoveredGroundPosition.reset();
		}

       if (DrawTextButton(panels.modelHeightModeToggle, modelHeightEditMode ? U"Model Height: ON" : U"Model Height: OFF"))
		{
			modelHeightEditMode = not modelHeightEditMode;
		}

		SimpleGUI::CheckBox(showUI, U"UI", Vec2{ 20, Scene::Height() - 100 });
		SimpleGUI::Slider(U"time: {:.2f}"_fmt(skyTime), skyTime, -2.0, 4.0, Vec2{ 20, Scene::Height() - 60 }, 120, Scene::Width() - 160);
	}
}
