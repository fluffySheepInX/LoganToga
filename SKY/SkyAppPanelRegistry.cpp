# include "SkyAppPanelRegistry.hpp"
# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"

namespace SkyAppFlow
{
    using namespace MainSupport;
    using namespace SkyAppSupport;

    namespace
    {
        // --- Per-panel draw adapters -----------------------------------
        // Each adapter has the unified signature
        //   (SkyAppResources&, SkyAppState&, const SkyAppFrameState&)
        // and forwards to the existing Draw*Panel function with the
        // arguments it needs. This keeps DrawSettingsHud agnostic of the
        // individual panel signatures.

        void DrawSkyPanelAdapter(SkyAppResources& /*resources*/, SkyAppState& state, const SkyAppFrameState& frame)
        {
            DrawSkySettingsPanel(state.env.sky, state.env.skyTime, state.hud.skySettingsExpanded, frame.panels);
        }

        void DrawCameraPanelAdapter(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
        {
            DrawCameraSettingsPanel(state.env.camera,
                state.env.cameraSettings,
                state.hud.cameraSettingsExpanded,
                resources.GetUnitRenderModel(UnitRenderModel::Bird),
                resources.GetUnitRenderModel(UnitRenderModel::Ashigaru),
                state.messages[SkyAppSupport::MessageChannel::CameraSave],
                frame.panels);
        }

        void DrawTerrainVisualPanelAdapter(SkyAppResources& /*resources*/, SkyAppState& state, const SkyAppFrameState& frame)
        {
            DrawTerrainVisualSettingsPanel(state.world.terrainVisualSettings, state.hud.uiEditMode, state.hud.terrainVisualSettingsExpanded, frame.panels);
        }

        void DrawFogPanelAdapter(SkyAppResources& /*resources*/, SkyAppState& state, const SkyAppFrameState& frame)
        {
            DrawFogSettingsPanel(state.env.fogOfWarSettings, state.hud.uiEditMode, state.hud.fogSettingsExpanded, frame.panels);
        }

        // --- The registry ---------------------------------------------
        // Adding a new HUD settings panel = add a single entry here.
        inline const std::array<SettingsPanelDescriptor, 4> Registry{ {
            { U"sky",
                &HudUiState::skySettingsExpanded,
                &SkyAppPanels::skySettingsToggle,
                U"Ÿ", U"ž",
                +[]() -> String { return SkyAppText::Tr(U"HudTooltipSkySettingsPanel"); },
                &DrawSkyPanelAdapter },
            { U"camera",
                &HudUiState::cameraSettingsExpanded,
                &SkyAppPanels::cameraSettingsToggle,
                U"?", U"",
                +[]() -> String { return SkyAppText::Tr(U"HudTooltipCameraSettingsPanel"); },
                &DrawCameraPanelAdapter },
            { U"terrainVisual",
                &HudUiState::terrainVisualSettingsExpanded,
                &SkyAppPanels::terrainVisualToggle,
                U"’n", U"“y",
                +[]() -> String { return SkyAppText::Tr(U"HudTooltipTerrainNoiseSettings"); },
                &DrawTerrainVisualPanelAdapter },
            { U"fog",
                &HudUiState::fogSettingsExpanded,
                &SkyAppPanels::fogSettingsToggle,
                U"Fog", U"fog",
                +[]() -> String { return String{ U"Fog editor panel" }; },
                &DrawFogPanelAdapter },
        } };
    }

    const std::array<SettingsPanelDescriptor, SettingsPanelCount>& GetSettingsPanelRegistry()
    {
        return Registry;
    }
}
