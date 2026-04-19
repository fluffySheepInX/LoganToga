# include "SkyAppLoopInternal.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
   UnitModel& SkyAppResources::GetModelHeightEditorPreviewModel(const size_t index, const UnitModelAnimationRole role)
	{
		switch (role)
		{
		case UnitModelAnimationRole::Move:
			return modelHeightEditorMoveUnitRenderModels[index];

		case UnitModelAnimationRole::Attack:
			return modelHeightEditorAttackUnitRenderModels[index];

		case UnitModelAnimationRole::Idle:
		default:
			return modelHeightEditorIdleUnitRenderModels[index];
		}
	}

  const UnitModel& SkyAppResources::GetModelHeightEditorPreviewModel(const size_t index, const UnitModelAnimationRole role) const
	{
		switch (role)
		{
		case UnitModelAnimationRole::Move:
			return modelHeightEditorMoveUnitRenderModels[index];

		case UnitModelAnimationRole::Attack:
			return modelHeightEditorAttackUnitRenderModels[index];

		case UnitModelAnimationRole::Idle:
		default:
			return modelHeightEditorIdleUnitRenderModels[index];
		}
	}

   StringView SkyAppResources::GetModelHeightEditorPreviewLabel(const size_t index) const
	{
        return modelHeightEditorPreviewLabels[index];
	}

	namespace
	{
		namespace Assets
		{
			inline constexpr StringView GrassPatchModelFileName = U"grass2.obj";
			inline constexpr StringView SkyTextureDirectory = U"texture/";
			inline constexpr StringView SkyModelDirectory = U"model/";
			inline constexpr StringView GroundTextureFileName = U"ground.jpg";
			inline constexpr StringView RoadTextureFileName = U"road.jpg";
			inline constexpr StringView TireTrackTextureFileName = U"taiya.png";
			inline constexpr StringView BlacksmithModelFileName = U"blacksmith.obj";
			inline constexpr StringView MillModelFileName = U"mill.obj";
			inline constexpr StringView TreeModelFileName = U"tree.obj";
			inline constexpr StringView PineModelFileName = U"pine.obj";
			inline constexpr StringView SugoiCarModelBaseName = U"sugoiCar";

			struct TireTrackTextureLoadResult
			{
				Image image;
				String warning;
			};

			[[nodiscard]] FilePath FindFirstExistingPath(const Array<FilePath>& candidates, const FilePath& fallback)
			{
				for (const auto& candidate : candidates)
				{
					if (FileSystem::Exists(candidate))
					{
						return candidate;
					}
				}

				return fallback;
			}

			[[nodiscard]] FilePath ResolveTexturePath(const StringView fileName)
			{
				const Array<FilePath> candidates{
					FilePath{ SkyTextureDirectory } + fileName,
					FilePath{ fileName },
				};

				return FindFirstExistingPath(candidates, FilePath{ SkyTextureDirectory } + fileName);
			}

			[[nodiscard]] FilePath ResolveModelPath(const StringView fileName)
			{
				const Array<FilePath> candidates{
					FilePath{ SkyModelDirectory } + fileName,
					FilePath{ fileName },
				};

				return FindFirstExistingPath(candidates, FilePath{ SkyModelDirectory } + fileName);
			}

			[[nodiscard]] FilePath ResolveGrassPatchModelPath()
			{
				const Array<FilePath> candidates{
					FilePath{ SkyModelDirectory } + GrassPatchModelFileName,
					FilePath{ GrassPatchModelFileName },
				};

				return FindFirstExistingPath(candidates, FilePath{ SkyModelDirectory } + GrassPatchModelFileName);
			}

			[[nodiscard]] Image MakeMissingTireTrackWarningImage()
			{
				Image image{ Size{ 96, 96 }, Color{ 0, 0, 0, 0 } };

				for (int32 y = 0; y < image.height(); ++y)
				{
					for (int32 x = 0; x < image.width(); ++x)
					{
						const bool stripe = ((((x + y) / 8) % 2) == 0);
						image[Point{ x, y }] = stripe
							? Color{ 255, 80, 180, 220 }
							: Color{ 255, 220, 40, 220 };
					}
				}

				return image;
			}

			[[nodiscard]] TireTrackTextureLoadResult LoadTireTrackSourceImage()
			{
				const FilePath resolvedPath = ResolveTexturePath(TireTrackTextureFileName);
				Image loaded{ resolvedPath };

				if ((0 < loaded.width()) && (96 <= loaded.height()))
				{
					return TireTrackTextureLoadResult{ .image = std::move(loaded) };
				}

				const String reason = ((0 < loaded.width()) || (0 < loaded.height()))
					? U"invalid size"
					: U"file not found or unreadable";
				return TireTrackTextureLoadResult{
					.image = MakeMissingTireTrackWarningImage(),
					.warning = U"Tire track texture load failed: {} ({}, cwd={})"_fmt(resolvedPath, reason, FileSystem::CurrentDirectory()),
				};
			}

			[[nodiscard]] const TireTrackTextureLoadResult& TireTrackSourceData()
			{
				static const TireTrackTextureLoadResult result = LoadTireTrackSourceImage();
				return result;
			}

			[[nodiscard]] const Image& TireTrackSourceImage()
			{
				return TireTrackSourceData().image;
			}

			[[nodiscard]] const String& TireTrackLoadWarning()
			{
				return TireTrackSourceData().warning;
			}

			[[nodiscard]] Texture LoadTireTrackSegmentTexture(const int32 y)
			{
				const Image& image = TireTrackSourceImage();
				const Rect segmentRect{ 0, y, image.width(), Min(32, image.height() - y) };
				Image segment = image.clipped(segmentRect);
				for (Color& pixel : segment)
				{
					if (pixel.a == 0)
					{
						continue;
					}

					pixel.r = 255;
					pixel.g = 255;
					pixel.b = 255;
				}
				return Texture{ segment, TextureDesc::MippedSRGB };
			}

			[[nodiscard]] FilePath ResolveSugoiCarModelPath()
			{
				const Array<FilePath> candidates{
					ResolveModelPath(SugoiCarModelBaseName + U".glb"),
					ResolveModelPath(SugoiCarModelBaseName + U".obj"),
					FilePath{ SugoiCarModelBaseName } + U".glb",
					FilePath{ SugoiCarModelBaseName } + U".obj",
				};

				return FindFirstExistingPath(candidates, FilePath{ SkyModelDirectory } + SugoiCarModelBaseName + U".glb");
			}

			[[nodiscard]] bool HasGlbExtension(const FilePath& path)
			{
				return FileSystem::FileName(path).lowercased().ends_with(U".glb");
			}

			void AppendUniqueFullPath(Array<FilePath>& paths, const FilePath& candidate)
			{
				const FilePath fullPath = FileSystem::FullPath(candidate);
				if (std::find(paths.begin(), paths.end(), fullPath) == paths.end())
				{
					paths << fullPath;
				}
			}

			void AppendConfiguredUnitEditorModelPaths(Array<FilePath>& paths)
			{
				const UnitEditorSettings unitEditorSettings = LoadUnitEditorSettings();
				for (const auto& modelPath : unitEditorSettings.modelPaths)
				{
					if (modelPath.isEmpty() || (not FileSystem::Exists(modelPath)))
					{
						continue;
					}

					AppendUniqueFullPath(paths, modelPath);
				}
			}

			[[nodiscard]] std::array<FilePath, UnitRenderModelCount> DefaultModelHeightEditorPreviewPaths()
			{
				return {
					FilePath{ BirdModelPath },
					FilePath{ AshigaruModelPath },
					ResolveSugoiCarModelPath(),
					FilePath{ HoheiModelPath },
				};
			}

			[[nodiscard]] Array<FilePath> DiscoverGlbModelPaths()
			{
				Array<FilePath> discoveredPaths;
				const FilePath modelDirectory = FilePath{ SkyModelDirectory };

				if (FileSystem::IsDirectory(modelDirectory))
				{
					for (const auto& path : FileSystem::DirectoryContents(modelDirectory))
					{
						if (FileSystem::IsFile(path) && HasGlbExtension(path))
						{
							AppendUniqueFullPath(discoveredPaths, path);
						}
					}
				}

				AppendConfiguredUnitEditorModelPaths(discoveredPaths);

				std::sort(discoveredPaths.begin(), discoveredPaths.end(), [](const FilePath& a, const FilePath& b)
					{
						return FileSystem::FileName(a) < FileSystem::FileName(b);
					});

				return discoveredPaths;
			}

			[[nodiscard]] bool IsDefaultModelHeightEditorPreviewPath(const FilePath& path, const std::array<FilePath, UnitRenderModelCount>& defaultPaths)
			{
				const FilePath fullPath = FileSystem::FullPath(path);
				for (const auto& defaultPath : defaultPaths)
				{
					if ((not defaultPath.isEmpty()) && (FileSystem::FullPath(defaultPath) == fullPath))
					{
						return true;
					}
				}

				return false;
			}

         [[nodiscard]] Array<FilePath> ResolveModelHeightEditorPreviewPaths()
			{
               const auto defaultPaths = DefaultModelHeightEditorPreviewPaths();
				const Array<FilePath> discoveredPaths = DiscoverGlbModelPaths();
				Array<FilePath> prioritizedPaths;
				prioritizedPaths.reserve((discoveredPaths.size() + defaultPaths.size()));

				for (const auto& path : discoveredPaths)
				{
					if (not IsDefaultModelHeightEditorPreviewPath(path, defaultPaths))
					{
						AppendUniqueFullPath(prioritizedPaths, path);
					}
				}

				for (const auto& path : discoveredPaths)
				{
					if (IsDefaultModelHeightEditorPreviewPath(path, defaultPaths))
					{
						AppendUniqueFullPath(prioritizedPaths, path);
					}
				}

				for (const auto& defaultPath : defaultPaths)
				{
					AppendUniqueFullPath(prioritizedPaths, defaultPath);
				}

              if (prioritizedPaths.isEmpty())
				{
					for (const auto& defaultPath : defaultPaths)
					{
						AppendUniqueFullPath(prioritizedPaths, defaultPath);
					}
				}

				return prioritizedPaths;
			}

          [[nodiscard]] Array<String> BuildModelHeightEditorPreviewLabels(const Array<FilePath>& previewPaths)
			{
                Array<String> labels;
				labels.reserve(previewPaths.size());
				for (const auto& previewPath : previewPaths)
				{
                  String label = FileSystem::FileName(previewPath);
					if (label.isEmpty())
					{
                     label = U"unknown.glb";
					}
                   labels << std::move(label);
				}

				return labels;
			}

          [[nodiscard]] Array<UnitModel> BuildModelHeightEditorPreviewModels(const Array<FilePath>& previewPaths)
			{
				Array<UnitModel> models;
               models.reserve(previewPaths.size());

				for (const auto& previewPath : previewPaths)
				{
                  const auto renderModel = TryGetDefaultModelRenderModel(previewPath).value_or(UnitRenderModel::Bird);
					models.emplace_back(previewPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(renderModel));
				}

				return models;
			}
		}
	}

	SkyAppResources::SkyAppResources()
		: groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ Assets::ResolveTexturePath(Assets::GroundTextureFileName), TextureDesc::MippedSRGB }
		, roadTexture{ Assets::ResolveTexturePath(Assets::RoadTextureFileName), TextureDesc::MippedSRGB }
		, tireTrackStartTexture{ Assets::LoadTireTrackSegmentTexture(64) }
		, tireTrackMiddleTexture{ Assets::LoadTireTrackSegmentTexture(32) }
		, tireTrackEndTexture{ Assets::LoadTireTrackSegmentTexture(0) }
		, blacksmithModel{ Assets::ResolveModelPath(Assets::BlacksmithModelFileName) }
		, millModel{ Assets::ResolveModelPath(Assets::MillModelFileName) }
		, treeModel{ Assets::ResolveModelPath(Assets::TreeModelFileName) }
		, pineModel{ Assets::ResolveModelPath(Assets::PineModelFileName) }
		, grassPatchModel{ Assets::ResolveGrassPatchModelPath() }
        , modelHeightEditorPreviewPaths{ Assets::ResolveModelHeightEditorPreviewPaths() }
		, modelHeightEditorPreviewLabels{ Assets::BuildModelHeightEditorPreviewLabels(modelHeightEditorPreviewPaths) }
      , idleUnitRenderModels{ {
			UnitModel{ BirdModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Bird) },
			UnitModel{ AshigaruModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Ashigaru) },
			UnitModel{ Assets::ResolveSugoiCarModelPath(), BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::SugoiCar) },
			UnitModel{ HoheiModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Hohei) },
		} }
		, moveUnitRenderModels{ {
			UnitModel{ BirdModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Bird) },
			UnitModel{ AshigaruModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Ashigaru) },
			UnitModel{ Assets::ResolveSugoiCarModelPath(), BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::SugoiCar) },
			UnitModel{ HoheiModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Hohei) },
		} }
		, attackUnitRenderModels{ {
			UnitModel{ BirdModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Bird) },
			UnitModel{ AshigaruModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Ashigaru) },
			UnitModel{ Assets::ResolveSugoiCarModelPath(), BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::SugoiCar) },
			UnitModel{ HoheiModelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(UnitRenderModel::Hohei) },
		} }
     , modelHeightEditorIdleUnitRenderModels{ Assets::BuildModelHeightEditorPreviewModels(modelHeightEditorPreviewPaths) }
		, modelHeightEditorMoveUnitRenderModels{ Assets::BuildModelHeightEditorPreviewModels(modelHeightEditorPreviewPaths) }
		, modelHeightEditorAttackUnitRenderModels{ Assets::BuildModelHeightEditorPreviewModels(modelHeightEditorPreviewPaths) }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
	{
		if (not Assets::TireTrackLoadWarning().isEmpty())
		{
			loadWarnings << Assets::TireTrackLoadWarning();
		}

		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(grassPatchModel, TextureDesc::MippedSRGB);
	}
}
