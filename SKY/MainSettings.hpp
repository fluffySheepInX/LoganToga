# pragma once
# include "MainContext.hpp"

namespace MainSupport
{
	[[nodiscard]] CameraSettings LoadCameraSettings();
	bool SaveCameraSettings(const CameraSettings& settings);
	[[nodiscard]] ModelHeightSettings LoadModelHeightSettings();
	bool SaveModelHeightSettings(const ModelHeightSettings& settings);
}
