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
	};

	struct CameraSettings
	{
		Vec3 eye = DefaultCameraEye;
		Vec3 focus = DefaultCameraFocus;
	};

	struct ModelHeightSettings
	{
       std::array<double, UnitRenderModelCount> offsetY{};
		std::array<double, UnitRenderModelCount> scale{};
     std::array<int32, UnitRenderModelCount> idleAnimationClip{};
		std::array<int32, UnitRenderModelCount> moveAnimationClip{};
		std::array<int32, UnitRenderModelCount> attackAnimationClip{};
		std::array<double, 3> tireTrackYOffset{};
		std::array<double, 3> tireTrackOpacity{};
		std::array<double, 3> tireTrackSoftness{};
		std::array<double, 3> tireTrackWarmth{};

		ModelHeightSettings()
		{
			scale.fill(1.0);
          idleAnimationClip.fill(-1);
			moveAnimationClip.fill(-1);
			attackAnimationClip.fill(-1);
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

    [[nodiscard]] inline int32& GetModelAnimationClipIndex(ModelHeightSettings& settings, const UnitRenderModel renderModel, const UnitModelAnimationRole role)
	{
        switch (role)
		{
		case UnitModelAnimationRole::Move:
			return settings.moveAnimationClip[GetUnitRenderModelIndex(renderModel)];

		case UnitModelAnimationRole::Attack:
			return settings.attackAnimationClip[GetUnitRenderModelIndex(renderModel)];

		case UnitModelAnimationRole::Idle:
		default:
			return settings.idleAnimationClip[GetUnitRenderModelIndex(renderModel)];
		}
	}

   [[nodiscard]] inline int32 GetModelAnimationClipIndex(const ModelHeightSettings& settings, const UnitRenderModel renderModel, const UnitModelAnimationRole role)
	{
        switch (role)
		{
		case UnitModelAnimationRole::Move:
			return settings.moveAnimationClip[GetUnitRenderModelIndex(renderModel)];

		case UnitModelAnimationRole::Attack:
			return settings.attackAnimationClip[GetUnitRenderModelIndex(renderModel)];

		case UnitModelAnimationRole::Idle:
		default:
			return settings.idleAnimationClip[GetUnitRenderModelIndex(renderModel)];
		}
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
