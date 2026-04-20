# pragma once
# include <array>
# include <Siv3D.hpp>
# include "SkyAppLoop.hpp"
# include "SkyAppText.hpp"

namespace SkyAppFlow
{
    // --- Settings Panel Registry --------------------------------------
    //
    // Centralizes the description of HUD settings panels so that adding a
    // new panel is a single table-entry change instead of:
    //   1) adding an `*Expanded` flag to SkyAppState,
    //   2) adding a Rect to SkyAppPanels,
    //   3) adding a toggle button + tooltip in DrawHudModeToggles,
    //   4) adding a draw call in DrawSettingsHud,
    //   5) (for draggable panels) adding a TryBeginPanelInteraction call.
    //
    // Each descriptor binds:
    //   * The expanded-flag member pointer on SkyAppState
    //   * The toggle-button Rect member pointer on SkyAppPanels
    //   * Toggle button labels (expanded / collapsed) + tooltip TextId
    //   * The draw function (adapter that pulls needed members itself)
    //
    struct SettingsPanelDescriptor
    {
        StringView id;
        bool SkyAppFlow::HudUiState::* expandedFlag = nullptr;
        Rect SkyAppSupport::SkyAppPanels::* toggleRect = nullptr;
        StringView labelExpanded;
        StringView labelCollapsed;
        // Returns the localized (or literal) tooltip text shown when the
        // toggle button is hovered. Encapsulating this as a function lets
        // individual panels choose between TextId-based localization and
        // hard-coded strings.
        String (*tooltip)() = nullptr;
        void (*draw)(SkyAppFlow::SkyAppResources&, SkyAppFlow::SkyAppState&, const SkyAppFlow::SkyAppFrameState&) = nullptr;
    };

    inline constexpr size_t SettingsPanelCount = 4;

    [[nodiscard]] const std::array<SettingsPanelDescriptor, SettingsPanelCount>& GetSettingsPanelRegistry();

    // --- Bottom-Bar HUD Toggle Registry -------------------------------
    //
    // Every clickable toggle button on the bottom HUD bar (mode toggles,
    // settings-panel toggles, UI-edit toggle, resource-adjust toggle,
    // battle-command-scale toggle, debug enemy-plan toggle) is described
    // by a single descriptor. Adding a new toggle = a single entry in
    // `SkyAppHudToggleRegistry.cpp`; no edits to `DrawHudModeToggles`.
    //
    // Each descriptor binds:
    //   * The Rect member pointer on SkyAppPanels (button position)
    //   * label(state)   : returns the (state-dependent) button label
    //   * onClick(state) : action to perform on click
    //   * tooltip(state) : tooltip text shown while hovered
    //   * isVisible(state) (optional, nullptr = always shown)
    //
    struct HudToggleDescriptor
    {
        StringView id;
        Rect SkyAppSupport::SkyAppPanels::* rect = nullptr;
        String (*label)(const SkyAppFlow::SkyAppState&, const SkyAppFlow::SkyAppFrameState&) = nullptr;
        void (*onClick)(SkyAppFlow::SkyAppState&, const SkyAppFlow::SkyAppFrameState&) = nullptr;
        String (*tooltip)(const SkyAppFlow::SkyAppState&, const SkyAppFlow::SkyAppFrameState&) = nullptr;
        bool (*isVisible)(const SkyAppFlow::SkyAppState&, const SkyAppFlow::SkyAppFrameState&) = nullptr;
    };

    [[nodiscard]] const Array<HudToggleDescriptor>& GetHudToggleRegistry();
}
