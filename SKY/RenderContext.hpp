# pragma once
# include <array>
# include "BirdModelTypes.hpp"
# include "UnitContext.hpp"

namespace MainSupport
{
	struct CameraSettings
	{
		Vec3 eye = DefaultCameraEye;
		Vec3 focus = DefaultCameraFocus;
	};

	struct ModelHeightSettings
	{
       std::array<double, UnitRenderModelCount> offsetY{};
		std::array<double, UnitRenderModelCount> scale{};

		ModelHeightSettings()
		{
			scale.fill(1.0);
		}
	};

	inline constexpr double ModelScaleMin = 0.25;
	inline constexpr double ModelScaleMax = 4.0;

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
