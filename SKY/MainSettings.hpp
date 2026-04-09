# pragma once
# include "MainContext.hpp"

namespace MainSupport
{
	[[nodiscard]] CameraSettings LoadCameraSettings();
	bool SaveCameraSettings(const CameraSettings& settings);
	[[nodiscard]] ModelHeightSettings LoadModelHeightSettings();
	bool SaveModelHeightSettings(const ModelHeightSettings& settings);
   [[nodiscard]] UiLayoutSettings LoadUiLayoutSettings(int32 sceneWidth, int32 sceneHeight);
	bool SaveUiLayoutSettings(const UiLayoutSettings& settings);
   [[nodiscard]] UnitEditorSettings LoadUnitEditorSettings();
	bool SaveUnitEditorSettings(const UnitEditorSettings& settings);
}
