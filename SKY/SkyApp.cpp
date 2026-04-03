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
	String blacksmithMenuMessage;
	double blacksmithMenuMessageUntil = 0.0;
	String cameraSaveMessage;
	double cameraSaveMessageUntil = 0.0;
	String modelHeightMessage;
	double modelHeightMessageUntil = 0.0;
	bool modelHeightEditMode = false;

	while (System::Update())
	{
		birdModel.update(Scene::DeltaTime());
		ashigaruModel.update(Scene::DeltaTime());
		const bool isEditorMode = (appMode == AppMode::EditMap);
		mapEditor.enabled = isEditorMode;
		const Rect skySettingsPanel{ 20, 20, 480, 430 };
		const Rect cameraSettingsPanel{ 520, 20, 360, 380 };
		const Rect mapEditorPanel{ (Scene::Width() - 360), 20, 340, 300 };
		const Rect blacksmithMenuPanel{ (Scene::Width() - 320), (Scene::Height() - 190), 300, 150 };
		const Rect modelHeightPanel{ 860, 20, 400, 300 };
		const Rect uiTogglePanel{ 20, (Scene::Height() - 100), 140, 36 };
		const Rect mapModeTogglePanel{ 180, (Scene::Height() - 100), 180, 36 };
		const Rect modelHeightModeTogglePanel{ 380, (Scene::Height() - 100), 220, 36 };
		const Rect timeSliderPanel{ 20, (Scene::Height() - 60), (Scene::Width() - 20), 36 };
		const bool isHoveringUI = (showUI && (skySettingsPanel.mouseOver() || cameraSettingsPanel.mouseOver()))
			|| (isEditorMode && mapEditorPanel.mouseOver())
			|| (showBlacksmithMenu && blacksmithMenuPanel.mouseOver())
			|| (modelHeightEditMode && modelHeightPanel.mouseOver())
			|| uiTogglePanel.mouseOver()
			|| mapModeTogglePanel.mouseOver()
			|| modelHeightModeTogglePanel.mouseOver()
			|| timeSliderPanel.mouseOver();

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
				const double wheel = Mouse::Wheel();

				if (wheel != 0.0)
				{
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

			DrawMapEditorScene(mapEditor, mapData);

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

			{
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
			}

			{
				sky.starBrightness = Math::Saturate(1.0 - Pow(Abs(1.0 - starCenteredTime) * 1.8, 4));
				sky.fogHeightSky = (1.0 - tf);
				sky.cloudColor = ColorF{ 0.02 + (night ? 0.0 : (0.98 * tc)) };
				sky.sunEnabled = (not night);
				sky.cloudTime = skyTime * sky.cloudScale * 40.0;
				sky.starTime = skyTime;
			}

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
				DrawModelHeightEditor(modelHeightSettings, modelHeightMessage, modelHeightMessageUntil, modelHeightPanel, birdRenderPosition, ashigaruRenderPosition);
			}
		}

		if (showUI)
		{
			skySettingsPanel.draw(ColorF{ 1.0, 0.92 });
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

			cameraSettingsPanel.draw(ColorF{ 1.0, 0.92 });
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
				cameraSaveMessage = SaveCameraSettings(cameraSettings)
					? U"Saved: {}"_fmt(CameraSettingsPath)
					: U"Save failed";
				cameraSaveMessageUntil = (Scene::Time() + 2.0);
			}

			if (DrawTextButton(Rect{ 710, 330, 150, 30 }, U"視点初期化"))
			{
				cameraSettings.eye = DefaultCameraEye;
				cameraSettings.focus = DefaultCameraFocus;
				camera.setView(cameraSettings.eye, cameraSettings.focus);
				cameraSaveMessage = U"Camera reset";
				cameraSaveMessageUntil = (Scene::Time() + 2.0);
			}

			if (Scene::Time() < cameraSaveMessageUntil)
			{
				SimpleGUI::GetFont()(cameraSaveMessage).draw(540, 372, ColorF{ 0.11 });
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
			DrawMapEditorPanel(mapEditor, mapData, MapDataPath, mapEditorPanel);
		}

		if ((not isEditorMode) && showBlacksmithMenu)
		{
			blacksmithMenuPanel.draw(ColorF{ 0.98, 0.95 });
			blacksmithMenuPanel.drawFrame(2, 0, ColorF{ 0.25 });
			SimpleGUI::GetFont()(U"ユニット生産メニュー").draw((blacksmithMenuPanel.x + 16), (blacksmithMenuPanel.y + 12), ColorF{ 0.12 });

			const Rect produceSapperButton{ (blacksmithMenuPanel.x + 16), (blacksmithMenuPanel.y + 46), (blacksmithMenuPanel.w - 32), 36 };
			const Rect tierUpgradeButton{ (blacksmithMenuPanel.x + 16), (blacksmithMenuPanel.y + 92), (blacksmithMenuPanel.w - 32), 36 };

			if (DrawTextButton(produceSapperButton, U"工兵産出"))
			{
				const size_t sapperIndex = spawnedSappers.size();
				const Vec3 startPosition = BlacksmithPosition.movedBy(2.4, 0, 2.2);
				const Vec3 targetPosition = GetSapperPopTargetPosition(mapData.sapperRallyPoint, sapperIndex);
				spawnedSappers << SpawnedSapper{
					.startPosition = startPosition,
					.position = targetPosition,
					.targetPosition = targetPosition,
					.spawnedAt = Scene::Time(),
				};
				blacksmithMenuMessage = U"工兵を産出";
				blacksmithMenuMessageUntil = (Scene::Time() + 2.0);
			}

			if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード"))
			{
				blacksmithMenuMessage = U"ティアアップグレードを選択";
				blacksmithMenuMessageUntil = (Scene::Time() + 2.0);
			}

			if (Scene::Time() < blacksmithMenuMessageUntil)
			{
				SimpleGUI::GetFont()(blacksmithMenuMessage).draw((blacksmithMenuPanel.x + 16), (blacksmithMenuPanel.y - 28), ColorF{ 0.12 });
			}
		}

		if (DrawTextButton(mapModeTogglePanel, isEditorMode ? U"Map Edit: ON" : U"Map Edit: OFF"))
		{
			appMode = isEditorMode ? AppMode::Play : AppMode::EditMap;
			showBlacksmithMenu = false;
			mapEditor.hoveredGroundPosition.reset();
		}

		if (DrawTextButton(modelHeightModeTogglePanel, modelHeightEditMode ? U"Model Height: ON" : U"Model Height: OFF"))
		{
			modelHeightEditMode = not modelHeightEditMode;
		}

		SimpleGUI::CheckBox(showUI, U"UI", Vec2{ 20, Scene::Height() - 100 });
		SimpleGUI::Slider(U"time: {:.2f}"_fmt(skyTime), skyTime, -2.0, 4.0, Vec2{ 20, Scene::Height() - 60 }, 120, Scene::Width() - 160);
	}
}
