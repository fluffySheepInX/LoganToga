# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
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

		[[nodiscard]] MainSupport::EnemyBattlePlanOverride GetNextEnemyBattlePlanOverride(const MainSupport::EnemyBattlePlanOverride mode)
		{
			switch (mode)
			{
			case MainSupport::EnemyBattlePlanOverride::Auto:
				return MainSupport::EnemyBattlePlanOverride::ForceSecureResources;

			case MainSupport::EnemyBattlePlanOverride::ForceSecureResources:
				return MainSupport::EnemyBattlePlanOverride::ForceAssaultBase;

			case MainSupport::EnemyBattlePlanOverride::ForceAssaultBase:
			default:
				return MainSupport::EnemyBattlePlanOverride::Auto;
			}
		}

		[[nodiscard]] StringView GetEnemyBattlePlanToggleLabel(const SkyAppState& state)
		{
			switch (state.enemyBattlePlanOverride)
			{
			case MainSupport::EnemyBattlePlanOverride::ForceSecureResources:
				return U"固定:資源";

			case MainSupport::EnemyBattlePlanOverride::ForceAssaultBase:
				return U"固定:拠点";

			case MainSupport::EnemyBattlePlanOverride::Auto:
			default:
				return (state.enemyBattlePlan == MainSupport::EnemyBattlePlan::SecureResources)
					? U"自動:資源"
					: U"自動:拠点";
			}
		}

		[[nodiscard]] StringView GetEnemyBattlePlanToggleTooltip(const SkyAppState& state)
		{
			switch (state.enemyBattlePlanOverride)
			{
			case MainSupport::EnemyBattlePlanOverride::ForceSecureResources:
				return U"敵全体方針: 資源確保に固定";

			case MainSupport::EnemyBattlePlanOverride::ForceAssaultBase:
				return U"敵全体方針: 拠点攻撃に固定";

			case MainSupport::EnemyBattlePlanOverride::Auto:
			default:
				return U"敵全体方針: 状況から自動決定";
			}
		}

		void ResizeBattleWindow(SkyAppResources& resources, SkyAppState& state, const Size& size)
		{
			Window::Resize(size);
			resources.renderTexture = MSRenderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
			state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.camera.getEyePosition(), state.camera.getFocusPosition() };
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
	}

   bool HandleEscMenu(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.showEscMenu)
		{
			return false;
		}

		switch (DrawEscMenu(frame.panels.escMenu))
		{
		case EscMenuAction::Restart:
			ResetMatch(state);
			state.restartMessage.show(U"試合をリスタート");
			state.showEscMenu = false;
			break;

		case EscMenuAction::Title:
			state.requestTitleScene = true;
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1280x720:
			ResizeBattleWindow(resources, state, Size{ 1280, 720 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1600x900:
			ResizeBattleWindow(resources, state, Size{ 1600, 900 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::Resize1920x1080:
			ResizeBattleWindow(resources, state, Size{ 1920, 1080 });
			state.showEscMenu = false;
			break;

		case EscMenuAction::None:
		default:
			break;
		}

		return true;
	}

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
	}

	void DrawContextHud(SkyAppState& state, const SkyAppFrameState& frame)
	{
		DrawMiniMap(state.miniMapExpanded,
			frame.panels,
           state.uiEditMode,
			state.camera,
			state.mapData,
			state.spawnedSappers,
			state.enemySappers,
			state.resourceAreaStates,
			state.selectedSapperIndices);

		if (frame.isEditorMode)
		{
            DrawMapEditorPanel(state.mapEditor, state.mapData, state.currentMapPath, frame.panels.mapEditor);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && state.showBlacksmithMenu)
		{
			DrawBlacksmithMenu(frame.panels,
				state.spawnedSappers,
               state.mapData,
				state.mapData.playerBasePosition,
				state.mapData.sapperRallyPoint,
				state.playerResources,
				state.playerTier,
             state.unitEditorSettings,
               state.modelHeightSettings,
				state.blacksmithMenuMessage);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
		{
            const size_t selectedIndex = state.selectedSapperIndices.front();

			const SapperMenuAction menuAction = DrawSapperMenu(frame.panels,
				state.spawnedSappers,
				state.playerResources,
				selectedIndex,
				state.blacksmithMenuMessage);
			if (menuAction == SapperMenuAction::UseExplosionSkill)
			{
				TryUsePlayerSapperExplosionSkill(state, selectedIndex);
			}
			else if (menuAction == SapperMenuAction::Retreat)
			{
				TryOrderPlayerSapperRetreat(state, selectedIndex);
			}
		}

		if (frame.showUnitEditor)
		{
			DrawUnitEditor(frame.panels,
               state.uiEditMode,
				state.unitEditorSettings,
                state.unitEditorSelection,
               state.unitEditorPage,
				state.spawnedSappers,
				state.enemySappers,
				state.unitEditorMessage);
		}

		if (frame.showMillStatusEditor && state.selectedMillIndex)
		{
			DrawMillStatusEditor(frame.panels, state.mapData, *state.selectedMillIndex, MapDataPath, state.mapDataMessage);
		}
	}

	void DrawHudModeToggles(SkyAppState& state, const SkyAppFrameState& frame)
	{
		if (not state.showUI)
		{
			return;
		}

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

		SimpleGUI::CheckBox(state.showUI, U"UI", SkyAppUiLayout::UiTogglePosition(frame.panels.uiToggle));

	}
}
