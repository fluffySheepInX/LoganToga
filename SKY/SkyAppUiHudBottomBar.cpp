# include "SkyAppLoopInternal.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;
	using SkyAppText::TextId;
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
			switch (state.enemyBattlePlanOverride)
			{
			case EnemyBattlePlanOverride::ForceSecureResources:
                return Tr(TextId::HudEnemyPlanFixedResource);

			case EnemyBattlePlanOverride::ForceAssaultBase:
                return Tr(TextId::HudEnemyPlanFixedBase);

			case EnemyBattlePlanOverride::Auto:
			default:
				return (state.enemyBattlePlan == EnemyBattlePlan::SecureResources)
                  ? Tr(TextId::HudEnemyPlanAutoResource)
					: Tr(TextId::HudEnemyPlanAutoBase);
			}
		}

      [[nodiscard]] String GetEnemyBattlePlanToggleTooltip(const SkyAppState& state)
		{
			switch (state.enemyBattlePlanOverride)
			{
			case EnemyBattlePlanOverride::ForceSecureResources:
               return Tr(TextId::HudEnemyPlanTooltipFixedResource);

			case EnemyBattlePlanOverride::ForceAssaultBase:
               return Tr(TextId::HudEnemyPlanTooltipFixedBase);

			case EnemyBattlePlanOverride::Auto:
			default:
              return Tr(TextId::HudEnemyPlanTooltipAuto);
			}
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
	}

	void DrawHudModeToggles(SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (IsBottomControlRevealHotZoneHovered())
		{
			state.showUI = true;
		}

		if (not state.showUI)
		{
			return;
		}

		UiInternal::DrawNinePatchPanelFrame(frame.panels.uiToggle, U"", ColorF{ 1.0, 0.92 });

		String hoveredTooltip;
		Optional<Rect> hoveredTooltipRect;

		if (DrawTextButton(frame.panels.mapModeToggle, frame.isEditorMode ? U"★" : U"☆"))
		{
			state.appMode = frame.isEditorMode ? AppMode::Play : AppMode::EditMap;
			state.showBlacksmithMenu = false;
			state.selectedSapperIndices.clear();
			state.selectedMillIndex.reset();
			state.selectionDragStart.reset();
			state.mapEditor.hoveredGroundPosition.reset();
			state.unitEditorMode = false;
			state.modelHeightEditMode = false;
		}
		if (frame.panels.mapModeToggle.mouseOver())
		{
            hoveredTooltip = frame.isEditorMode ? Tr(TextId::HudTooltipReturnToPlay) : Tr(TextId::HudTooltipMapEditMode);
			hoveredTooltipRect = frame.panels.mapModeToggle;
		}

		if (DrawTextButton(frame.panels.modelHeightModeToggle, state.modelHeightEditMode ? U"▲" : U"△"))
		{
			state.modelHeightEditMode = not state.modelHeightEditMode;
			if (state.modelHeightEditMode)
			{
				state.unitEditorMode = false;
			}
		}
		if (frame.panels.modelHeightModeToggle.mouseOver())
		{
         hoveredTooltip = Tr(TextId::HudTooltipModelHeightScale);
			hoveredTooltipRect = frame.panels.modelHeightModeToggle;
		}

		if (DrawTextButton(frame.panels.unitEditorModeToggle, state.unitEditorMode ? U"Unit" : U"unit"))
		{
			state.unitEditorMode = not state.unitEditorMode;
			if (state.unitEditorMode)
			{
				state.modelHeightEditMode = false;
			}
		}
		if (frame.panels.unitEditorModeToggle.mouseOver())
		{
          hoveredTooltip = Tr(TextId::HudTooltipUnitParameterEditor);
			hoveredTooltipRect = frame.panels.unitEditorModeToggle;
		}

		if (DrawTextButton(frame.panels.skySettingsToggle, state.skySettingsExpanded ? U"◆" : U"◇"))
		{
			state.skySettingsExpanded = not state.skySettingsExpanded;
		}
		if (frame.panels.skySettingsToggle.mouseOver())
		{
         hoveredTooltip = Tr(TextId::HudTooltipSkySettingsPanel);
			hoveredTooltipRect = frame.panels.skySettingsToggle;
		}

		if (DrawTextButton(frame.panels.cameraSettingsToggle, state.cameraSettingsExpanded ? U"◉" : U"◎"))
		{
			state.cameraSettingsExpanded = not state.cameraSettingsExpanded;
		}
		if (frame.panels.cameraSettingsToggle.mouseOver())
		{
           hoveredTooltip = Tr(TextId::HudTooltipCameraSettingsPanel);
			hoveredTooltipRect = frame.panels.cameraSettingsToggle;
		}

		if (DrawTextButton(frame.panels.terrainVisualToggle, state.terrainVisualSettingsExpanded ? U"地" : U"土"))
		{
			state.terrainVisualSettingsExpanded = not state.terrainVisualSettingsExpanded;
		}
		if (frame.panels.terrainVisualToggle.mouseOver())
		{
            hoveredTooltip = Tr(TextId::HudTooltipTerrainNoiseSettings);
			hoveredTooltipRect = frame.panels.terrainVisualToggle;
		}

		if (DrawTextButton(frame.panels.fogSettingsToggle, state.fogSettingsExpanded ? U"Fog" : U"fog"))
		{
			state.fogSettingsExpanded = not state.fogSettingsExpanded;
		}
		if (frame.panels.fogSettingsToggle.mouseOver())
		{
			hoveredTooltip = U"Fog editor panel";
			hoveredTooltipRect = frame.panels.fogSettingsToggle;
		}

		if (DrawTextButton(frame.panels.uiEditModeToggle, state.uiEditMode ? U"UI+" : U"ui+"))
		{
			state.uiEditMode = not state.uiEditMode;
			state.uiPanelDrag.reset();

			if (state.uiEditMode)
			{
                state.uiLayoutMessage.show(Tr(TextId::HudUiEditModeActivated));
			}
		}
		if (frame.panels.uiEditModeToggle.mouseOver())
		{
         hoveredTooltip = Tr(TextId::HudTooltipUiLayoutEdit);
			hoveredTooltipRect = frame.panels.uiEditModeToggle;
		}

		if (DrawTextButton(frame.panels.resourceAdjustToggle, state.showResourceAdjustUi ? U"資源+" : U"資源"))
		{
			state.showResourceAdjustUi = not state.showResourceAdjustUi;
		}
		if (frame.panels.resourceAdjustToggle.mouseOver())
		{
           hoveredTooltip = Tr(TextId::HudTooltipManualResourceAdjust);
			hoveredTooltipRect = frame.panels.resourceAdjustToggle;
		}

		if (ShowDebugEnemyBattlePlanToggle)
		{
			if (DrawTextButton(frame.panels.enemyPlanToggle, GetEnemyBattlePlanToggleLabel(state)))
			{
				state.enemyBattlePlanOverride = GetNextEnemyBattlePlanOverride(state.enemyBattlePlanOverride);
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
		if (not state.showUI)
		{
			return;
		}

		if (state.mapDataMessage.isVisible())
		{
			const Vec2 messagePosition = SkyAppUiLayout::BottomMessagePosition(frame.panels.uiToggle, 0);
			SimpleGUI::GetFont()(state.mapDataMessage.text).draw(messagePosition, ColorF{ 0.12 });
		}

		if (state.restartMessage.isVisible())
		{
			const Vec2 messagePosition = SkyAppUiLayout::BottomMessagePosition(frame.panels.uiToggle, 1);
			SimpleGUI::GetFont()(state.restartMessage.text).draw(messagePosition, ColorF{ 0.12 });
		}

     DrawCheckBox(SkyAppUiLayout::UiToggleCheckBox(frame.panels.uiToggle), state.showUI, Tr(TextId::CommonUi));

		if (state.showUI)
		{
            const Rect& anchorToggle = ShowDebugEnemyBattlePlanToggle ? frame.panels.enemyPlanToggle : frame.panels.resourceAdjustToggle;
			const Rect editorTextColorsButtonRect = SkyAppUiLayout::BottomEditorTextColorsButton(anchorToggle);

			if (UiInternal::DrawEditorIconButton(editorTextColorsButtonRect, U"色"))
			{
				UiInternal::OpenSharedEditorTextColorEditor();
			}
		}
	}
}
