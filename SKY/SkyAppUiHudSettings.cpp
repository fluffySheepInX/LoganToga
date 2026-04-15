# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "SkyAppUiSettingsInternal.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	using namespace UiSettingsDetail;

		namespace
		{
			StringView ToFogCurveLabel(const FogOverlayDarknessCurve curve)
			{
				switch (curve)
				{
				case FogOverlayDarknessCurve::Soft:
					return U"Soft";

				case FogOverlayDarknessCurve::Strong:
					return U"Strong";

				case FogOverlayDarknessCurve::Linear:
				default:
					return U"Linear";
				}
			}
		}

	void DrawFogSettingsPanel(FogOfWarSettings& settings,
		const bool uiEditMode,
		bool& isExpanded,
		const SkyAppPanels& panels)
	{
		if (not isExpanded)
		{
			return;
		}

		const Rect& panelRect = panels.fogSettings;
		const Rect dragHandleRect = SkyAppUiLayout::FogSettingsPanelDragHandle(panelRect);
		const Rect resetButtonRect{ (panelRect.rightX() - 104), (panelRect.bottomY() - 40), 92, 28 };
		DrawSettingsPanelFrame(panelRect, U"Fog Of War");

		if (uiEditMode)
		{
			dragHandleRect.rounded(6).draw(ColorF{ 0.80, 0.86, 0.96, 0.32 }).drawFrame(1.0, 0.0, ColorF{ 0.56, 0.66, 0.82, 0.42 });
			SimpleGUI::GetFont()(U"Drag").drawAt(dragHandleRect.center(), ColorF{ 0.92, 0.96, 1.0, 0.88 });
		}

		DrawEditorCheckBox(settings.enabled, U"enabled", Vec2{ panelRect.x + 14.0, panelRect.y + 44.0 }, panelRect.w - 28.0);
		DrawEditorSlider(60, U"unit vision: {:.1f}"_fmt(settings.defaultUnitVisionRange), settings.defaultUnitVisionRange, 1.0, 24.0, Vec2{ panelRect.x + 12.0, panelRect.y + 82.0 }, 132.0, 180.0);
		DrawEditorSlider(61, U"base vision: {:.1f}"_fmt(settings.baseVisionRange), settings.baseVisionRange, 4.0, 36.0, Vec2{ panelRect.x + 12.0, panelRect.y + 118.0 }, 132.0, 180.0);
		DrawEditorSlider(62, U"resource vision: {:.1f}"_fmt(settings.ownedResourceVisionRange), settings.ownedResourceVisionRange, 2.0, 24.0, Vec2{ panelRect.x + 12.0, panelRect.y + 154.0 }, 132.0, 180.0);
		DrawEditorSlider(63, U"mill vision: {:.1f}"_fmt(settings.millVisionRange), settings.millVisionRange, 2.0, 24.0, Vec2{ panelRect.x + 12.0, panelRect.y + 190.0 }, 132.0, 180.0);
		DrawEditorSlider(64, U"darkness: {:.2f}"_fmt(settings.overlayDarkness), settings.overlayDarkness, 0.0, 1.0, Vec2{ panelRect.x + 12.0, panelRect.y + 226.0 }, 132.0, 180.0);

			SimpleGUI::GetFont()(U"curve:").draw((panelRect.x + 16), (panelRect.y + 269), UiInternal::EditorTextOnLightPrimaryColor());
			for (int32 i = 0; i < 3; ++i)
			{
				const FogOverlayDarknessCurve curve = (i == 0)
					? FogOverlayDarknessCurve::Linear
					: ((i == 1) ? FogOverlayDarknessCurve::Soft : FogOverlayDarknessCurve::Strong);
				const Rect buttonRect{ (panelRect.x + 78 + i * 82), (panelRect.y + 262), 76, 28 };
				if (DrawTerrainPageButton(buttonRect, ToFogCurveLabel(curve), (settings.overlayDarknessCurve == curve)))
				{
					settings.overlayDarknessCurve = curve;
				}
			}

			DrawEditorSlider(65, U"overlay height: {:.1f}"_fmt(settings.overlayHeight), settings.overlayHeight, 0.5, 8.0, Vec2{ panelRect.x + 12.0, panelRect.y + 304.0 }, 132.0, 180.0);

		settings.defaultUnitVisionRange = Max(0.5, settings.defaultUnitVisionRange);
		settings.baseVisionRange = Max(0.5, settings.baseVisionRange);
		settings.ownedResourceVisionRange = Max(0.0, settings.ownedResourceVisionRange);
		settings.millVisionRange = Max(0.0, settings.millVisionRange);
		settings.overlayDarkness = Clamp(settings.overlayDarkness, 0.0, 1.0);
		settings.overlayHeight = Max(0.1, settings.overlayHeight);

		if (DrawTextButton(resetButtonRect, U"Reset"))
		{
			settings = FogOfWarSettings{};
		}
	}
}

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	void DrawSettingsHud(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.showUI)
		{
			return;
		}

		DrawSkySettingsPanel(state.sky, state.skyTime, state.skySettingsExpanded, frame.panels);

		DrawCameraSettingsPanel(state.camera,
			state.cameraSettings,
			state.cameraSettingsExpanded,
			resources.GetUnitRenderModel(UnitRenderModel::Bird),
			resources.GetUnitRenderModel(UnitRenderModel::Ashigaru),
			state.cameraSaveMessage,
			frame.panels);
		DrawTerrainVisualSettingsPanel(state.terrainVisualSettings, state.uiEditMode, state.terrainVisualSettingsExpanded, frame.panels);
       DrawFogSettingsPanel(state.fogOfWarSettings, state.uiEditMode, state.fogSettingsExpanded, frame.panels);
	}
}
