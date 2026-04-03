# include <Siv3D.hpp>
# include <assimp/Importer.hpp>
# include <assimp/postprocess.h>
# include <assimp/scene.h>
# include "MapData.hpp"
# include "MapEditor.hpp"

namespace
{
	constexpr FilePathView CameraSettingsPath = U"App/settings/camera_settings.toml";
   constexpr FilePathView MapDataPath = U"App/settings/map_data.toml";
   constexpr FilePathView BirdModelPath = U"model/bird.glb";
	constexpr Vec3 DefaultCameraEye{ 0, 3, -16 };
	constexpr Vec3 DefaultCameraFocus{ 0, 0, 0 };
   constexpr Vec3 BirdDisplayPosition{ -8, 0, -2.5 };
   constexpr double BirdDisplayYaw = 0_deg;
   constexpr Vec3 BlacksmithPosition{ 8, 0, 4 };
	constexpr Sphere BlacksmithInteractionSphere{ BlacksmithPosition + Vec3{ 0, 4.0, 0 }, 4.5 };
	constexpr Vec3 BlacksmithSelectionBoxSize{ 8.0, 8.0, 8.0 };
	constexpr Vec3 BlacksmithSelectionBoxPadding{ 1.2, 0.8, 1.2 };
	constexpr double CameraZoomMinDistance = 3.0;
	constexpr double CameraZoomMaxDistance = 80.0;
	constexpr double CameraZoomFactorPerWheelStep = 0.85;
	constexpr double BirdDisplayHeight = 3.6;

	struct CameraSettings
	{
		Vec3 eye = DefaultCameraEye;
		Vec3 focus = DefaultCameraFocus;
	};

	struct SpawnedSapper
	{
      Vec3 startPosition;
		Vec3 position;
     Vec3 targetPosition;
		double spawnedAt = 0.0;
	};

	struct ImportedStaticMesh
	{
		Mesh mesh;
		bool loaded = false;
	};

	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
	{
		try
		{
          Array<double> values;
			values.reserve(3);

			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 3)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	[[nodiscard]] CameraSettings LoadCameraSettings()
	{
		const TOMLReader toml{ CameraSettingsPath };

		if (not toml)
		{
			return{};
		}

		return{
			.eye = ReadTomlVec3(toml, U"eye", DefaultCameraEye),
			.focus = ReadTomlVec3(toml, U"focus", DefaultCameraFocus),
		};
	}

	bool SaveCameraSettings(const CameraSettings& settings)
	{
		FileSystem::CreateDirectories(U"App/settings");

		TextWriter writer{ CameraSettingsPath };

		if (not writer)
		{
			return false;
		}

		writer.writeln(U"eye = [{:.3f}, {:.3f}, {:.3f}]"_fmt(settings.eye.x, settings.eye.y, settings.eye.z));
		writer.writeln(U"focus = [{:.3f}, {:.3f}, {:.3f}]"_fmt(settings.focus.x, settings.focus.y, settings.focus.z));
		return true;
	}

	[[nodiscard]] ImportedStaticMesh LoadStaticGlbMesh(FilePathView path)
	{
		if (not FileSystem::Exists(path))
		{
			return{};
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(Unicode::ToUTF8(FileSystem::FullPath(path)),
			(aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_PreTransformVertices
			| aiProcess_GenSmoothNormals
			| aiProcess_SortByPType
			| aiProcess_ConvertToLeftHanded
			| aiProcess_FlipUVs));

		if ((scene == nullptr) || (not scene->HasMeshes()))
		{
			return{};
		}

		Array<Vertex3D> vertices;
		Array<TriangleIndex32> indices;
		size_t totalVertices = 0;
		size_t totalTriangles = 0;

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if (mesh == nullptr)
			{
				continue;
			}

			totalVertices += mesh->mNumVertices;
			totalTriangles += mesh->mNumFaces;
		}

		vertices.reserve(totalVertices);
		indices.reserve(totalTriangles);

		for (uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			const aiMesh* mesh = scene->mMeshes[meshIndex];

			if ((mesh == nullptr) || (mesh->mNumVertices == 0))
			{
				continue;
			}

			const uint32 vertexOffset = static_cast<uint32>(vertices.size());

			for (uint32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
			{
				const aiVector3D& pos = mesh->mVertices[vertexIndex];
				const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D{ 0.0f, 1.0f, 0.0f };
				const aiVector3D tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D{};

				vertices << Vertex3D{
					.pos = Float3{ pos.x, pos.y, pos.z },
					.normal = Float3{ normal.x, normal.y, normal.z },
					.tex = Float2{ tex.x, tex.y },
				};
			}

			for (uint32 faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
			{
				const aiFace& face = mesh->mFaces[faceIndex];

				if (face.mNumIndices != 3)
				{
					continue;
				}

				indices << TriangleIndex32{
					.i0 = (vertexOffset + face.mIndices[0]),
					.i1 = (vertexOffset + face.mIndices[1]),
					.i2 = (vertexOffset + face.mIndices[2]),
				};
			}
		}

		if (vertices.isEmpty() || indices.isEmpty())
		{
			return{};
		}

		MeshData meshData{ std::move(vertices), std::move(indices) };
		const Box bounds = meshData.computeBoundingBox();
		const double scale = (0.001 < bounds.h)
			? (BirdDisplayHeight / bounds.h)
			: 1.0;
		const double bottomY = (bounds.center.y - (bounds.h * 0.5));
		meshData.translate(-bounds.center.x, -bottomY, -bounds.center.z);
		meshData.scale(scale);

		return{
			.mesh = Mesh{ meshData },
			.loaded = true,
		};
	}

	bool DrawTextButton(const Rect& rect, StringView label)
	{
      static const Font buttonFont{ 18, Typeface::Bold };
		const bool hovered = rect.mouseOver();
		rect.draw(hovered ? ColorF{ 0.82 } : ColorF{ 0.72 })
			.drawFrame(1, 0, ColorF{ 0.35 });
     buttonFont(label).drawAt(rect.center(), ColorF{ 0.15 });
		return hovered && MouseL.down();
	}

	[[nodiscard]] Optional<Vec3> GetWheelZoomFocusPosition(const DebugCamera3D& camera)
	{
		const Ray centerRay = camera.screenToRay(Scene::CenterF());
		double nearestDistance = Math::Inf;
		Optional<Vec3> focusPosition;

		if (const auto distance = centerRay.intersects(BlacksmithInteractionSphere))
		{
			nearestDistance = *distance;
			focusPosition = centerRay.point_at(*distance);
		}

		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

		if (const auto distance = centerRay.intersects(groundPlane))
		{
			if (*distance < nearestDistance)
			{
				nearestDistance = *distance;
				focusPosition = centerRay.point_at(*distance);
			}
		}

		return focusPosition;
	}

 [[nodiscard]] Vec3 GetSapperPopTargetPosition(const Vec3& rallyPoint, const size_t sapperIndex)
	{
		const int32 columns = 3;
		const double spacing = 1.9;
		const int32 row = static_cast<int32>(sapperIndex / columns);
		const int32 column = static_cast<int32>(sapperIndex % columns);
		const double xOffset = ((column - 1) * spacing);
		const double zOffset = (row * spacing);
        return rallyPoint.movedBy(xOffset, 0, zOffset);
	}
}

enum class AppMode
{
	Play,
	EditMap,
};

void DrawMillModel(const Model& model, const Mat4x4& mat)
{
	const auto& materials = model.materials();

	for (const auto& object : model.objects())
	{
		Mat4x4 m = Mat4x4::Identity();

		if (object.name == U"Mill_Blades_Cube.007")
		{
			m *= Mat4x4::Rotate(Vec3{ 0,0,-1 }, (Scene::Time() * -120_deg), Vec3{ 0, 9.37401, 0 });
		}

		const Transformer3D t{ (m * mat) };

		object.draw(materials);
	}
}

void DrawPlacedModel(const PlacedModel& placedModel, const Model& millModel, const Model& treeModel, const Model& pineModel)
{
	switch (placedModel.type)
	{
	case PlaceableModelType::Mill:
		DrawMillModel(millModel, Mat4x4::Translate(placedModel.position));
		return;

	case PlaceableModelType::Tree:
		{
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(placedModel.position);
			return;
		}

	case PlaceableModelType::Pine:
		{
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			pineModel.draw(placedModel.position);
			return;
		}

	default:
		return;
	}
}

bool DrawProjectedLine3D(const DebugCamera3D& camera, const Vec3& start, const Vec3& end, const double thickness, const ColorF& color)
{
  const Float3 startScreen = camera.worldToScreenPoint(Float3{ static_cast<float>(start.x), static_cast<float>(start.y), static_cast<float>(start.z) });
	const Float3 endScreen = camera.worldToScreenPoint(Float3{ static_cast<float>(end.x), static_cast<float>(end.y), static_cast<float>(end.z) });

   if ((startScreen.z <= 0.0f) || (endScreen.z <= 0.0f))
	{
       return false;
	}

  Line{ Vec2{ startScreen.x, startScreen.y }, Vec2{ endScreen.x, endScreen.y } }.draw(thickness, color);
	return true;
}

void DrawArrowAccent3D(const DebugCamera3D& camera, const Vec3& anchor, const Vec3& armA, const Vec3& armB, const double thickness, const ColorF& color)
{
 DrawProjectedLine3D(camera, anchor, (anchor + armA), thickness, color);
	DrawProjectedLine3D(camera, anchor, (anchor + armB), thickness, color);
}

void DrawSelectionIndicator(const DebugCamera3D& camera, const Vec3& position)
{
 const Vec3 selectionBoxSize = (BlacksmithSelectionBoxSize + BlacksmithSelectionBoxPadding);
	const double halfWidth = (selectionBoxSize.x * 0.5);
	const double halfDepth = (selectionBoxSize.z * 0.5);
	constexpr double GroundHeight = 0.06;
	const Vec3 topLeft = position.movedBy(-halfWidth, GroundHeight, -halfDepth);
	const Vec3 topRight = position.movedBy(halfWidth, GroundHeight, -halfDepth);
	const Vec3 bottomRight = position.movedBy(halfWidth, GroundHeight, halfDepth);
	const Vec3 bottomLeft = position.movedBy(-halfWidth, GroundHeight, halfDepth);
	const double pulse = (0.65 + (0.35 * Periodic::Sine1_1(1.6s)));
	const ColorF lineColor{ 1.0, 0.9, 0.15, pulse };
	constexpr double lineThickness = 4.5;
 const double accentSize = Max(1.1, Min(selectionBoxSize.x, selectionBoxSize.z) * 0.13);

	DrawProjectedLine3D(camera, topLeft, topRight, lineThickness, lineColor);
	DrawProjectedLine3D(camera, topRight, bottomRight, lineThickness, lineColor);
	DrawProjectedLine3D(camera, bottomRight, bottomLeft, lineThickness, lineColor);
	DrawProjectedLine3D(camera, bottomLeft, topLeft, lineThickness, lineColor);

   auto samplePerimeter = [&](double t)
	{
		t = Math::Fraction(t);

		if (t < 0.25)
		{
			const double edgeT = (t / 0.25);
			return std::pair{ topRight.lerp(topLeft, edgeT), Vec3{ -1, 0, 0 } };
		}
		else if (t < 0.50)
		{
			const double edgeT = ((t - 0.25) / 0.25);
			return std::pair{ topLeft.lerp(bottomLeft, edgeT), Vec3{ 0, 0, 1 } };
		}
		else if (t < 0.75)
		{
			const double edgeT = ((t - 0.50) / 0.25);
			return std::pair{ bottomLeft.lerp(bottomRight, edgeT), Vec3{ 1, 0, 0 } };
		}

		const double edgeT = ((t - 0.75) / 0.25);
		return std::pair{ bottomRight.lerp(topRight, edgeT), Vec3{ 0, 0, -1 } };
	};

	constexpr size_t ArrowCount = 6;
	constexpr double ArrowLoopSpeed = 0.12;

	for (size_t i = 0; i < ArrowCount; ++i)
	{
		const double progress = Math::Fraction((Scene::Time() * ArrowLoopSpeed) + (static_cast<double>(i) / ArrowCount));
		const auto [anchor, tangent] = samplePerimeter(progress);
		const Vec3 backward = (-tangent * accentSize);
		const Vec3 side{ -tangent.z, 0, tangent.x };
		const Vec3 armA = (backward + (side * (accentSize * 0.75)));
		const Vec3 armB = (backward - (side * (accentSize * 0.75)));
		DrawArrowAccent3D(camera, anchor, armA, armB, lineThickness, lineColor);
	}
}

void Main()
{
	Window::Resize(1280, 720);
	const Mesh groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) };
	const Texture groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB };
	const Model blacksmithModel{ U"example/obj/blacksmith.obj" };
	const Model millModel{ U"example/obj/mill.obj" };
	const Model treeModel{ U"example/obj/tree.obj" };
	const Model pineModel{ U"example/obj/pine.obj" };
 const ImportedStaticMesh birdMesh = LoadStaticGlbMesh(BirdModelPath);
	Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
   CameraSettings cameraSettings = LoadCameraSettings();
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

	while (System::Update())
	{
      const bool isEditorMode = (appMode == AppMode::EditMap);
		mapEditor.enabled = isEditorMode;
      const Rect skySettingsPanel{ 20, 20, 480, 430 };
		const Rect cameraSettingsPanel{ 520, 20, 360, 380 };
     const Rect mapEditorPanel{ (Scene::Width() - 360), 20, 340, 300 };
       const Rect blacksmithMenuPanel{ (Scene::Width() - 320), (Scene::Height() - 190), 300, 150 };
		const Rect uiTogglePanel{ 20, (Scene::Height() - 100), 140, 36 };
        const Rect mapModeTogglePanel{ 180, (Scene::Height() - 100), 180, 36 };
		const Rect timeSliderPanel{ 20, (Scene::Height() - 60), (Scene::Width() - 20), 36 };
		const bool isHoveringUI = (showUI && (skySettingsPanel.mouseOver() || cameraSettingsPanel.mouseOver()))
          || (isEditorMode && mapEditorPanel.mouseOver())
            || (showBlacksmithMenu && blacksmithMenuPanel.mouseOver())
			|| uiTogglePanel.mouseOver()
         || mapModeTogglePanel.mouseOver()
			|| timeSliderPanel.mouseOver();

		// [3D シーンの描画]
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

			// [モデルの描画]
			{
				groundPlane.draw(groundTexture);
				Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());
              blacksmithModel.draw(BlacksmithPosition);
                if (birdMesh.loaded)
				{
                 birdMesh.mesh.draw(BirdDisplayPosition, Quaternion::RotateY(BirdDisplayYaw), ColorF{ 0.92, 0.95, 1.0 }.removeSRGBCurve());
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
			}

			// [天空レンダリング]
			{
				const double time0_2 = Math::Fraction(skyTime * 0.5) * 2.0;
				const double halfDay0_1 = Math::Fraction(skyTime);
				const double distanceFromNoon0_1 = Math::Saturate(1.0 - (Abs(0.5 - halfDay0_1) * 2.0));
				const bool night = (1.0 < time0_2);
				const double tf = EaseOutCubic(distanceFromNoon0_1);
				const double tc = EaseInOutCubic(distanceFromNoon0_1);
				const double starCenteredTime = Math::Fmod(time0_2 + 1.5, 2.0);

				// set sun
				{
					const Quaternion q = (Quaternion::RotateY(halfDay0_1 * 180_deg) * Quaternion::RotateX(50_deg));
					const Vec3 sunDirection = q * Vec3::Right();
					const ColorF sunColor{ 0.1 + Math::Pow(tf, 1.0 / 2.0) * (night ? 0.1 : 0.9) };

					Graphics3D::SetSunDirection(sunDirection);
					Graphics3D::SetSunColor(sunColor);
					Graphics3D::SetGlobalAmbientColor(ColorF{ sky.zenithColor });
				}

				// set sky color
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

				// set parameters
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
		}

		// [RenderTexture を 2D シーンに描画]
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			Shader::LinearToScreen(renderTexture);

         if ((not isEditorMode) && showBlacksmithMenu)
			{
				DrawSelectionIndicator(camera, BlacksmithPosition);
			}
		}

		// 天空レンダリングのパラメータ設定
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

			if (not birdMesh.loaded)
			{
				SimpleGUI::GetFont()(U"bird.glb load failed").draw(540, 396, ColorF{ 0.75, 0.2, 0.2 });
			}
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

		SimpleGUI::CheckBox(showUI, U"UI", Vec2{ 20, Scene::Height() - 100 });
		SimpleGUI::Slider(U"time: {:.2f}"_fmt(skyTime), skyTime, -2.0, 4.0, Vec2{ 20, Scene::Height() - 60 }, 120, Scene::Width() - 160);
	}
}
