# include "SkyAppSupport.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		constexpr double MessageDisplaySeconds = 2.0;

		[[nodiscard]] bool IsSafeCameraPair(const Vec3& eye, const Vec3& focus)
		{
			if ((not std::isfinite(eye.x)) || (not std::isfinite(eye.y)) || (not std::isfinite(eye.z))
				|| (not std::isfinite(focus.x)) || (not std::isfinite(focus.y)) || (not std::isfinite(focus.z)))
			{
				return false;
			}

			const double distanceSq = (focus - eye).lengthSq();
            if ((not std::isfinite(distanceSq)) || (distanceSq < Square(MinimumCameraEyeFocusDistance * 0.5)))
			{
				return false;
			}

			const Vec3 direction = ((focus - eye) / std::sqrt(distanceSq));
			const double upAlignment = Abs(direction.dot(Vec3::Up()));
			return std::isfinite(distanceSq)
             && (upAlignment < 0.995);
		}
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

      SkyAppPanels::SkyAppPanels(const UiLayoutSettings& uiLayoutSettings, const bool skySettingsExpanded, const bool cameraSettingsExpanded, const bool miniMapExpanded, const bool resourceAdjustExpanded)
		: miniMap{ SkyAppUiLayout::MiniMap(Scene::Width(), Scene::Height(), uiLayoutSettings.miniMapPosition, miniMapExpanded) }
       , skySettings{ SkyAppUiLayout::SkySettings(Scene::Width(), Scene::Height(), skySettingsExpanded) }
		, cameraSettings{ SkyAppUiLayout::CameraSettings(Scene::Width(), Scene::Height(), skySettingsExpanded, cameraSettingsExpanded) }
		, mapEditor{ SkyAppUiLayout::MapEditor(Scene::Width(), Scene::Height()) }
		, blacksmithMenu{ SkyAppUiLayout::BlacksmithMenu(Scene::Width(), Scene::Height()) }
		, sapperMenu{ SkyAppUiLayout::SapperMenu(Scene::Width(), Scene::Height()) }
		, millStatusEditor{ SkyAppUiLayout::MillStatusEditor(Scene::Width(), Scene::Height()) }
       , modelHeight{ SkyAppUiLayout::ModelHeight(Scene::Width(), Scene::Height(), skySettingsExpanded, cameraSettingsExpanded) }
       , unitEditor{ SkyAppUiLayout::UnitEditor(Scene::Width(), Scene::Height(), uiLayoutSettings.unitEditorPosition) }
	   , unitEditorList{ SkyAppUiLayout::UnitEditorList(Scene::Width(), Scene::Height(), uiLayoutSettings.unitEditorListPosition) }
        , resourcePanel{ SkyAppUiLayout::ResourcePanel(Scene::Width(), Scene::Height(), uiLayoutSettings.resourcePanelPosition, resourceAdjustExpanded) }
     , escMenu{ SkyAppUiLayout::EscMenu(Scene::Width(), Scene::Height()) }
		, uiToggle{ SkyAppUiLayout::UiToggle(Scene::Width(), Scene::Height()) }
		, mapModeToggle{ SkyAppUiLayout::MapModeToggle(Scene::Width(), Scene::Height()) }
		, modelHeightModeToggle{ SkyAppUiLayout::ModelHeightModeToggle(Scene::Width(), Scene::Height()) }
     , unitEditorModeToggle{ SkyAppUiLayout::UnitEditorModeToggle(Scene::Width(), Scene::Height()) }
     , skySettingsToggle{ SkyAppUiLayout::SkySettingsToggle(Scene::Width(), Scene::Height()) }
		, cameraSettingsToggle{ SkyAppUiLayout::CameraSettingsToggle(Scene::Width(), Scene::Height()) }
     , uiEditModeToggle{ SkyAppUiLayout::UiEditModeToggle(Scene::Width(), Scene::Height()) }
     , resourceAdjustToggle{ SkyAppUiLayout::ResourceAdjustToggle(Scene::Width(), Scene::Height()) }
		, timeSlider{ SkyAppUiLayout::TimeSlider(Scene::Width(), Scene::Height()) }
	{
	}

  bool SkyAppPanels::isHoveringUi(const bool showUI, const bool showSkySettings, const bool showCameraSettings, const bool isEditorMode, const bool showBlacksmithMenu, const bool showSapperMenu, const bool showMillStatusEditor, const bool modelHeightEditMode, const bool showUnitEditor) const
	{
       return (showUI && ((showSkySettings && skySettings.mouseOver()) || (showCameraSettings && cameraSettings.mouseOver())))
			|| miniMap.mouseOver()
          || resourcePanel.mouseOver()
          || SkyAppUiLayout::ResourcePanelCameraHomeButton(resourcePanel).mouseOver()
			|| (isEditorMode && mapEditor.mouseOver())
			|| (showBlacksmithMenu && blacksmithMenu.mouseOver())
			|| (showSapperMenu && sapperMenu.mouseOver())
         || (showMillStatusEditor && millStatusEditor.mouseOver())
			|| (modelHeightEditMode && modelHeight.mouseOver())
          || (showUnitEditor && (unitEditor.mouseOver() || unitEditorList.mouseOver()))
			|| uiToggle.mouseOver()
            || (showUI && mapModeToggle.mouseOver())
            || (showUI && modelHeightModeToggle.mouseOver())
            || (showUI && unitEditorModeToggle.mouseOver())
            || (showUI && skySettingsToggle.mouseOver())
			|| (showUI && cameraSettingsToggle.mouseOver())
            || (showUI && uiEditModeToggle.mouseOver())
            || (showUI && resourceAdjustToggle.mouseOver())
          || (showUI && timeSlider.mouseOver());
	}

   void UpdateCameraWheelZoom(AppCamera3D& camera, CameraSettings& cameraSettings, const Vec3& playerBasePosition)
	{
      EnsureValidCameraSettings(cameraSettings);
		if (not IsSafeCameraPair(cameraSettings.eye, cameraSettings.focus))
		{
          ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraWheelZoom: pre-setView cameraSettings");
		}
       ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraWheelZoom: camera.setView (initial)");
		camera.setView(cameraSettings.eye, cameraSettings.focus);

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
				EnsureValidCameraSettings(cameraSettings);
				if (not IsSafeCameraPair(cameraSettings.eye, cameraSettings.focus))
				{
                  ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraWheelZoom: post-zoom cameraSettings");
				}
               ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraWheelZoom: camera.setView (post-zoom)");
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
