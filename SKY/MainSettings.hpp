# pragma once
# include "MainContextTypes.hpp"
# include "MainContext.hpp"

namespace MainSupport
{
  enum class PanelSkinTarget
	{
		Default,
       Settings,
      CameraSettings,
		Hud,
		MapEditor,
      UnitEditor,
		ToolModal,
	};

	[[nodiscard]] CameraSettings LoadCameraSettings();
	bool SaveCameraSettings(const CameraSettings& settings);
	[[nodiscard]] ModelHeightSettings LoadModelHeightSettings();
	bool SaveModelHeightSettings(const ModelHeightSettings& settings);
    [[nodiscard]] EditorTextColorSettings LoadEditorTextColorSettings();
	bool SaveEditorTextColorSettings(const EditorTextColorSettings& settings);
	[[nodiscard]] const EditorTextColorSettings& GetEditorTextColorSettings();
	EditorTextColorSettings& GetMutableEditorTextColorSettings();
	void ResetEditorTextColorSettings();
    [[nodiscard]] FilePath LoadPanelNinePatchPath();
 [[nodiscard]] FilePath LoadPanelNinePatchPath(PanelSkinTarget target);
	bool SavePanelNinePatchPath(FilePathView path);
  bool SavePanelNinePatchPath(PanelSkinTarget target, FilePathView path);
	[[nodiscard]] const FilePath& GetPanelNinePatchPath();
  [[nodiscard]] const FilePath& GetConfiguredPanelNinePatchPath(PanelSkinTarget target);
	[[nodiscard]] FilePath GetEffectivePanelNinePatchPath(PanelSkinTarget target);
	void SetPanelNinePatchPath(FilePathView path);
 void SetPanelNinePatchPath(PanelSkinTarget target, FilePathView path);
	void ResetPanelNinePatchPath();
    void ResetPanelNinePatchPath(PanelSkinTarget target);
   [[nodiscard]] UiLayoutSettings LoadUiLayoutSettings(int32 sceneWidth, int32 sceneHeight);
	bool SaveUiLayoutSettings(const UiLayoutSettings& settings);
   [[nodiscard]] ResourceStock LoadInitialPlayerResources();
	bool SaveInitialPlayerResources(const ResourceStock& resources);
   [[nodiscard]] UnitEditorSettings LoadUnitEditorSettings();
	bool SaveUnitEditorSettings(const UnitEditorSettings& settings);
}
