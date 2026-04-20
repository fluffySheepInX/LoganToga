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

			state.editor.mapEditor.enabled = (state.env.appMode == AppMode::EditMap);
			state.battle.battleCommandUnlockedSlotCount = Clamp(state.battle.battleCommandUnlockedSlotCount, 1, BattleCommandSlotCount);
			state.battle.battleCommandSelectedSlotIndex = Min(state.battle.battleCommandSelectedSlotIndex, static_cast<size_t>(state.battle.battleCommandUnlockedSlotCount - 1));

			if (not IsValidMillIndex(state, state.battle.selectedMillIndex))
			{
				state.battle.selectedMillIndex.reset();
			}
		}

		void RefreshTerrainSurface(SkyAppState& state)
		{
			const uint64 revision = ComputeTerrainSurfaceRevision(state.world.mapData, state.world.terrainVisualSettings);
			if ((state.world.terrainSurfaceRevision == revision) && (not state.world.terrainSurface.cells.isEmpty()))
			{
				return;
			}

			state.world.terrainSurface = BuildTerrainSurface(state.world.mapData, state.world.terrainVisualSettings);
			state.world.terrainSurfaceRevision = revision;
		}

      SkyAppFrameState BuildFrameState(const SkyAppState& state)
		{
			SkyAppFrameState frame;
			const bool resourcePanelExpandedForLayout = state.hud.showResourceAdjustUi;
			const bool isEditingResourcePanel = state.hud.uiPanelDrag
				&& (state.hud.uiPanelDrag->panel == UiLayoutPanel::ResourcePanel);
			const bool resourcePanelShowStoredHeight = (state.hud.uiEditMode && isEditingResourcePanel);
			frame.panels = SkyAppPanels{ state.hud.uiLayoutSettings, state.hud.skySettingsExpanded, state.hud.cameraSettingsExpanded, state.hud.terrainVisualSettingsExpanded, state.hud.miniMapExpanded, resourcePanelExpandedForLayout, resourcePanelShowStoredHeight };
			frame.isEditorMode = (state.env.appMode == AppMode::EditMap);
			frame.showEscMenu = state.hud.showEscMenu;
			const bool hasValidSelectedMill = IsValidMillIndex(state, state.battle.selectedMillIndex);
			frame.showSapperMenu = ((state.battle.selectedSapperIndices.size() == 1) && (not state.battle.playerWon));
			frame.showMillStatusEditor = ((not frame.isEditorMode) && hasValidSelectedMill);
			frame.showUnitEditor = (state.hud.showUI && state.editor.unitEditorMode && (not frame.isEditorMode) && (not state.battle.playerWon));
			frame.isHoveringUI = frame.panels.isHoveringUi(state.hud.showUI, state.hud.skySettingsExpanded, state.hud.cameraSettingsExpanded, state.hud.terrainVisualSettingsExpanded, state.hud.fogSettingsExpanded, frame.isEditorMode, state.hud.showBlacksmithMenu, frame.showSapperMenu, frame.showMillStatusEditor, state.editor.modelHeightEditMode, frame.showUnitEditor);
			for (const UnitRenderModel renderModel : GetUnitRenderModels())
			{
				frame.previewRenderPositions[GetUnitRenderModelIndex(renderModel)] = GetUnitRenderModelDisplayPosition(renderModel).movedBy(0, GetModelHeightOffset(state.editor.modelHeightSettings, renderModel), 0);
			}

			return frame;
		}

		SkyAppFrameState BuildFrameState(const SkyAppState& state, const SkyAppResources& resources)
		{
         SkyAppFrameState frame = BuildFrameState(state);

			frame.modelHeightPreviewRenderPositions.clear();
			frame.modelHeightPreviewRenderPositions.reserve(resources.modelHeightEditorPreviewPaths.size());
         constexpr int32 PreviewColumnCount = 4;
			constexpr double PreviewColumnSpacing = 5.0;
			constexpr double PreviewRowSpacing = 6.0;
			const int32 previewCount = static_cast<int32>(resources.modelHeightEditorPreviewPaths.size());
			const int32 rowCount = Max(1, ((previewCount + PreviewColumnCount - 1) / PreviewColumnCount));
			for (size_t index = 0; index < resources.modelHeightEditorPreviewPaths.size(); ++index)
			{
				const FilePath& previewPath = resources.modelHeightEditorPreviewPaths[index];
                const int32 column = static_cast<int32>(index % PreviewColumnCount);
				const int32 row = static_cast<int32>(index / PreviewColumnCount);
				const double x = ((static_cast<double>(column) - ((PreviewColumnCount - 1) * 0.5)) * PreviewColumnSpacing);
				const double z = (-2.5 + (static_cast<double>(row) - ((rowCount - 1) * 0.5)) * PreviewRowSpacing);
				frame.modelHeightPreviewRenderPositions << Vec3{ x, GetModelHeightOffset(state.editor.modelHeightSettings, previewPath), z };
			}
			return frame;
		}
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.env.cameraSettings = LoadCameraSettings();
		EnsureValidCameraSettings(state.env.cameraSettings);
		ThrowIfInvalidCameraPair(state.env.cameraSettings.eye, state.env.cameraSettings.focus, U"InitializeSkyAppState");
		state.editor.modelHeightSettings = LoadModelHeightSettings();
		state.hud.uiLayoutSettings = LoadUiLayoutSettings(Scene::Width(), Scene::Height());
		state.battle.initialPlayerResources = LoadInitialPlayerResources();
		state.editor.unitEditorSettings = LoadUnitEditorSettings();
		state.world.currentMapPath = MainSupport::MapDataPath;
		const MapDataLoadResult initialMapDataLoad = LoadMapDataWithStatus(MapDataPath);
		state.world.mapData = initialMapDataLoad.mapData;
		state.world.terrainSurfaceRevision = 0;
		Detail::RefreshTerrainSurface(state);
		state.env.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.env.cameraSettings.eye, state.env.cameraSettings.focus };
		ResetMatch(state);

		if (not initialMapDataLoad.message.isEmpty())
		{
			state.messages[SkyAppSupport::MessageChannel::MapData].show(initialMapDataLoad.message);
		}
	}
}
