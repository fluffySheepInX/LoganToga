# include "SkyAppPanelRegistry.hpp"
# include "SkyAppLoopInternal.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUi.hpp"
# include "MainSettings.hpp"

namespace SkyAppFlow
{
    using namespace MainSupport;
    using namespace SkyAppSupport;
    using SkyAppText::Tr;

    namespace
    {
        // --- Helpers shared by descriptors ----------------------------

        [[nodiscard]] EnemyBattlePlanOverride GetNextEnemyBattlePlanOverride(const EnemyBattlePlanOverride mode)
        {
            switch (mode)
            {
            case EnemyBattlePlanOverride::Auto:
                return EnemyBattlePlanOverride::ForceSecureResources;
            case EnemyBattlePlanOverride::ForceSecureResources:
                return EnemyBattlePlanOverride::ForceAssaultBase;
            case EnemyBattlePlanOverride::ForceAssaultBase:
            default:
                return EnemyBattlePlanOverride::Auto;
            }
        }

        constexpr bool DebugEnemyBattlePlanVisible =
            #if _DEBUG
                true;
            #else
                false;
            #endif

        // --- Per-toggle handlers --------------------------------------
        // Each handler mirrors the logic that previously lived inline in
        // DrawHudModeToggles. The label/onClick/tooltip triple stays
        // adjacent so adding a new toggle is local.

        // mapMode ------------------------------------------------------
        String LabelMapMode(const SkyAppState&, const SkyAppFrameState& frame)
        {
            return frame.isEditorMode ? String{ U"M+" } : String{ U"m+" };
        }
        void ClickMapMode(SkyAppState& state, const SkyAppFrameState& frame)
        {
            state.env.appMode = frame.isEditorMode ? AppMode::Play : AppMode::EditMap;
            state.hud.showBlacksmithMenu = false;
            state.battle.selectedSapperIndices.clear();
            state.battle.selectedMillIndex.reset();
            state.battle.selectionDragStart.reset();
            state.editor.mapEditor.hoveredGroundPosition.reset();
            state.editor.unitEditorMode = false;
            state.editor.modelHeightEditMode = false;
        }
        String TooltipMapMode(const SkyAppState&, const SkyAppFrameState& frame)
        {
            return frame.isEditorMode ? Tr(U"HudTooltipReturnToPlay") : Tr(U"HudTooltipMapEditMode");
        }

        // modelHeight --------------------------------------------------
        String LabelModelHeight(const SkyAppState& state, const SkyAppFrameState&)
        {
            return state.editor.modelHeightEditMode ? String{ U"H+" } : String{ U"h+" };
        }
        void ClickModelHeight(SkyAppState& state, const SkyAppFrameState&)
        {
            state.editor.modelHeightEditMode = (not state.editor.modelHeightEditMode);
            if (state.editor.modelHeightEditMode)
            {
                state.editor.unitEditorMode = false;
            }
        }
        String TooltipModelHeight(const SkyAppState&, const SkyAppFrameState&)
        {
            return Tr(U"HudTooltipModelHeightScale");
        }

        // unitEditor ---------------------------------------------------
        String LabelUnitEditor(const SkyAppState& state, const SkyAppFrameState&)
        {
            return state.editor.unitEditorMode ? String{ U"Unit" } : String{ U"unit" };
        }
        void ClickUnitEditor(SkyAppState& state, const SkyAppFrameState&)
        {
            state.editor.unitEditorMode = (not state.editor.unitEditorMode);
            if (state.editor.unitEditorMode)
            {
                state.editor.modelHeightEditMode = false;
            }
        }
        String TooltipUnitEditor(const SkyAppState&, const SkyAppFrameState&)
        {
            return Tr(U"HudTooltipUnitParameterEditor");
        }

        // uiEditMode ---------------------------------------------------
        String LabelUiEditMode(const SkyAppState& state, const SkyAppFrameState&)
        {
            return state.hud.uiEditMode ? String{ U"UI+" } : String{ U"ui+" };
        }
        void ClickUiEditMode(SkyAppState& state, const SkyAppFrameState&)
        {
            state.hud.uiEditMode = (not state.hud.uiEditMode);
            state.hud.uiPanelDrag.reset();
            if (state.hud.uiEditMode)
            {
                state.messages[SkyAppSupport::MessageChannel::UiLayout].show(Tr(U"HudUiEditModeActivated"));
            }
        }
        String TooltipUiEditMode(const SkyAppState&, const SkyAppFrameState&)
        {
            return Tr(U"HudTooltipUiLayoutEdit");
        }

        // resourceAdjust ----------------------------------------------
        String LabelResourceAdjust(const SkyAppState& state, const SkyAppFrameState&)
        {
            return state.hud.showResourceAdjustUi ? String{ U"R+" } : String{ U"r+" };
        }
        void ClickResourceAdjust(SkyAppState& state, const SkyAppFrameState&)
        {
            state.hud.showResourceAdjustUi = (not state.hud.showResourceAdjustUi);
        }
        String TooltipResourceAdjust(const SkyAppState&, const SkyAppFrameState&)
        {
            return Tr(U"HudTooltipManualResourceAdjust");
        }

        // battleCommandScale ------------------------------------------
        String LabelBattleCommandScale(const SkyAppState& state, const SkyAppFrameState&)
        {
            return U"Cmd {}"_fmt(SkyAppUiLayout::ClampBattleCommandIconSize(state.hud.uiLayoutSettings.battleCommandIconSize));
        }
        void ClickBattleCommandScale(SkyAppState& state, const SkyAppFrameState&)
        {
            const int32 currentSize = SkyAppUiLayout::ClampBattleCommandIconSize(state.hud.uiLayoutSettings.battleCommandIconSize);
            state.hud.uiLayoutSettings.battleCommandIconSize = ((currentSize <= 96) ? 128 : 96);
            state.messages[SkyAppSupport::MessageChannel::UiLayout].show(SaveUiLayoutSettings(state.hud.uiLayoutSettings)
                ? U"Battle command scale {}px"_fmt(state.hud.uiLayoutSettings.battleCommandIconSize)
                : U"UI layout save failed");
        }
        String TooltipBattleCommandScale(const SkyAppState&, const SkyAppFrameState&)
        {
            return String{ U"Battle command icon size 96 / 128" };
        }

        // enemyPlan (debug) -------------------------------------------
        String LabelEnemyPlan(const SkyAppState& state, const SkyAppFrameState&)
        {
            switch (state.enemyAi.battlePlanOverride)
            {
            case EnemyBattlePlanOverride::ForceSecureResources:
                return Tr(U"HudEnemyPlanFixedResource");
            case EnemyBattlePlanOverride::ForceAssaultBase:
                return Tr(U"HudEnemyPlanFixedBase");
            case EnemyBattlePlanOverride::Auto:
            default:
                return (state.enemyAi.battlePlan == EnemyBattlePlan::SecureResources)
                    ? Tr(U"HudEnemyPlanAutoResource")
                    : Tr(U"HudEnemyPlanAutoBase");
            }
        }
        void ClickEnemyPlan(SkyAppState& state, const SkyAppFrameState&)
        {
            state.enemyAi.battlePlanOverride = GetNextEnemyBattlePlanOverride(state.enemyAi.battlePlanOverride);
        }
        String TooltipEnemyPlan(const SkyAppState& state, const SkyAppFrameState&)
        {
            switch (state.enemyAi.battlePlanOverride)
            {
            case EnemyBattlePlanOverride::ForceSecureResources:
                return Tr(U"HudEnemyPlanTooltipFixedResource");
            case EnemyBattlePlanOverride::ForceAssaultBase:
                return Tr(U"HudEnemyPlanTooltipFixedBase");
            case EnemyBattlePlanOverride::Auto:
            default:
                return Tr(U"HudEnemyPlanTooltipAuto");
            }
        }
        bool VisibleEnemyPlan(const SkyAppState&, const SkyAppFrameState&)
        {
            return DebugEnemyBattlePlanVisible;
        }

        // --- Settings-panel adapter ----------------------------------
        // The 4 settings panels (sky/camera/terrainVisual/fog) already
        // have a SettingsPanelDescriptor; we generate a HudToggleDescriptor
        // for each one so the bottom-bar registry stays a single list.

        struct SettingsPanelToggleAdapter
        {
            const SettingsPanelDescriptor* desc = nullptr;
        };

        // We need a stable ToggleAdapter per settings panel, so we store
        // the index and dereference the registry inside the lambdas.
        template <size_t Index>
        struct SettingsToggle
        {
            static String Label(const SkyAppState& state, const SkyAppFrameState&)
            {
                const auto& desc = GetSettingsPanelRegistry()[Index];
                return String{ (state.*&SkyAppState::hud).*(desc.expandedFlag) ? desc.labelExpanded : desc.labelCollapsed };
            }
            static void Click(SkyAppState& state, const SkyAppFrameState&)
            {
                const auto& desc = GetSettingsPanelRegistry()[Index];
                bool& expanded = state.hud.*(desc.expandedFlag);
                expanded = (not expanded);
            }
            static String Tooltip(const SkyAppState&, const SkyAppFrameState&)
            {
                const auto& desc = GetSettingsPanelRegistry()[Index];
                return desc.tooltip ? desc.tooltip() : String{};
            }
            static Rect SkyAppPanels::* RectMember()
            {
                return GetSettingsPanelRegistry()[Index].toggleRect;
            }
            static StringView Id()
            {
                return GetSettingsPanelRegistry()[Index].id;
            }
        };

        [[nodiscard]] const Array<HudToggleDescriptor>& BuildHudToggleRegistry()
        {
            static const Array<HudToggleDescriptor> registry = []
            {
                Array<HudToggleDescriptor> r;

                r.push_back({ U"mapMode",     &SkyAppPanels::mapModeToggle,
                              &LabelMapMode, &ClickMapMode, &TooltipMapMode });

                r.push_back({ U"modelHeight", &SkyAppPanels::modelHeightModeToggle,
                              &LabelModelHeight, &ClickModelHeight, &TooltipModelHeight });

                r.push_back({ U"unitEditor", &SkyAppPanels::unitEditorModeToggle,
                              &LabelUnitEditor, &ClickUnitEditor, &TooltipUnitEditor });

                // Settings panels (sky / camera / terrainVisual / fog) ------
                static_assert(SettingsPanelCount == 4, "Update SettingsToggle<> instantiations if SettingsPanelCount changes.");
                r.push_back({ SettingsToggle<0>::Id(), SettingsToggle<0>::RectMember(),
                              &SettingsToggle<0>::Label, &SettingsToggle<0>::Click, &SettingsToggle<0>::Tooltip });
                r.push_back({ SettingsToggle<1>::Id(), SettingsToggle<1>::RectMember(),
                              &SettingsToggle<1>::Label, &SettingsToggle<1>::Click, &SettingsToggle<1>::Tooltip });
                r.push_back({ SettingsToggle<2>::Id(), SettingsToggle<2>::RectMember(),
                              &SettingsToggle<2>::Label, &SettingsToggle<2>::Click, &SettingsToggle<2>::Tooltip });
                r.push_back({ SettingsToggle<3>::Id(), SettingsToggle<3>::RectMember(),
                              &SettingsToggle<3>::Label, &SettingsToggle<3>::Click, &SettingsToggle<3>::Tooltip });

                r.push_back({ U"uiEditMode", &SkyAppPanels::uiEditModeToggle,
                              &LabelUiEditMode, &ClickUiEditMode, &TooltipUiEditMode });

                r.push_back({ U"resourceAdjust", &SkyAppPanels::resourceAdjustToggle,
                              &LabelResourceAdjust, &ClickResourceAdjust, &TooltipResourceAdjust });

                r.push_back({ U"battleCommandScale", &SkyAppPanels::battleCommandScaleToggle,
                              &LabelBattleCommandScale, &ClickBattleCommandScale, &TooltipBattleCommandScale });

                r.push_back({ U"enemyPlan", &SkyAppPanels::enemyPlanToggle,
                              &LabelEnemyPlan, &ClickEnemyPlan, &TooltipEnemyPlan, &VisibleEnemyPlan });

                return r;
            }();
            return registry;
        }
    }

    const Array<HudToggleDescriptor>& GetHudToggleRegistry()
    {
        return BuildHudToggleRegistry();
    }
}
