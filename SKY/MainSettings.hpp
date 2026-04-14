# pragma once
# include "MainContextTypes.hpp"
# include "MainContext.hpp"

namespace MainSupport
{
	[[nodiscard]] CameraSettings LoadCameraSettings();
	bool SaveCameraSettings(const CameraSettings& settings);
	[[nodiscard]] ModelHeightSettings LoadModelHeightSettings();
	bool SaveModelHeightSettings(const ModelHeightSettings& settings);
    [[nodiscard]] EditorTextColorSettings LoadEditorTextColorSettings();
	bool SaveEditorTextColorSettings(const EditorTextColorSettings& settings);
	[[nodiscard]] const EditorTextColorSettings& GetEditorTextColorSettings();
	EditorTextColorSettings& GetMutableEditorTextColorSettings();
	void ResetEditorTextColorSettings();
   [[nodiscard]] UiLayoutSettings LoadUiLayoutSettings(int32 sceneWidth, int32 sceneHeight);
	bool SaveUiLayoutSettings(const UiLayoutSettings& settings);
   [[nodiscard]] ResourceStock LoadInitialPlayerResources();
	bool SaveInitialPlayerResources(const ResourceStock& resources);
   [[nodiscard]] UnitEditorSettings LoadUnitEditorSettings();
	bool SaveUnitEditorSettings(const UnitEditorSettings& settings);
}
