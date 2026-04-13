# include "SkyAppUiSettingsInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	using namespace UiSettingsDetail;

	void DrawSkySettingsPanel(Sky& sky, double& skyTime, bool& isExpanded, const SkyAppPanels& panels)
	{
		if (not isExpanded)
		{
			return;
		}

		const Rect& panelRect = panels.skySettings;
		const Rect timePanelRect = SkyAppUiLayout::SkySettingsTimePanel(panelRect);
		DrawSkySettingsPanelFrame(panelRect, U"Sky Settings");
		DrawSkySettingsPanelFrame(timePanelRect, U"Sky Time");
		constexpr int32 contentOffsetY = 36;
		constexpr double SkyTimeMin = -2.0;
		constexpr double SkyTimeMax = 4.0;
		constexpr double SkyTimeSmallStep = 0.01;
		Rect{ panelRect.x, panelRect.y + contentOffsetY, panelRect.w, 76 }.draw();
		SimpleGUI::GetFont()(U"zenith:").draw((panelRect.x + 8), (panelRect.y + contentOffsetY + 4), UiInternal::EditorTextOnLightPrimaryColor());
		Rect{ (panelRect.x + 80), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.zenithColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"horizon:").draw((panelRect.x + 128), (panelRect.y + contentOffsetY + 4), UiInternal::EditorTextOnLightPrimaryColor());
		Rect{ (panelRect.x + 210), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.horizonColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"cloud:").draw((panelRect.x + 256), (panelRect.y + contentOffsetY + 4), UiInternal::EditorTextOnLightPrimaryColor());
		Rect{ (panelRect.x + 320), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(sky.cloudColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sun:").draw((panelRect.x + 366), (panelRect.y + contentOffsetY + 4), UiInternal::EditorTextOnLightPrimaryColor());
		Rect{ (panelRect.x + 410), (panelRect.y + contentOffsetY + 6), 28, 28 }.draw(Graphics3D::GetSunColor().gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sunDir: {:.2f}   cloudTime: {:.1f}"_fmt(Graphics3D::GetSunDirection(), sky.cloudTime)).draw((panelRect.x + 8), (panelRect.y + contentOffsetY + 40), UiInternal::EditorTextOnLightSecondaryColor());

		DrawEditorSlider(0, U"cloudiness: {:.3f}"_fmt(sky.cloudiness), sky.cloudiness, 0.0, 1.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 80.0 }, 180, 300);
		DrawEditorSlider(1, U"cloudScale: {:.2f}"_fmt(sky.cloudScale), sky.cloudScale, 0.0, 2.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 120.0 }, 180, 300);
		DrawEditorSlider(2, U"cloudHeight: {:.0f}"_fmt(sky.cloudPlaneHeight), sky.cloudPlaneHeight, 20.0, 6000.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 160.0 }, 180, 300);
		DrawEditorSlider(3, U"orientation: {:.0f}"_fmt(Math::ToDegrees(sky.cloudOrientation)), sky.cloudOrientation, 0.0, Math::TwoPi, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 200.0 }, 180, 300);
		DrawEditorSlider(4, U"fogHeightSky: {:.2f}"_fmt(sky.fogHeightSky), sky.fogHeightSky, GetSliderMin(sky.fogHeightSky, 0.0), GetSliderMax(sky.fogHeightSky, 1.0), Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 240.0 }, 180, 300);
		DrawEditorSlider(5, U"star: {:.2f}"_fmt(sky.starBrightness), sky.starBrightness, GetSliderMin(sky.starBrightness, 0.0), GetSliderMax(sky.starBrightness, 1.0), Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 280.0 }, 180, 300);
		DrawEditorSlider(6, U"starF: {:.2f}"_fmt(sky.starBrightnessFactor), sky.starBrightnessFactor, GetSliderMin(sky.starBrightnessFactor, 0.0), GetSliderMax(sky.starBrightnessFactor, 2.0), Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 320.0 }, 180, 300);
		DrawEditorSlider(7, U"starSat: {:.2f}"_fmt(sky.starSaturation), sky.starSaturation, 0.0, 1.0, Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 360.0 }, 180, 300);
		DrawEditorCheckBox(sky.sunEnabled, U"sun", Vec2{ panelRect.x + 0.0, panelRect.y + contentOffsetY + 400.0 }, 120, false);
		DrawEditorCheckBox(sky.cloudsEnabled, U"clouds", Vec2{ panelRect.x + 130.0, panelRect.y + contentOffsetY + 400.0 }, 120);
		DrawEditorCheckBox(sky.cloudsLightingEnabled, U"cloudsLighting", Vec2{ panelRect.x + 260.0, panelRect.y + contentOffsetY + 400.0 }, 220);

		DrawEditorSlider(8,
			U"time: {:.2f}"_fmt(skyTime),
			skyTime,
			SkyTimeMin,
			SkyTimeMax,
			SkyAppUiLayout::SkySettingsTimeSliderPosition(timePanelRect),
			SkyAppUiLayout::SkySettingsTimeSliderLabelWidth(),
			SkyAppUiLayout::SkySettingsTimeSliderTrackWidth(timePanelRect));

		if (DrawTextButton(SkyAppUiLayout::SkySettingsTimeStepButton(timePanelRect, 0), U"-S"))
		{
			skyTime = Max(SkyTimeMin, (skyTime - SkyTimeSmallStep));
		}

		if (DrawTextButton(SkyAppUiLayout::SkySettingsTimeStepButton(timePanelRect, 1), U"+S"))
		{
			skyTime = Min(SkyTimeMax, (skyTime + SkyTimeSmallStep));
		}
	}
}
