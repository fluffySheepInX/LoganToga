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
		void ResizeBattleWindow(SkyAppResources& resources, SkyAppState& state, const Size& size)
		{
			Window::Resize(size);
			resources.renderTexture = MSRenderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
			state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.camera.getEyePosition(), state.camera.getFocusPosition() };
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

		DrawSkySettingsPanel(state.sky, state.skySettingsExpanded, frame.panels);

		DrawCameraSettingsPanel(state.camera,
			state.cameraSettings,
			state.cameraSettingsExpanded,
			resources.birdModel,
			resources.ashigaruModel,
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
				state.blacksmithMenuMessage);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
		{
            const size_t selectedIndex = state.selectedSapperIndices.front();

			if (DrawSapperMenu(frame.panels,
				state.spawnedSappers,
				state.playerResources,
                selectedIndex,
               state.blacksmithMenuMessage) == SapperMenuAction::UseExplosionSkill)
			{
				TryUsePlayerSapperExplosionSkill(state, selectedIndex);
			}
		}

		if (frame.showUnitEditor)
		{
			DrawUnitEditor(frame.panels,
               state.uiEditMode,
				state.unitEditorSettings,
				state.unitEditorSection,
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

		if (DrawTextButton(frame.panels.modelHeightModeToggle, state.modelHeightEditMode ? U"▲" : U"△"))
		{
			state.modelHeightEditMode = not state.modelHeightEditMode;
           if (state.modelHeightEditMode)
			{
				state.unitEditorMode = false;
			}
		}

		if (DrawTextButton(frame.panels.unitEditorModeToggle, state.unitEditorMode ? U"Unit" : U"unit"))
		{
			state.unitEditorMode = not state.unitEditorMode;
			if (state.unitEditorMode)
			{
				state.modelHeightEditMode = false;
			}
		}

		if (DrawTextButton(frame.panels.skySettingsToggle, state.skySettingsExpanded ? U"◆" : U"◇"))
		{
			state.skySettingsExpanded = not state.skySettingsExpanded;
		}

		if (DrawTextButton(frame.panels.cameraSettingsToggle, state.cameraSettingsExpanded ? U"◉" : U"◎"))
		{
			state.cameraSettingsExpanded = not state.cameraSettingsExpanded;
		}

		if (DrawTextButton(frame.panels.uiEditModeToggle, state.uiEditMode ? U"UI+" : U"ui+"))
		{
			state.uiEditMode = not state.uiEditMode;
			state.uiPanelDrag.reset();

			if (state.uiEditMode)
			{
				state.uiLayoutMessage.show(U"UI Edit: drag mini map / resource");
			}
		}

		if (DrawTextButton(frame.panels.resourceAdjustToggle, state.showResourceAdjustUi ? U"資源+" : U"資源"))
		{
			state.showResourceAdjustUi = not state.showResourceAdjustUi;
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

		if (state.showUI)
		{
			SimpleGUI::Slider(U"time: {:.2f}"_fmt(state.skyTime),
				state.skyTime,
				-2.0,
				4.0,
				SkyAppUiLayout::TimeSliderPosition(frame.panels.timeSlider),
				SkyAppUiLayout::TimeSliderLabelWidth(),
				SkyAppUiLayout::TimeSliderTrackWidth(frame.panels.timeSlider));
		}
	}
}
