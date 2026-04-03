# include <Siv3D.hpp> // Siv3D v0.6.16

namespace
{
 constexpr double LegacyDefaultHouseModelScale = 0.03;
	constexpr double DefaultHouseModelScaleMultiplier = 1.0;
	constexpr double MinHouseModelScaleMultiplier = 0.25;
	constexpr double MaxHouseModelScaleMultiplier = 4.0;
	const FilePath HouseModelSettingsPath = U"settings/house_model_settings.toml";

 [[nodiscard]] double LoadPersistedHouseModelScaleMultiplier()
	{
		const TOMLReader toml{ HouseModelSettingsPath };
		if (!toml)
		{
          return DefaultHouseModelScaleMultiplier;
		}

		try
		{
           const int32 schemaVersion = toml[U"schemaVersion"].get<int32>();

			if (schemaVersion == 2)
			{
				return Clamp(toml[U"houseModelScaleMultiplier"].get<double>(), MinHouseModelScaleMultiplier, MaxHouseModelScaleMultiplier);
			}

			if (schemaVersion == 1)
			{
              const double legacyScale = toml[U"houseModelScale"].get<double>();
				return Clamp((legacyScale / LegacyDefaultHouseModelScale), MinHouseModelScaleMultiplier, MaxHouseModelScaleMultiplier);
			}

           return DefaultHouseModelScaleMultiplier;
		}
		catch (const std::exception&)
		{
          return DefaultHouseModelScaleMultiplier;
		}
	}

 [[nodiscard]] bool SavePersistedHouseModelScaleMultiplier(const double scaleMultiplier)
	{
		FileSystem::CreateDirectories(FileSystem::ParentPath(HouseModelSettingsPath));

		TextWriter writer{ HouseModelSettingsPath };
		if (!writer)
		{
			return false;
		}

       const double clampedScaleMultiplier = Clamp(scaleMultiplier, MinHouseModelScaleMultiplier, MaxHouseModelScaleMultiplier);
		writer.write(U"schemaVersion = 2\nhouseModelScaleMultiplier = {:.4f}\n"_fmt(clampedScaleMultiplier));
		return true;
	}
}

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(ColorF{ 0.55, 0.75, 0.95 });

	Font font{ FontMethod::MSDF, 24, Typeface::Bold };
  const Vec3 initialCameraEye{ 0, 28, -10 };
	const Vec3 initialCameraFocus{ 0, 0, 0 };
	DebugCamera3D camera{ Scene::Size(), 35_deg, initialCameraEye, initialCameraFocus };

  Sky sky;
	bool skyEnabled = true;
	double skyExposure = 1.0;
	Vec3 sunLightPosition{ 30.0, 50.0, 20.0 };
	double globalAmbientIntensity = 0.35;
	ColorF sunColor{ 1.0, 0.98, 0.9 };
	const Mesh ground{ MeshData::Box(Float3{ 80.0f, 0.5f, 80.0f }) };
	const Mesh block{ MeshData::Box(Float3{ 3.0f, 3.0f, 3.0f }) };
  const Mesh selectionMarker{ MeshData::Box(Float3{ 4.0f, 0.25f, 4.0f }) };
	const Mesh tallBlock{ MeshData::Box(Float3{ 2.0f, 8.0f, 2.0f }) };
	const Mesh sphere{ MeshData::Sphere(1.4, 32) };
	const FilePath houseModelPath = U"example/obj/mill.obj";
	const FilePath treeModelPath = U"example/obj/tree.obj";
	const bool hasHouseModel = FileSystem::Exists(houseModelPath);
	const bool hasTreeModel = FileSystem::Exists(treeModelPath);
	const Model houseModel = hasHouseModel ? Model{ houseModelPath } : Model{};
	const Model treeModel = hasTreeModel ? Model{ treeModelPath } : Model{};

	const PhongMaterial groundMaterial{ ColorF{ 0.35, 0.7, 0.4 } };
	const PhongMaterial blockMaterial{ ColorF{ 0.85, 0.75, 0.6 } };
 const PhongMaterial selectedBlockMaterial{ ColorF{ 1.0, 0.9, 0.45 } };
	const PhongMaterial towerMaterial{ ColorF{ 0.55, 0.65, 0.8 } };
	const PhongMaterial sphereMaterial{ ColorF{ 0.95, 0.5, 0.35 } };
  const Vec3 houseSize{ 3.0, 3.0, 3.0 };
    double houseModelScaleMultiplier = LoadPersistedHouseModelScaleMultiplier();
	const double treeModelScale = 0.04;
	const Array<String> buildOptions =
	{
		U"住居を強化",
		U"畑を建てる",
		U"倉庫を建てる",
	};

	const Array<Vec3> blockPositions =
	{
		{ -12, 1.5, 8 },
		{ -6, 1.5, 14 },
		{ 2, 1.5, 10 },
		{ 10, 1.5, 4 },
		{ 14, 1.5, -8 },
		{ 4, 1.5, -14 },
		{ -10, 1.5, -10 },
		{ 0, 1.5, 0 },
	};

	const Array<Vec3> towerPositions =
	{
		{ -18, 4.0, 18 },
		{ 18, 4.0, 18 },
		{ -18, 4.0, -18 },
		{ 18, 4.0, -18 },
	};

 const Array<Vec3> treePositions =
	{
		{ -16, 1.4, 0 },
		{ 16, 1.4, 0 },
		{ 0, 1.4, 16 },
		{ 0, 1.4, -16 },
	};

	const Box houseModelBounds = hasHouseModel ? houseModel.boundingBox() : Box{ Vec3{ 0, 0, 0 }, houseSize };
	const Box treeModelBounds = hasTreeModel ? treeModel.boundingBox() : Box{ Vec3{ 0, 0, 0 }, Vec3{ 2.8, 6.0, 2.8 } };
	const auto makeModelTransform = [](const Box& bounds, const Vec3& worldPos, const double scale, const double yaw)
	{
		const Vec3 localOffset
		{
			-bounds.center.x,
			-(bounds.center.y - (bounds.size.y * 0.5)),
			-bounds.center.z,
		};

		return Mat4x4::Identity()
			.translated(localOffset)
			.scaled(scale)
			.rotatedY(yaw)
			.translated(worldPos);
	};
	const double houseAutoNormalizeScale = hasHouseModel
		? Min(
			Min(
				houseSize.x / Max(houseModelBounds.size.x, 0.001),
				houseSize.y / Max(houseModelBounds.size.y, 0.001)
			),
			houseSize.z / Max(houseModelBounds.size.z, 0.001)
		)
		: 1.0;

	Optional<size_t> selectedHouse;
 double cameraMoveSpeed = 12.0;
	Optional<Vec3> wheelZoomFocus;
	int32 wheelZoomFocusFrames = 0;
	String lastBuildAction = U"家をクリックするとビルドメニューを開けます。";

	if (!FileSystem::Exists(HouseModelSettingsPath))
	{
      SavePersistedHouseModelScaleMultiplier(houseModelScaleMultiplier);
	}

	while (System::Update())
	{
     if (KeyUp.down())
		{
			cameraMoveSpeed = Min(cameraMoveSpeed + 2.0, 40.0);
		}

		if (KeyDown.down())
		{
			cameraMoveSpeed = Max(cameraMoveSpeed - 2.0, 2.0);
		}

		camera.update(cameraMoveSpeed);

		const RectF buildMenuRect{ Scene::Width() - 320, 110, 280, 240 };
        const RectF resetCameraRect{ Scene::Width() - 320, 20, 280, 50 };
        const RectF smallerHouseModelRect{ Scene::Width() - 320, 80, 136, 44 };
		const RectF largerHouseModelRect{ Scene::Width() - 176, 80, 136, 44 };
        const RectF environmentGuiRect{ 20, 248, 360, 440 };
		const bool cursorOnBuildMenu = (selectedHouse.has_value() && buildMenuRect.mouseOver());
        const bool cursorOnResetCameraButton = resetCameraRect.mouseOver();
        const bool cursorOnHouseScaleButton = (smallerHouseModelRect.mouseOver() || largerHouseModelRect.mouseOver());
        const bool cursorOnEnvironmentGui = environmentGuiRect.mouseOver();
		const double wheel = Mouse::Wheel();

		if (resetCameraRect.leftClicked())
		{
			camera.setView(initialCameraEye, initialCameraFocus);
			wheelZoomFocus.reset();
			wheelZoomFocusFrames = 0;
			lastBuildAction = U"カメラを俯瞰位置にリセットしました。";
		}

		if (smallerHouseModelRect.leftClicked())
		{
           houseModelScaleMultiplier = Clamp((houseModelScaleMultiplier / 1.2), MinHouseModelScaleMultiplier, MaxHouseModelScaleMultiplier);
			lastBuildAction = SavePersistedHouseModelScaleMultiplier(houseModelScaleMultiplier)
				? U"家モデル補正倍率を {:.2f} に保存しました。"_fmt(houseModelScaleMultiplier)
				: U"家モデル倍率の保存に失敗しました。";
		}

		if (largerHouseModelRect.leftClicked())
		{
           houseModelScaleMultiplier = Clamp((houseModelScaleMultiplier * 1.2), MinHouseModelScaleMultiplier, MaxHouseModelScaleMultiplier);
			lastBuildAction = SavePersistedHouseModelScaleMultiplier(houseModelScaleMultiplier)
				? U"家モデル補正倍率を {:.2f} に保存しました。"_fmt(houseModelScaleMultiplier)
				: U"家モデル倍率の保存に失敗しました。";
		}

      if ((wheel != 0.0) && (not cursorOnBuildMenu) && (not cursorOnEnvironmentGui))
		{
          if (not wheelZoomFocus.has_value())
			{
              Vec3 focusPos = camera.getFocusPosition();
				double nearestDistance = Math::Inf;
               const Ray cursorRay = camera.screenToRay(Scene::CenterF());

				for (const auto& pos : blockPositions)
				{
                   if (const auto distance = cursorRay.intersects(Box{ pos, houseSize }))
					{
						nearestDistance = Min(nearestDistance, static_cast<double>(*distance));
					}
				}

              for (const auto& pos : towerPositions)
				{
                   if (const auto distance = cursorRay.intersects(Box{ pos, Vec3{ 2.0, 8.0, 2.0 } }))
					{
						nearestDistance = Min(nearestDistance, static_cast<double>(*distance));
					}
				}

               for (const auto& pos : treePositions)
				{
                   if (const auto distance = cursorRay.intersects(Box{ pos, Vec3{ 2.8, 6.0, 2.8 } }))
					{
						nearestDistance = Min(nearestDistance, static_cast<double>(*distance));
					}
				}

                if (const auto distance = cursorRay.intersects(Box{ Vec3{ 0, -0.25, 0 }, Vec3{ 80.0, 0.5, 80.0 } }))
				{
					nearestDistance = Min(nearestDistance, static_cast<double>(*distance));
				}

                if (nearestDistance < Math::Inf)
				{
					focusPos = cursorRay.point_at(nearestDistance);
				}

				wheelZoomFocus = focusPos;
			}

            wheelZoomFocusFrames = 12;
			const Vec3 focusPos = *wheelZoomFocus;
			const Vec3 eyeOffset = (camera.getEyePosition() - focusPos);
			const double currentDistance = eyeOffset.length();

			if (0.001 < currentDistance)
			{
				const double targetDistance = Clamp(currentDistance * Pow(0.8, wheel), 2.5, 80.0);
				camera.setView(
					focusPos + (eyeOffset * (targetDistance / currentDistance)),
					focusPos
				);
			}
		}
       else if (wheelZoomFocus.has_value())
		{
			if (0 < wheelZoomFocusFrames)
			{
				--wheelZoomFocusFrames;
			}

			if (wheelZoomFocusFrames <= 0)
			{
				wheelZoomFocus.reset();
			}
		}

  if (MouseL.down() && (not cursorOnBuildMenu) && (not cursorOnResetCameraButton) && (not cursorOnHouseScaleButton) && (not cursorOnEnvironmentGui))
		{
			Optional<size_t> clickedHouse;
			double nearestDistance = Math::Inf;
			const Ray cursorRay = camera.screenToRay(Cursor::PosF());

			for (size_t i = 0; i < blockPositions.size(); ++i)
			{
				if (const auto distance = cursorRay.intersects(Box{ blockPositions[i], houseSize }))
				{
					if (*distance < nearestDistance)
					{
						nearestDistance = *distance;
						clickedHouse = i;
					}
				}
			}

			selectedHouse = clickedHouse;

			if (selectedHouse.has_value())
			{
				lastBuildAction = U"家 {} を選択中"_fmt(*selectedHouse + 1);
			}
		}

		Graphics3D::SetCameraTransform(camera);
      const Vec3 sunDirection = ((sunLightPosition.lengthSq() > 0.001)
			? (-sunLightPosition.normalized())
			: Vec3{ -0.6, -1.0, -0.4 }.normalized());
		Graphics3D::SetGlobalAmbientColor(ColorF{ globalAmbientIntensity });
		Graphics3D::SetSunDirection(sunDirection);
		Graphics3D::SetSunColor(sunColor);

     if (skyEnabled)
		{
			sky.draw(skyExposure);
		}
		ground.draw(Vec3{ 0, -0.25, 0 }, groundMaterial);
		const double effectiveHouseModelScale = (houseAutoNormalizeScale * houseModelScaleMultiplier);

        for (size_t i = 0; i < blockPositions.size(); ++i)
		{
          const bool isSelected = (selectedHouse.has_value() && (*selectedHouse == i));

			if (isSelected)
			{
				selectionMarker.draw(Vec3{ blockPositions[i].x, 0.125, blockPositions[i].z }, selectedBlockMaterial);
			}

			if (hasHouseModel)
			{
                houseModel.draw(makeModelTransform(houseModelBounds, Vec3{ blockPositions[i].x, 0.0, blockPositions[i].z }, effectiveHouseModelScale, (i * 37_deg)));
			}
			else
			{
				block.draw(blockPositions[i], (isSelected ? selectedBlockMaterial : blockMaterial));
			}
		}

		for (const auto& pos : towerPositions)
		{
			tallBlock.draw(pos, towerMaterial);
		}

     for (size_t i = 0; i < treePositions.size(); ++i)
		{
           if (hasTreeModel)
			{
				treeModel.draw(makeModelTransform(treeModelBounds, Vec3{ treePositions[i].x, 0.0, treePositions[i].z }, treeModelScale, (i * 53_deg)));
			}
			else
			{
				sphere.draw(treePositions[i], sphereMaterial);
			}
		}

		const Vec3 eyePos = camera.getEyePosition();
        font(U"初期視点は俯瞰です。WASD / Space / X / 右ドラッグ で移動").draw(20, 20, Palette::White);
      font(U"ホイール: 画面中央へズーム / ↑↓: 移動速度 {:.1f}"_fmt(cameraMoveSpeed)).draw(20, 52, Palette::White);
		font(U"Camera: ({:.1f}, {:.1f}, {:.1f})"_fmt(eyePos.x, eyePos.y, eyePos.z)).draw(20, 84, Palette::White);
		font(lastBuildAction).draw(20, 116, Palette::White);
		font(U"家: {} / 木: {}"_fmt(hasHouseModel ? U"App/3dmodel" : U"簡易メッシュ", hasTreeModel ? U"App/3dmodel" : U"簡易メッシュ")).draw(20, 148, Palette::White);
       if (hasHouseModel)
		{
			font(U"家bounds: ({:.1f}, {:.1f}, {:.1f})"_fmt(houseModelBounds.size.x, houseModelBounds.size.y, houseModelBounds.size.z)).draw(20, 180, Palette::White);
			font(U"家モデル補正 {:.2f} / 自動正規化 {:.3f} / 実効 {:.3f}"_fmt(houseModelScaleMultiplier, houseAutoNormalizeScale, effectiveHouseModelScale)).draw(20, 212, Palette::White);
		}
		resetCameraRect.rounded(8).draw(cursorOnResetCameraButton ? ColorF{ 0.38, 0.52, 0.72, 0.95 } : ColorF{ 0.24, 0.31, 0.42, 0.92 });
		resetCameraRect.rounded(8).drawFrame(1, 0, ColorF{ 0.9 });
		font(U"俯瞰位置にリセット").draw(resetCameraRect.x + 18, resetCameraRect.y + 10, Palette::White);
		smallerHouseModelRect.rounded(8).draw(smallerHouseModelRect.mouseOver() ? ColorF{ 0.38, 0.52, 0.72, 0.95 } : ColorF{ 0.24, 0.31, 0.42, 0.92 });
		smallerHouseModelRect.rounded(8).drawFrame(1, 0, ColorF{ 0.9 });
		font(U"家モデル -").draw(smallerHouseModelRect.x + 18, smallerHouseModelRect.y + 7, Palette::White);
		largerHouseModelRect.rounded(8).draw(largerHouseModelRect.mouseOver() ? ColorF{ 0.38, 0.52, 0.72, 0.95 } : ColorF{ 0.24, 0.31, 0.42, 0.92 });
		largerHouseModelRect.rounded(8).drawFrame(1, 0, ColorF{ 0.9 });
		font(U"家モデル +").draw(largerHouseModelRect.x + 18, largerHouseModelRect.y + 7, Palette::White);
        font(U"家モデル補正 {:.2f}"_fmt(houseModelScaleMultiplier)).draw(Scene::Width() - 320, 132, Palette::White);

		environmentGuiRect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.14, 0.82 });
		environmentGuiRect.rounded(8).drawFrame(1, 0, ColorF{ 0.9 });
		font(U"SKY / 光源パラメータ").draw(environmentGuiRect.x + 12, environmentGuiRect.y + 8, Palette::White);
		SimpleGUI::CheckBox(skyEnabled, U"Sky を描画", Vec2{ 32, 286 }, 160);
		SimpleGUI::CheckBox(sky.sunEnabled, U"Sun 表示", Vec2{ 192, 286 }, 160);
		SimpleGUI::CheckBox(sky.cloudsEnabled, U"Cloud 表示", Vec2{ 32, 322 }, 160);
		SimpleGUI::CheckBox(sky.cloudsLightingEnabled, U"Cloud Lighting", Vec2{ 192, 322 }, 160);
		SimpleGUI::Slider(U"Exposure", skyExposure, 0.1, 4.0, Vec2{ 32, 362 }, 110, 180);
		SimpleGUI::Slider(U"Cloud", sky.cloudiness, 0.0, 1.0, Vec2{ 32, 398 }, 110, 180);
		SimpleGUI::Slider(U"CScale", sky.cloudScale, 0.1, 3.0, Vec2{ 32, 434 }, 110, 180);
		SimpleGUI::Slider(U"CHeight", sky.cloudPlaneHeight, 100.0, 3000.0, Vec2{ 32, 470 }, 110, 180);
		SimpleGUI::Slider(U"FogSky", sky.fogHeightSky, 0.0, 1.0, Vec2{ 32, 506 }, 110, 180);
		SimpleGUI::Slider(U"SunX", sunLightPosition.x, -100.0, 100.0, Vec2{ 32, 542 }, 110, 180);
		SimpleGUI::Slider(U"SunY", sunLightPosition.y, 1.0, 100.0, Vec2{ 32, 578 }, 110, 180);
		SimpleGUI::Slider(U"SunZ", sunLightPosition.z, -100.0, 100.0, Vec2{ 32, 614 }, 110, 180);
		SimpleGUI::Slider(U"Ambient", globalAmbientIntensity, 0.0, 1.0, Vec2{ 32, 650 }, 110, 180);
		SimpleGUI::Slider(U"SunR", sunColor.r, 0.0, 2.0, Vec2{ 32, 686 }, 110, 180);
		SimpleGUI::Slider(U"SunG", sunColor.g, 0.0, 2.0, Vec2{ 192, 686 }, 80, 100);
		SimpleGUI::Slider(U"SunB", sunColor.b, 0.0, 2.0, Vec2{ 280, 686 }, 80, 100);

		if (selectedHouse.has_value())
		{
			buildMenuRect.draw(ColorF{ 0.12, 0.14, 0.18, 0.92 });
			buildMenuRect.drawFrame(2, Palette::White);
			font(U"家 {} のビルド"_fmt(*selectedHouse + 1)).draw(buildMenuRect.x + 20, buildMenuRect.y + 16, Palette::White);

			for (size_t i = 0; i < buildOptions.size(); ++i)
			{
				const RectF buttonRect{ buildMenuRect.x + 20, buildMenuRect.y + 58 + (i * 56), buildMenuRect.w - 40, 42 };
				const bool hovered = buttonRect.mouseOver();
				buttonRect.rounded(8).draw(hovered ? ColorF{ 0.38, 0.52, 0.72 } : ColorF{ 0.24, 0.31, 0.42 });
				buttonRect.rounded(8).drawFrame(1, 0, ColorF{ 0.9 });
				font(buildOptions[i]).draw(buttonRect.x + 16, buttonRect.y + 8, Palette::White);
			}
		}
	}
}
