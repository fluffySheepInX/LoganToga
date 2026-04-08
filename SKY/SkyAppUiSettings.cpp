# include "SkyAppUiInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		[[nodiscard]] double GetSliderMin(const double value, const double defaultMin)
		{
			return Min(defaultMin, (value - 1.0));
		}

		[[nodiscard]] double GetSliderMax(const double value, const double defaultMax)
		{
			return Max(defaultMax, (value + 1.0));
		}
	}

	void DrawSkySettingsPanel(Sky& sky, bool& isExpanded, const SkyAppPanels& panels)
	{
		const Rect& panelRect = panels.skySettings;
		if (UiInternal::DrawAccordionHeader(panelRect, U"Sky Settings", isExpanded, ColorF{ 1.0, 0.92 }))
		{
			isExpanded = not isExpanded;
		}

		if (not isExpanded)
		{
			return;
		}

		UiInternal::DrawPanelFrame(panelRect, U"", ColorF{ 1.0, 0.92 });
		UiInternal::DrawAccordionHeader(panelRect, U"Sky Settings", isExpanded, ColorF{ 1.0, 0.92 });
		constexpr int32 contentOffsetY = SkyAppUiLayout::AccordionHeaderHeight;
		Rect{ panelRect.x, panelRect.y + contentOffsetY, panelRect.w, 76 }.draw();
		SimpleGUI::GetFont()(U"zenith:").draw((panelRect.x + 8), (panelRect.y + contentOffsetY + 4), ColorF{ 0.11 });
		Rect{ (panelRect.x + 80), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.zenithColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"horizon:").draw((panelRect.x + 128), (panelRect.y + contentOffsetY + 4), ColorF{ 0.11 });
		Rect{ (panelRect.x + 210), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.horizonColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"cloud:").draw((panelRect.x + 256), (panelRect.y + contentOffsetY + 4), ColorF{ 0.11 });
		Rect{ (panelRect.x + 320), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.cloudColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sun:").draw((panelRect.x + 366), (panelRect.y + contentOffsetY + 4), ColorF{ 0.11 });
		Rect{ (panelRect.x + 410), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(Graphics3D::GetSunColor().gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sunDir: {:.2f}   cloudTime: {:.1f}"_fmt(Graphics3D::GetSunDirection(), sky.cloudTime)).draw((panelRect.x + 8), (panelRect.y + contentOffsetY + 40), ColorF{ 0.11 });

		SimpleGUI::Slider(U"cloudiness: {:.3f}"_fmt(sky.cloudiness), sky.cloudiness, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 80.0 }, 180, 300);
		SimpleGUI::Slider(U"cloudScale: {:.2f}"_fmt(sky.cloudScale), sky.cloudScale, 0.0, 2.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 120.0 }, 180, 300);
		SimpleGUI::Slider(U"cloudHeight: {:.0f}"_fmt(sky.cloudPlaneHeight), sky.cloudPlaneHeight, 20.0, 6000.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 160.0 }, 180, 300);
		SimpleGUI::Slider(U"orientation: {:.0f}"_fmt(Math::ToDegrees(sky.cloudOrientation)), sky.cloudOrientation, 0.0, Math::TwoPi, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 200.0 }, 180, 300);
		SimpleGUI::Slider(U"fogHeightSky: {:.2f}"_fmt(sky.fogHeightSky), sky.fogHeightSky, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 240.0 }, 180, 300, false);
		SimpleGUI::Slider(U"star: {:.2f}"_fmt(sky.starBrightness), sky.starBrightness, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 280.0 }, 180, 300, false);
		SimpleGUI::Slider(U"starF: {:.2f}"_fmt(sky.starBrightnessFactor), sky.starBrightnessFactor, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 320.0 }, 180, 300);
		SimpleGUI::Slider(U"starSat: {:.2f}"_fmt(sky.starSaturation), sky.starSaturation, 0.0, 1.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 360.0 }, 180, 300);
		SimpleGUI::CheckBox(sky.sunEnabled, U"sun", Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 400.0 }, 120, false);
		SimpleGUI::CheckBox(sky.cloudsEnabled, U"clouds", Vec2{ panelRect.x + 130.0, panelRect.y + contentOffsetY + 400.0 }, 120);
		SimpleGUI::CheckBox(sky.cloudsLightingEnabled, U"cloudsLighting", Vec2{ panelRect.x + 260.0, panelRect.y + contentOffsetY + 400.0 }, 220);
	}

	void DrawCameraSettingsPanel(AppCamera3D& camera,
		CameraSettings& cameraSettings,
		bool& isExpanded,
		BirdModel& birdModel,
		BirdModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels)
	{
		Vec3 editedEye = cameraSettings.eye;
		Vec3 editedFocus = cameraSettings.focus;
		bool cameraChanged = false;
		const Rect& panelRect = panels.cameraSettings;
		constexpr int32 contentOffsetY = SkyAppUiLayout::AccordionHeaderHeight;

		if (UiInternal::DrawAccordionHeader(panelRect, U"Camera Settings", isExpanded, ColorF{ 1.0, 0.92 }))
		{
			isExpanded = not isExpanded;
		}

		if (not isExpanded)
		{
			return;
		}

		UiInternal::DrawPanelFrame(panelRect, U"", ColorF{ 1.0, 0.92 });
		UiInternal::DrawAccordionHeader(panelRect, U"Camera Settings", isExpanded, ColorF{ 1.0, 0.92 });
		SimpleGUI::GetFont()(U"Camera eye").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (32 + contentOffsetY)), ColorF{ 0.11 });
		cameraChanged = SimpleGUI::Slider(U"eyeX: {:.2f}"_fmt(editedEye.x), editedEye.x, GetSliderMin(cameraSettings.eye.x, -50.0), GetSliderMax(cameraSettings.eye.x, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (40 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"eyeY: {:.2f}"_fmt(editedEye.y), editedEye.y, GetSliderMin(cameraSettings.eye.y, -10.0), GetSliderMax(cameraSettings.eye.y, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (80 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"eyeZ: {:.2f}"_fmt(editedEye.z), editedEye.z, GetSliderMin(cameraSettings.eye.z, -50.0), GetSliderMax(cameraSettings.eye.z, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (120 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		SimpleGUI::GetFont()(U"Camera focus").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (170 + contentOffsetY)), ColorF{ 0.11 });
		cameraChanged = SimpleGUI::Slider(U"focusX: {:.2f}"_fmt(editedFocus.x), editedFocus.x, GetSliderMin(cameraSettings.focus.x, -50.0), GetSliderMax(cameraSettings.focus.x, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (200 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"focusY: {:.2f}"_fmt(editedFocus.y), editedFocus.y, GetSliderMin(cameraSettings.focus.y, -10.0), GetSliderMax(cameraSettings.focus.y, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (240 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;
		cameraChanged = SimpleGUI::Slider(U"focusZ: {:.2f}"_fmt(editedFocus.z), editedFocus.z, GetSliderMin(cameraSettings.focus.z, -50.0), GetSliderMax(cameraSettings.focus.z, 50.0), SkyAppUiLayout::CameraPanelSliderPosition(panelRect, (280 + contentOffsetY)), SkyAppUiLayout::CameraPanelSliderLabelWidth(), SkyAppUiLayout::CameraPanelSliderWidth()) || cameraChanged;

		if (cameraChanged)
		{
			cameraSettings.eye = editedEye;
			cameraSettings.focus = editedFocus;
			EnsureValidCameraSettings(cameraSettings);
		}

		if (DrawTextButton(SkyAppUiLayout::CameraPanelButtonRect(panelRect, 20, (310 + contentOffsetY)), U"Save TOML"))
		{
			cameraSaveMessage.show(SaveCameraSettings(cameraSettings)
				? U"Saved: {}"_fmt(CameraSettingsPath)
				: U"Save failed");
		}

		if (DrawTextButton(SkyAppUiLayout::CameraPanelButtonRect(panelRect, 190, (310 + contentOffsetY)), U"視点初期化"))
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
			SimpleGUI::GetFont()(cameraSaveMessage.text).draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (352 + contentOffsetY)), ColorF{ 0.11 });
		}

		if (not birdModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"bird.glb load failed").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (376 + contentOffsetY)), ColorF{ 0.75, 0.2, 0.2 });
		}

		if (not ashigaruModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"ashigaru_v2.1.glb load failed").draw(SkyAppUiLayout::CameraPanelTextPosition(panelRect, 20, (400 + contentOffsetY)), ColorF{ 0.75, 0.2, 0.2 });
		}

		DrawAnimationClipSelector(birdModel, U"Bird Clips", (panelRect.x + 20), (panelRect.y + contentOffsetY + 400), SkyAppUiLayout::CameraPanelClipSelectorWidth());
		DrawAnimationClipSelector(ashigaruModel, U"Ashigaru Clips", (panelRect.x + 190), (panelRect.y + contentOffsetY + 400), SkyAppUiLayout::CameraPanelClipSelectorWidth());
	}
}
