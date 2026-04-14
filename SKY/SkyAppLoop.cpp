# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace Detail
	{
		void NormalizeFrameStateInputs(SkyAppState& state)
		{
          constexpr int32 BattleCommandSlotCount = 4;

			state.mapEditor.enabled = (state.appMode == AppMode::EditMap);
			state.battleCommandUnlockedSlotCount = Clamp(state.battleCommandUnlockedSlotCount, 1, BattleCommandSlotCount);
			state.battleCommandSelectedSlotIndex = Min(state.battleCommandSelectedSlotIndex, static_cast<size_t>(state.battleCommandUnlockedSlotCount - 1));

			if (not IsValidMillIndex(state, state.selectedMillIndex))
			{
				state.selectedMillIndex.reset();
			}
		}

		void RefreshTerrainSurface(SkyAppState& state)
		{
			const uint64 revision = ComputeTerrainSurfaceRevision(state.mapData, state.terrainVisualSettings);
			if ((state.terrainSurfaceRevision == revision) && (not state.terrainSurface.cells.isEmpty()))
			{
				return;
			}

			state.terrainSurface = BuildTerrainSurface(state.mapData, state.terrainVisualSettings);
			state.terrainSurfaceRevision = revision;
		}

		SkyAppFrameState BuildFrameState(const SkyAppState& state)
		{
			SkyAppFrameState frame;
			const bool resourcePanelExpandedForLayout = state.showResourceAdjustUi;
			const bool isEditingResourcePanel = state.uiPanelDrag
				&& (state.uiPanelDrag->panel == UiLayoutPanel::ResourcePanel);
			const bool resourcePanelShowStoredHeight = (state.uiEditMode && isEditingResourcePanel);
			frame.panels = SkyAppPanels{ state.uiLayoutSettings, state.skySettingsExpanded, state.cameraSettingsExpanded, state.terrainVisualSettingsExpanded, state.miniMapExpanded, resourcePanelExpandedForLayout, resourcePanelShowStoredHeight };
			frame.isEditorMode = (state.appMode == AppMode::EditMap);
			frame.showEscMenu = state.showEscMenu;
			const bool hasValidSelectedMill = IsValidMillIndex(state, state.selectedMillIndex);
			frame.showSapperMenu = ((state.selectedSapperIndices.size() == 1) && (not state.playerWon));
			frame.showMillStatusEditor = ((not frame.isEditorMode) && hasValidSelectedMill);
			frame.showUnitEditor = (state.showUI && state.unitEditorMode && (not frame.isEditorMode) && (not state.playerWon));
			frame.isHoveringUI = frame.panels.isHoveringUi(state.showUI, state.skySettingsExpanded, state.cameraSettingsExpanded, state.terrainVisualSettingsExpanded, frame.isEditorMode, state.showBlacksmithMenu, frame.showSapperMenu, frame.showMillStatusEditor, state.modelHeightEditMode, frame.showUnitEditor);
			for (const UnitRenderModel renderModel : GetUnitRenderModels())
			{
				frame.previewRenderPositions[GetUnitRenderModelIndex(renderModel)] = GetUnitRenderModelDisplayPosition(renderModel).movedBy(0, GetModelHeightOffset(state.modelHeightSettings, renderModel), 0);
			}
			return frame;
		}
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.cameraSettings = LoadCameraSettings();
		EnsureValidCameraSettings(state.cameraSettings);
		ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"InitializeSkyAppState");
		state.modelHeightSettings = LoadModelHeightSettings();
		state.uiLayoutSettings = LoadUiLayoutSettings(Scene::Width(), Scene::Height());
		state.initialPlayerResources = LoadInitialPlayerResources();
		state.unitEditorSettings = LoadUnitEditorSettings();
		state.currentMapPath = MainSupport::MapDataPath;
		const MapDataLoadResult initialMapDataLoad = LoadMapDataWithStatus(MapDataPath);
		state.mapData = initialMapDataLoad.mapData;
		state.terrainSurfaceRevision = 0;
		Detail::RefreshTerrainSurface(state);
		state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.cameraSettings.eye, state.cameraSettings.focus };
		ResetMatch(state);

		if (not initialMapDataLoad.message.isEmpty())
		{
			state.mapDataMessage.show(initialMapDataLoad.message, 4.0);
		}
	}
}
