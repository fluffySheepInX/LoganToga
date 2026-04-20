# include "SkyAppLoopInternal.hpp"
# include "SkyAppPanelRegistry.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MapEditor.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;
	using SkyAppText::Tr;

	namespace
	{
		inline constexpr bool ShowDebugEnemyBattlePlanToggle =
		#if _DEBUG
			true;
		#else
			false;
		#endif

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

		[[nodiscard]] String GetEnemyBattlePlanToggleLabel(const SkyAppState& state)
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

	  [[nodiscard]] String GetEnemyBattlePlanToggleTooltip(const SkyAppState& state)
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

		[[nodiscard]] String GetBattleCommandScaleToggleLabel(const SkyAppState& state)
		{
			return U"Cmd {}"_fmt(SkyAppUiLayout::ClampBattleCommandIconSize(state.hud.uiLayoutSettings.battleCommandIconSize));
		}

		void DrawBottomToggleTooltip(const Rect& anchorRect, const StringView label)
		{
			const double width = Max(104.0, (SimpleGUI::GetFont()(label).region().w + 24.0));
			RectF tooltipRect{ (anchorRect.center().x - width * 0.5), (anchorRect.y - 34), width, 26 };
			tooltipRect.x = Clamp(tooltipRect.x, 8.0, Max(8.0, (Scene::Width() - width - 8.0)));
			tooltipRect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.12, 0.96 });
			tooltipRect.rounded(8).drawFrame(1, 0, ColorF{ 0.72, 0.82, 0.94, 0.84 });
			SimpleGUI::GetFont()(label).drawAt(tooltipRect.center(), Palette::White);
		}

		[[nodiscard]] bool IsBottomControlRevealHotZoneHovered()
		{
			return SkyAppUiLayout::BottomControlRevealHotZone(Scene::Width(), Scene::Height()).mouseOver();
		}

		// Returns the bottom-bar toggle Rect that should anchor the
		// shared editor buttons (text colors / panel skin). Uses the
		// rightmost visible toggle from the HUD toggle registry, with
		// the battle-command-scale toggle as a final fallback.
		[[nodiscard]] Rect GetBottomActionAnchor(const SkyAppState& state, const SkyAppFrameState& frame)
		{
			const Rect* anchor = nullptr;
			for (const auto& desc : GetHudToggleRegistry())
			{
				if (desc.isVisible && (not desc.isVisible(state, frame)))
				{
					continue;
				}
				const Rect& r = frame.panels.*(desc.rect);
				if ((not anchor) || (r.x > anchor->x))
				{
					anchor = &r;
				}
			}
			return anchor ? *anchor : frame.panels.battleCommandScaleToggle;
		}
	}

	void DrawHudModeToggles(SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (IsBottomControlRevealHotZoneHovered())
		{
			state.hud.showUI = true;
		}

		if (not state.hud.showUI)
		{
			return;
		}

       UiInternal::DrawNinePatchPanelFrame(frame.panels.uiToggle,
			U"",
			ColorF{ 1.0, 0.92 },
			UiInternal::DefaultPanelFrameColor,
			UiInternal::DefaultPanelTitleColor,
			MainSupport::PanelSkinTarget::Hud);

		String hoveredTooltip;
		Optional<Rect> hoveredTooltipRect;

		if (DrawTextButton(frame.panels.mapModeToggle, frame.isEditorMode ? U"Åö" : U"Åô"))
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
		if (frame.panels.mapModeToggle.mouseOver())
		{
            hoveredTooltip = frame.isEditorMode ? Tr(U"HudTooltipReturnToPlay") : Tr(U"HudTooltipMapEditMode");
			hoveredTooltipRect = frame.panels.mapModeToggle;
		}

		if (DrawTextButton(frame.panels.modelHeightModeToggle, state.editor.modelHeightEditMode ? U"Å£" : U"Å¢"))
		{
			state.editor.modelHeightEditMode = not state.editor.modelHeightEditMode;
			if (state.editor.modelHeightEditMode)
			{
				state.editor.unitEditorMode = false;
			}
		}
		if (frame.panels.modelHeightModeToggle.mouseOver())
		{
         hoveredTooltip = Tr(U"HudTooltipModelHeightScale");
			hoveredTooltipRect = frame.panels.modelHeightModeToggle;
		}

		if (DrawTextButton(frame.panels.unitEditorModeToggle, state.editor.unitEditorMode ? U"Unit" : U"unit"))
		{
			state.editor.unitEditorMode = not state.editor.unitEditorMode;
			if (state.editor.unitEditorMode)
			{
				state.editor.modelHeightEditMode = false;
			}
		}
		if (frame.panels.unitEditorModeToggle.mouseOver())
		{
          hoveredTooltip = Tr(U"HudTooltipUnitParameterEditor");
			hoveredTooltipRect = frame.panels.unitEditorModeToggle;
		}

		// Settings panel toggle buttons are driven by the registry so that
		// adding a panel only requires editing SkyAppPanelRegistry.cpp.
		for (const auto& panel : GetSettingsPanelRegistry())
		{
			const Rect& toggleRect = frame.panels.*(panel.toggleRect);
			bool& expanded = state.hud.*(panel.expandedFlag);
			if (DrawTextButton(toggleRect, (expanded ? panel.labelExpanded : panel.labelCollapsed)))
			{
				expanded = not expanded;
			}
			if (toggleRect.mouseOver())
			{
				hoveredTooltip = panel.tooltip();
				hoveredTooltipRect = toggleRect;
			}
		}

		if (DrawTextButton(frame.panels.uiEditModeToggle, state.hud.uiEditMode ? U"UI+" : U"ui+"))
		{
			state.hud.uiEditMode = not state.hud.uiEditMode;
			state.hud.uiPanelDrag.reset();

			if (state.hud.uiEditMode)
			{
                state.messages[SkyAppSupport::MessageChannel::UiLayout].show(Tr(U"HudUiEditModeActivated"));
			}
		}
		if (frame.panels.uiEditModeToggle.mouseOver())
		{
         hoveredTooltip = Tr(U"HudTooltipUiLayoutEdit");
			hoveredTooltipRect = frame.panels.uiEditModeToggle;
		}

		if (DrawTextButton(frame.panels.resourceAdjustToggle, state.hud.showResourceAdjustUi ? U"éëåπ+" : U"éëåπ"))
		{
			state.hud.showResourceAdjustUi = not state.hud.showResourceAdjustUi;
		}
		if (frame.panels.resourceAdjustToggle.mouseOver())
		{
           hoveredTooltip = Tr(U"HudTooltipManualResourceAdjust");
			hoveredTooltipRect = frame.panels.resourceAdjustToggle;
		}

		if (DrawTextButton(frame.panels.battleCommandScaleToggle, GetBattleCommandScaleToggleLabel(state)))
		{
			const int32 currentSize = SkyAppUiLayout::ClampBattleCommandIconSize(state.hud.uiLayoutSettings.battleCommandIconSize);
			state.hud.uiLayoutSettings.battleCommandIconSize = ((currentSize <= 96) ? 128 : 96);
			state.messages[SkyAppSupport::MessageChannel::UiLayout].show(SaveUiLayoutSettings(state.hud.uiLayoutSettings)
				? U"Battle command scale {}px"_fmt(state.hud.uiLayoutSettings.battleCommandIconSize)
				: U"UI layout save failed");
		}
		if (frame.panels.battleCommandScaleToggle.mouseOver())
		{
			hoveredTooltip = U"Battle command icon size 96 / 128";
			hoveredTooltipRect = frame.panels.battleCommandScaleToggle;
		}

		if (ShowDebugEnemyBattlePlanToggle)
		{
			if (DrawTextButton(frame.panels.enemyPlanToggle, GetEnemyBattlePlanToggleLabel(state)))
			{
				state.enemyAi.battlePlanOverride = GetNextEnemyBattlePlanOverride(state.enemyAi.battlePlanOverride);
			}

			if (frame.panels.enemyPlanToggle.mouseOver())
			{
				hoveredTooltip = GetEnemyBattlePlanToggleTooltip(state);
				hoveredTooltipRect = frame.panels.enemyPlanToggle;
			}
		}

		if (hoveredTooltipRect && (not hoveredTooltip.isEmpty()))
		{
			DrawBottomToggleTooltip(*hoveredTooltipRect, hoveredTooltip);
		}
	}

	void DrawHudFooter(SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.hud.showUI)
		{
			return;
		}

		if (state.messages[SkyAppSupport::MessageChannel::MapData].isVisible())
		{
			const Vec2 messagePosition = SkyAppUiLayout::BottomMessagePosition(frame.panels.uiToggle, 0);
			SimpleGUI::GetFont()(state.messages[SkyAppSupport::MessageChannel::MapData].text).draw(messagePosition, ColorF{ 0.12 });
		}

		if (state.messages[SkyAppSupport::MessageChannel::Restart].isVisible())
		{
			const Vec2 messagePosition = SkyAppUiLayout::BottomMessagePosition(frame.panels.uiToggle, 1);
			SimpleGUI::GetFont()(state.messages[SkyAppSupport::MessageChannel::Restart].text).draw(messagePosition, ColorF{ 0.12 });
		}

     DrawCheckBox(SkyAppUiLayout::UiToggleCheckBox(frame.panels.uiToggle), state.hud.showUI, Tr(U"CommonUi"));

		if (state.hud.showUI)
		{
			const Rect anchorToggle = GetBottomActionAnchor(state, frame);
			const Rect editorTextColorsButtonRect = SkyAppUiLayout::BottomEditorTextColorsButton(anchorToggle);
			const Rect panelSkinButtonRect = SkyAppUiLayout::BottomPanelSkinButton(editorTextColorsButtonRect);

			if (UiInternal::DrawEditorIconButton(editorTextColorsButtonRect, U"êF"))
			{
				UiInternal::OpenSharedEditorTextColorEditor();
			}

			if (UiInternal::DrawEditorIconButton(panelSkinButtonRect, U"òg"))
			{
				UiInternal::OpenSharedPanelSkinSelector();
			}

			if (editorTextColorsButtonRect.mouseOver())
			{
				DrawBottomToggleTooltip(editorTextColorsButtonRect, U"Text colors");
			}

			if (panelSkinButtonRect.mouseOver())
			{
				DrawBottomToggleTooltip(panelSkinButtonRect, U"Panel skin");
			}
		}
	}
}
