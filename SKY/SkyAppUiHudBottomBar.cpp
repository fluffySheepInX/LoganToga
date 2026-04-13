# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "SkyAppUiInternal.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

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

		[[nodiscard]] StringView GetEnemyBattlePlanToggleLabel(const SkyAppState& state)
		{
			switch (state.enemyBattlePlanOverride)
			{
			case EnemyBattlePlanOverride::ForceSecureResources:
				return U"固定:資源";

			case EnemyBattlePlanOverride::ForceAssaultBase:
				return U"固定:拠点";

			case EnemyBattlePlanOverride::Auto:
			default:
				return (state.enemyBattlePlan == EnemyBattlePlan::SecureResources)
					? U"自動:資源"
					: U"自動:拠点";
			}
		}

		[[nodiscard]] StringView GetEnemyBattlePlanToggleTooltip(const SkyAppState& state)
		{
			switch (state.enemyBattlePlanOverride)
			{
			case EnemyBattlePlanOverride::ForceSecureResources:
				return U"敵全体方針: 資源確保に固定";

			case EnemyBattlePlanOverride::ForceAssaultBase:
				return U"敵全体方針: 拠点攻撃に固定";

			case EnemyBattlePlanOverride::Auto:
			default:
				return U"敵全体方針: 状況から自動決定";
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
			hoveredTooltip = frame.isEditorMode ? U"プレイ表示に戻す" : U"マップ編集モード";
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
			hoveredTooltip = U"モデル高さ / スケール調整";
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
			hoveredTooltip = U"ユニットパラメータエディタ";
			hoveredTooltipRect = frame.panels.unitEditorModeToggle;
		}

		if (DrawTextButton(frame.panels.skySettingsToggle, state.skySettingsExpanded ? U"◆" : U"◇"))
		{
			state.skySettingsExpanded = not state.skySettingsExpanded;
		}
		if (frame.panels.skySettingsToggle.mouseOver())
		{
			hoveredTooltip = U"空設定パネル";
			hoveredTooltipRect = frame.panels.skySettingsToggle;
		}

		if (DrawTextButton(frame.panels.cameraSettingsToggle, state.cameraSettingsExpanded ? U"◉" : U"◎"))
		{
			state.cameraSettingsExpanded = not state.cameraSettingsExpanded;
		}
		if (frame.panels.cameraSettingsToggle.mouseOver())
		{
			hoveredTooltip = U"カメラ設定パネル";
			hoveredTooltipRect = frame.panels.cameraSettingsToggle;
		}

		if (DrawTextButton(frame.panels.terrainVisualToggle, state.terrainVisualSettingsExpanded ? U"地" : U"土"))
		{
			state.terrainVisualSettingsExpanded = not state.terrainVisualSettingsExpanded;
		}
		if (frame.panels.terrainVisualToggle.mouseOver())
		{
			hoveredTooltip = U"地面ノイズ設定";
			hoveredTooltipRect = frame.panels.terrainVisualToggle;
		}

		if (DrawTextButton(frame.panels.uiEditModeToggle, state.uiEditMode ? U"UI+" : U"ui+"))
		{
			state.uiEditMode = not state.uiEditMode;
			state.uiPanelDrag.reset();

			if (state.uiEditMode)
			{
				state.uiLayoutMessage.show(U"UI Edit: drag panels / grid snap");
			}
		}
		if (frame.panels.uiEditModeToggle.mouseOver())
		{
			hoveredTooltip = U"UI レイアウト編集";
			hoveredTooltipRect = frame.panels.uiEditModeToggle;
		}

		if (DrawTextButton(frame.panels.resourceAdjustToggle, state.showResourceAdjustUi ? U"資源+" : U"資源"))
		{
			state.showResourceAdjustUi = not state.showResourceAdjustUi;
		}
		if (frame.panels.resourceAdjustToggle.mouseOver())
		{
			hoveredTooltip = U"資源量の手動調整";
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

		DrawCheckBox(SkyAppUiLayout::UiToggleCheckBox(frame.panels.uiToggle), state.showUI, U"UI");

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
