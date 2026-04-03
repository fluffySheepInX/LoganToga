# pragma once
# include <Siv3D.hpp>

namespace MainSupport
{
	inline constexpr FilePathView CameraSettingsPath = U"settings/camera_settings.toml";
	inline constexpr FilePathView MapDataPath = U"settings/map_data.toml";
	inline constexpr FilePathView ModelHeightSettingsPath = U"settings/model_height_settings.toml";
	inline constexpr FilePathView BirdModelPath = U"model/bird.glb";
	inline constexpr FilePathView AshigaruModelPath = U"model/ashigaru_v2.1.glb";
	inline constexpr Vec3 DefaultCameraEye{ 0, 3, -16 };
	inline constexpr Vec3 DefaultCameraFocus{ 0, 0, 0 };
	inline constexpr Vec3 BirdDisplayPosition{ -10.5, 0, -2.5 };
	inline constexpr Vec3 AshigaruDisplayPosition{ -5.5, 0, -2.5 };
	inline constexpr double BirdDisplayYaw = 0_deg;
	inline constexpr Vec3 BlacksmithPosition{ 8, 0, 4 };
	inline constexpr Sphere BlacksmithInteractionSphere{ BlacksmithPosition + Vec3{ 0, 4.0, 0 }, 4.5 };
	inline constexpr Vec3 BlacksmithSelectionBoxSize{ 8.0, 8.0, 8.0 };
	inline constexpr Vec3 BlacksmithSelectionBoxPadding{ 1.2, 0.8, 1.2 };
	inline constexpr double CameraZoomMinDistance = 3.0;
	inline constexpr double CameraZoomMaxDistance = 80.0;
	inline constexpr double CameraZoomFactorPerWheelStep = 0.85;
	inline constexpr double BirdDisplayHeight = 3.6;
	inline constexpr double ModelHeightOffsetMin = -10.0;
	inline constexpr double ModelHeightOffsetMax = 10.0;

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

	struct ModelHeightSettings
	{
		double birdOffsetY = 0.0;
		double ashigaruOffsetY = 0.0;
	};

	enum class AppMode
	{
		Play,
		EditMap,
	};
}
