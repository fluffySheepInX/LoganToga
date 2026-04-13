# include "SkyAppUiSettingsInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	using namespace UiSettingsDetail;

	void DrawCameraSettingsPanel(AppCamera3D& camera,
		CameraSettings& cameraSettings,
		bool& isExpanded,
		UnitModel& birdModel,
		UnitModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels)
	{
		Vec3 editedEye = cameraSettings.eye;
		Vec3 editedFocus = cameraSettings.focus;
		bool cameraChanged = false;
		const Rect& panelRect = panels.cameraSettings;

		if (not isExpanded)
		{
			return;
		}

		constexpr int32 contentOffsetY = 20;
		const Rect saveButtonRect = SkyAppUiLayout::CameraPanelButtonRect(panelRect, 20, (310 + contentOffsetY));
		const Rect resetButtonRect = SkyAppUiLayout::CameraPanelButtonRect(panelRect, 190, (310 + contentOffsetY));
		DrawCameraSettingsPanelFrame(panelRect);
		SimpleGUI::GetFont()(U"Camera eye").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (32 + contentOffsetY)), UiInternal::EditorTextOnLightPrimaryColor());
		cameraChanged = DrawEditorSlider(20, U"eyeX: {:.2f}"_fmt(editedEye.x), editedEye.x, GetSliderMin(cameraSettings.eye.x, -50.0), GetSliderMax(cameraSettings.eye.x, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (40 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = DrawEditorSlider(21, U"eyeY: {:.2f}"_fmt(editedEye.y), editedEye.y, GetSliderMin(cameraSettings.eye.y, -10.0), GetSliderMax(cameraSettings.eye.y, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (80 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = DrawEditorSlider(22, U"eyeZ: {:.2f}"_fmt(editedEye.z), editedEye.z, GetSliderMin(cameraSettings.eye.z, -50.0), GetSliderMax(cameraSettings.eye.z, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (120 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		SimpleGUI::GetFont()(U"Camera focus").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (170 + contentOffsetY)), UiInternal::EditorTextOnLightPrimaryColor());
		cameraChanged = DrawEditorSlider(23, U"focusX: {:.2f}"_fmt(editedFocus.x), editedFocus.x, GetSliderMin(cameraSettings.focus.x, -50.0), GetSliderMax(cameraSettings.focus.x, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (200 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = DrawEditorSlider(24, U"focusY: {:.2f}"_fmt(editedFocus.y), editedFocus.y, GetSliderMin(cameraSettings.focus.y, -10.0), GetSliderMax(cameraSettings.focus.y, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (240 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = DrawEditorSlider(25, U"focusZ: {:.2f}"_fmt(editedFocus.z), editedFocus.z, GetSliderMin(cameraSettings.focus.z, -50.0), GetSliderMax(cameraSettings.focus.z, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (280 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;

		if (cameraChanged)
		{
			cameraSettings.eye = editedEye;
			cameraSettings.focus = editedFocus;
			EnsureValidCameraSettings(cameraSettings);
		}

		if (DrawTextButton(saveButtonRect, U"Save TOML"))
		{
			cameraSaveMessage.show(SaveCameraSettings(cameraSettings)
				? U"Saved: {}"_fmt(CameraSettingsPath)
				: U"Save failed");
		}

		if (DrawTextButton(resetButtonRect, U"視点初期化"))
		{
			cameraSettings.eye = DefaultCameraEye;
			cameraSettings.focus = DefaultCameraFocus;
			EnsureValidCameraSettings(cameraSettings);
			ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"DrawCameraSettingsPanel: reset button");
			camera.setView(cameraSettings.eye, cameraSettings.focus);
			cameraSaveMessage.show(U"Camera reset");
		}

		if (cameraSaveMessage.isVisible())
		{
			SimpleGUI::GetFont()(cameraSaveMessage.text).draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (352 + contentOffsetY)), UiInternal::EditorTextOnLightSecondaryColor());
		}

		if (not birdModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"bird.glb load failed").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (376 + contentOffsetY)), UiInternal::EditorTextErrorColor());
		}

		if (not ashigaruModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"ashigaru_v2.1.glb load failed").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (400 + contentOffsetY)), UiInternal::EditorTextErrorColor());
		}

		DrawAnimationClipSelector(birdModel, U"Bird Clips", (panelRect.x + 20), (panelRect.y + contentOffsetY + 400), SkyAppUiLayout::CameraPanelClipSelectorWidth());
		DrawAnimationClipSelector(ashigaruModel, U"Ashigaru Clips", (panelRect.x + 190), (panelRect.y + contentOffsetY + 400), SkyAppUiLayout::CameraPanelClipSelectorWidth());
	}
}
