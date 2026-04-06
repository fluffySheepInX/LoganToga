# include "SkyAppSupport.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		constexpr double MessageDisplaySeconds = 2.0;
	}

   void TimedMessage::show(const StringView message, const double durationSeconds)
	{
		text = message;
        until = (Scene::Time() + Max(0.0, durationSeconds));
	}

	bool TimedMessage::isVisible() const
	{
		return (Scene::Time() < until);
	}

	bool SkyAppPanels::isHoveringUi(const bool showUI, const bool isEditorMode, const bool showBlacksmithMenu, const bool showSapperMenu, const bool modelHeightEditMode) const
	{
		return (showUI && (skySettings.mouseOver() || cameraSettings.mouseOver() || miniMap.mouseOver()))
			|| (isEditorMode && mapEditor.mouseOver())
			|| (showBlacksmithMenu && blacksmithMenu.mouseOver())
			|| (showSapperMenu && sapperMenu.mouseOver())
			|| (modelHeightEditMode && modelHeight.mouseOver())
			|| uiToggle.mouseOver()
			|| mapModeToggle.mouseOver()
			|| modelHeightModeToggle.mouseOver()
            || reloadMapButton.mouseOver()
			|| restartButton.mouseOver()
			|| timeSlider.mouseOver();
	}

	void UpdateCameraWheelZoom(DebugCamera3D& camera, CameraSettings& cameraSettings, const Vec3& playerBasePosition)
	{
		const double wheel = Mouse::Wheel();

		if (wheel == 0.0)
		{
			return;
		}

		if (const auto zoomFocus = GetWheelZoomFocusPosition(camera, playerBasePosition))
		{
			const Vec3 focusPosition = *zoomFocus;
			const Vec3 eyeOffset = (cameraSettings.eye - focusPosition);
			const double currentDistance = eyeOffset.length();

			if (0.001 < currentDistance)
			{
				const double targetDistance = Clamp((currentDistance * Math::Pow(CameraZoomFactorPerWheelStep, wheel)), CameraZoomMinDistance, CameraZoomMaxDistance);
				cameraSettings.eye = (focusPosition + (eyeOffset * (targetDistance / currentDistance)));
				cameraSettings.focus = focusPosition;
				camera.setView(cameraSettings.eye, cameraSettings.focus);
			}
		}
	}

	void UpdateSkyFromTime(Sky& sky, const double skyTime)
	{
		const double time0_2 = Math::Fraction(skyTime * 0.5) * 2.0;
		const double halfDay0_1 = Math::Fraction(skyTime);
		const double distanceFromNoon0_1 = Math::Saturate(1.0 - (Abs(0.5 - halfDay0_1) * 2.0));
		const bool night = (1.0 < time0_2);
		const double tf = EaseOutCubic(distanceFromNoon0_1);
		const double tc = EaseInOutCubic(distanceFromNoon0_1);
		const double starCenteredTime = Math::Fmod(time0_2 + 1.5, 2.0);

		{
			const Quaternion q = (Quaternion::RotateY(halfDay0_1 * 180_deg) * Quaternion::RotateX(50_deg));
			const Vec3 sunDirection = q * Vec3::Right();
			const ColorF sunColor{ 0.1 + Math::Pow(tf, 1.0 / 2.0) * (night ? 0.1 : 0.9) };

			Graphics3D::SetSunDirection(sunDirection);
			Graphics3D::SetSunColor(sunColor);
			Graphics3D::SetGlobalAmbientColor(ColorF{ sky.zenithColor });
		}

		if (night)
		{
			sky.zenithColor = ColorF{ 0.3, 0.05, 0.1 }.lerp(ColorF{ 0.1, 0.1, 0.15 }, tf);
			sky.horizonColor = ColorF{ 0.1, 0.1, 0.15 }.lerp(ColorF{ 0.1, 0.1, 0.2 }, tf);
		}
		else
		{
			sky.zenithColor = ColorF{ 0.4, 0.05, 0.1 }.lerp(ColorF{ 0.15, 0.24, 0.56 }, tf);
			sky.horizonColor = ColorF{ 0.2, 0.05, 0.15 }.lerp(ColorF{ 0.3, 0.4, 0.5 }, tf);
		}

		sky.starBrightness = Math::Saturate(1.0 - Pow(Abs(1.0 - starCenteredTime) * 1.8, 4));
		sky.fogHeightSky = (1.0 - tf);
		sky.cloudColor = ColorF{ 0.02 + (night ? 0.0 : (0.98 * tc)) };
		sky.sunEnabled = (not night);
		sky.cloudTime = skyTime * sky.cloudScale * 40.0;
		sky.starTime = skyTime;
	}
}
