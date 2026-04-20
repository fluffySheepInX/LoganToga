# pragma once
# include <array>
# include "BirdModelTypes.hpp"
# include "UnitContext.hpp"

namespace MainSupport
{
   enum class UnitModelAnimationRole
	{
		Idle,
		Move,
		Attack,
		Count,
	};

	inline constexpr size_t UnitModelAnimationRoleCount = static_cast<size_t>(UnitModelAnimationRole::Count);

	[[nodiscard]] inline constexpr size_t GetUnitModelAnimationRoleIndex(const UnitModelAnimationRole role)
	{
		return static_cast<size_t>(role);
	}

	struct CameraSettings
	{
		Vec3 eye = DefaultCameraEye;
		Vec3 focus = DefaultCameraFocus;
	};

	struct ModelHeightSettings
	{
		struct FileSettings
		{
			double offsetY = 0.0;
			double scale = 1.0;
			std::array<int32, UnitModelAnimationRoleCount> animationClip{ -1, -1, -1 };
		};

	   std::array<double, UnitRenderModelCount> offsetY{};
		std::array<double, UnitRenderModelCount> scale{};
		std::array<std::array<int32, UnitRenderModelCount>, UnitModelAnimationRoleCount> animationClip{};
		std::array<double, 3> tireTrackYOffset{};
		std::array<double, 3> tireTrackOpacity{};
		std::array<double, 3> tireTrackSoftness{};
		std::array<double, 3> tireTrackWarmth{};
		HashTable<String, FileSettings> fileSettings;

		ModelHeightSettings()
		{
			scale.fill(1.0);
			for (auto& clips : animationClip) { clips.fill(-1); }
		   tireTrackYOffset = { 0.018, 0.019, 0.020 };
		   tireTrackOpacity.fill(1.0);
			tireTrackSoftness.fill(0.0);
			tireTrackWarmth.fill(0.35);
		}
	};

	inline constexpr double ModelScaleMin = 0.25;
	inline constexpr double ModelScaleMax = 4.0;
	inline constexpr double TireTrackYOffsetMin = -0.100;
	inline constexpr double TireTrackYOffsetMax = 0.100;
	inline constexpr double TireTrackOpacityMin = 0.0;
	inline constexpr double TireTrackOpacityMax = 1.0;
	inline constexpr double TireTrackSoftnessMin = 0.0;
	inline constexpr double TireTrackSoftnessMax = 1.0;
	inline constexpr double TireTrackWarmthMin = 0.0;
	inline constexpr double TireTrackWarmthMax = 1.0;

	enum class TireTrackTextureSegment
	{
		Start,
		Middle,
		End,
	};

	struct UnitRenderModelDefinition
	{
		StringView label;
		Vec3 displayPosition{ 0, 0, 0 };
		ColorF previewColor{ 1.0, 1.0, 1.0, 1.0 };
      UnitModelProceduralAnimationType proceduralAnimationType = UnitModelProceduralAnimationType::None;
	};

	[[nodiscard]] inline const UnitRenderModelDefinition& GetUnitRenderModelDefinition(const UnitRenderModel renderModel)
	{
        static const std::array<UnitRenderModelDefinition, UnitRenderModelCount> Definitions{
			UnitRenderModelDefinition{
				.label = U"bird",
				.displayPosition = BirdDisplayPosition,
				.previewColor = ColorF{ 0.92, 0.95, 1.0 },
              .proceduralAnimationType = UnitModelProceduralAnimationType::BirdWingFlap,
			},
			UnitRenderModelDefinition{
				.label = U"ashigaru",
				.displayPosition = AshigaruDisplayPosition,
				.previewColor = ColorF{ 0.95, 0.92, 0.90 },
			},
			UnitRenderModelDefinition{
				.label = U"sugoiCar",
				.displayPosition = SugoiCarDisplayPosition,
				.previewColor = ColorF{ 0.96, 0.94, 0.92 },
			},
          UnitRenderModelDefinition{
				.label = U"hohei",
				.displayPosition = HoheiDisplayPosition,
				.previewColor = ColorF{ 0.95, 0.93, 0.91 },
			},
		};

		return Definitions[GetUnitRenderModelIndex(renderModel)];
	}

    [[nodiscard]] inline StringView GetUnitRenderModelLabel(const UnitRenderModel renderModel)
	{
        return GetUnitRenderModelDefinition(renderModel).label;
	}

 [[nodiscard]] inline Vec3 GetUnitRenderModelDisplayPosition(const UnitRenderModel renderModel)
	{
       return GetUnitRenderModelDefinition(renderModel).displayPosition;
	}

	[[nodiscard]] inline ColorF GetUnitRenderModelPreviewColor(const UnitRenderModel renderModel)
	{
		return GetUnitRenderModelDefinition(renderModel).previewColor;
	}

	[[nodiscard]] inline UnitModelProceduralAnimationType GetUnitRenderModelProceduralAnimationType(const UnitRenderModel renderModel)
	{
		return GetUnitRenderModelDefinition(renderModel).proceduralAnimationType;
	}

	[[nodiscard]] inline FilePathView GetUnitRenderModelDefaultModelPath(const UnitRenderModel renderModel)
	{
		switch (renderModel)
		{
		case UnitRenderModel::Ashigaru:
			return AshigaruModelPath;

		case UnitRenderModel::SugoiCar:
			return SugoiCarModelPath;

		case UnitRenderModel::Hohei:
			return HoheiModelPath;

		case UnitRenderModel::Bird:
		default:
			return BirdModelPath;
		}
	}

	[[nodiscard]] inline String MakeModelHeightFileKey(FilePathView modelPath)
	{
		return FileSystem::FileName(FilePath{ modelPath }).lowercased();
	}

	[[nodiscard]] inline Optional<UnitRenderModel> TryGetDefaultModelRenderModel(FilePathView modelPath)
	{
		const String key = MakeModelHeightFileKey(modelPath);
		if (key == FileSystem::FileName(FilePath{ BirdModelPath }).lowercased())
		{
			return UnitRenderModel::Bird;
		}

		if (key == FileSystem::FileName(FilePath{ AshigaruModelPath }).lowercased())
		{
			return UnitRenderModel::Ashigaru;
		}

		if (key == FileSystem::FileName(FilePath{ SugoiCarModelPath }).lowercased())
		{
			return UnitRenderModel::SugoiCar;
		}

		if (key == FileSystem::FileName(FilePath{ HoheiModelPath }).lowercased())
		{
			return UnitRenderModel::Hohei;
		}

		return none;
	}

	[[nodiscard]] inline ModelHeightSettings::FileSettings& GetOrCreateModelHeightFileSettings(ModelHeightSettings& settings, FilePathView modelPath)
	{
		return settings.fileSettings[MakeModelHeightFileKey(modelPath)];
	}

	[[nodiscard]] inline const ModelHeightSettings::FileSettings* FindModelHeightFileSettings(const ModelHeightSettings& settings, FilePathView modelPath)
	{
		const String key = MakeModelHeightFileKey(modelPath);
		if (const auto it = settings.fileSettings.find(key); it != settings.fileSettings.end())
		{
			return std::addressof(it->second);
		}

		return nullptr;
	}

	[[nodiscard]] inline constexpr size_t GetTireTrackTextureSegmentIndex(const TireTrackTextureSegment segment)
	{
		switch (segment)
		{
		case TireTrackTextureSegment::Start:
			return 0;

		case TireTrackTextureSegment::Middle:
			return 1;

		case TireTrackTextureSegment::End:
		default:
			return 2;
		}
	}

	[[nodiscard]] inline StringView GetTireTrackTextureSegmentLabel(const TireTrackTextureSegment segment)
	{
		switch (segment)
		{
		case TireTrackTextureSegment::Start:
			return U"start";

		case TireTrackTextureSegment::Middle:
			return U"middle";

		case TireTrackTextureSegment::End:
		default:
			return U"end";
		}
	}

	[[nodiscard]] inline constexpr std::array<TireTrackTextureSegment, 3> GetTireTrackTextureSegments()
	{
		return{
			TireTrackTextureSegment::Start,
			TireTrackTextureSegment::Middle,
			TireTrackTextureSegment::End,
		};
	}

  [[nodiscard]] inline double& GetModelHeightOffset(ModelHeightSettings& settings, const UnitRenderModel renderModel)
	{
     return settings.offsetY[GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline double GetModelHeightOffset(const ModelHeightSettings& settings, const UnitRenderModel renderModel)
	{
		return settings.offsetY[GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline double& GetModelScale(ModelHeightSettings& settings, const UnitRenderModel renderModel)
	{
		return settings.scale[GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline double GetModelScale(const ModelHeightSettings& settings, const UnitRenderModel renderModel)
	{
		return settings.scale[GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline double& GetModelHeightOffset(ModelHeightSettings& settings, FilePathView modelPath)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelHeightOffset(settings, *renderModel);
		}

		return GetOrCreateModelHeightFileSettings(settings, modelPath).offsetY;
	}

	[[nodiscard]] inline double GetModelHeightOffset(const ModelHeightSettings& settings, FilePathView modelPath)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelHeightOffset(settings, *renderModel);
		}

		if (const auto* fileSettings = FindModelHeightFileSettings(settings, modelPath))
		{
			return fileSettings->offsetY;
		}

		return 0.0;
	}

	[[nodiscard]] inline double& GetModelScale(ModelHeightSettings& settings, FilePathView modelPath)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelScale(settings, *renderModel);
		}

		return GetOrCreateModelHeightFileSettings(settings, modelPath).scale;
	}

	[[nodiscard]] inline double GetModelScale(const ModelHeightSettings& settings, FilePathView modelPath)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelScale(settings, *renderModel);
		}

		if (const auto* fileSettings = FindModelHeightFileSettings(settings, modelPath))
		{
			return fileSettings->scale;
		}

		return 1.0;
	}

	[[nodiscard]] inline int32& GetModelAnimationClipIndex(ModelHeightSettings& settings, const UnitRenderModel renderModel, const UnitModelAnimationRole role)
	{
		return settings.animationClip[GetUnitModelAnimationRoleIndex(role)][GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline int32 GetModelAnimationClipIndex(const ModelHeightSettings& settings, const UnitRenderModel renderModel, const UnitModelAnimationRole role)
	{
		return settings.animationClip[GetUnitModelAnimationRoleIndex(role)][GetUnitRenderModelIndex(renderModel)];
	}

	[[nodiscard]] inline int32& GetModelAnimationClipIndex(ModelHeightSettings& settings, FilePathView modelPath, const UnitModelAnimationRole role)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelAnimationClipIndex(settings, *renderModel, role);
		}

		return GetOrCreateModelHeightFileSettings(settings, modelPath).animationClip[GetUnitModelAnimationRoleIndex(role)];
	}

	[[nodiscard]] inline int32 GetModelAnimationClipIndex(const ModelHeightSettings& settings, FilePathView modelPath, const UnitModelAnimationRole role)
	{
		if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
		{
			return GetModelAnimationClipIndex(settings, *renderModel, role);
		}

		if (const auto* fileSettings = FindModelHeightFileSettings(settings, modelPath))
		{
			return fileSettings->animationClip[GetUnitModelAnimationRoleIndex(role)];
		}

		return -1;
	}

	[[nodiscard]] inline double& GetTireTrackYOffset(ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackYOffset[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double GetTireTrackYOffset(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackYOffset[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double& GetTireTrackOpacity(ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackOpacity[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double GetTireTrackOpacity(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackOpacity[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double& GetTireTrackSoftness(ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackSoftness[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double GetTireTrackSoftness(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackSoftness[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double& GetTireTrackWarmth(ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackWarmth[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double GetTireTrackWarmth(const ModelHeightSettings& settings, const TireTrackTextureSegment segment)
	{
		return settings.tireTrackWarmth[GetTireTrackTextureSegmentIndex(segment)];
	}

	[[nodiscard]] inline double GetModelScaleForUnit(const ModelHeightSettings& settings, const UnitTeam team, const SapperUnitType unitType)
	{
        return GetModelScale(settings, GetUnitRenderModel(team, unitType));
	}

	[[nodiscard]] inline UnitRenderModel GetSpawnedSapperRenderModel(const SpawnedSapper& sapper)
	{
		return GetUnitRenderModel(sapper.team, sapper.unitType);
	}

	[[nodiscard]] inline double GetSpawnedSapperModelScale(const ModelHeightSettings& settings, const SpawnedSapper& sapper)
	{
		return GetModelScaleForUnit(settings, sapper.team, sapper.unitType);
	}
}
