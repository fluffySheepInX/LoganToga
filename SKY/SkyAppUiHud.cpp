# include "SkyAppLoopInternal.hpp"
# include "SkyAppUi.hpp"
# include "MapEditor.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppFlow
{
	using namespace MainSupport;
	using namespace SkyAppSupport;

	bool HandleEscMenu(SkyAppState& state, const SkyAppFrameState& frame)
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

		const bool wasSkySettingsExpanded = state.skySettingsExpanded;
		const bool wasCameraSettingsExpanded = state.cameraSettingsExpanded;
		DrawSkySettingsPanel(state.sky, state.skySettingsExpanded, frame.panels);

		if ((not wasSkySettingsExpanded) && state.skySettingsExpanded)
		{
			state.cameraSettingsExpanded = false;
		}

		DrawCameraSettingsPanel(state.camera,
			state.cameraSettings,
			state.cameraSettingsExpanded,
			resources.birdModel,
			resources.ashigaruModel,
			state.cameraSaveMessage,
			frame.panels);

		if ((not wasCameraSettingsExpanded) && state.cameraSettingsExpanded)
		{
			state.skySettingsExpanded = false;
		}
	}

	void DrawContextHud(SkyAppState& state, const SkyAppFrameState& frame)
	{
		DrawMiniMap(state.miniMapExpanded,
			frame.panels,
			state.camera,
			state.mapData,
			state.spawnedSappers,
			state.enemySappers,
			state.resourceAreaStates,
			state.selectedSapperIndices);

		if (frame.isEditorMode)
		{
			DrawMapEditorPanel(state.mapEditor, state.mapData, MapDataPath, frame.panels.mapEditor);
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
				SapperCost,
				state.blacksmithMenuMessage);
		}

		if ((not frame.isEditorMode) && (not state.playerWon) && (state.selectedSapperIndices.size() == 1))
		{
			DrawSapperMenu(frame.panels,
				state.spawnedSappers,
               state.mapData,
				state.mapData.playerBasePosition,
				state.mapData.sapperRallyPoint,
				state.playerResources,
				state.playerTier,
				SapperCost,
				state.blacksmithMenuMessage);
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
		}

		if (DrawTextButton(frame.panels.modelHeightModeToggle, state.modelHeightEditMode ? U"▲" : U"△"))
		{
			state.modelHeightEditMode = not state.modelHeightEditMode;
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
