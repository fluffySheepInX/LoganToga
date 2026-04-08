
# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace Detail
	{
		SkyAppFrameState BuildFrameState(SkyAppState& state)
		{
			SkyAppFrameState frame;
			frame.panels = SkyAppPanels{ state.skySettingsExpanded, state.cameraSettingsExpanded, state.miniMapExpanded };
			frame.isEditorMode = (state.appMode == AppMode::EditMap);
			frame.showEscMenu = state.showEscMenu;
			state.mapEditor.enabled = frame.isEditorMode;
			if (not IsValidMillIndex(state, state.selectedMillIndex))
			{
				state.selectedMillIndex.reset();
			}
			frame.showSapperMenu = ((state.selectedSapperIndices.size() == 1) && (not state.playerWon));
			frame.showMillStatusEditor = ((not frame.isEditorMode) && IsValidMillIndex(state, state.selectedMillIndex));
			frame.isHoveringUI = frame.panels.isHoveringUi(state.showUI, frame.isEditorMode, state.showBlacksmithMenu, frame.showSapperMenu, frame.showMillStatusEditor, state.modelHeightEditMode);
			frame.birdRenderPosition = BirdDisplayPosition.movedBy(0, state.modelHeightSettings.birdOffsetY, 0);
			frame.ashigaruRenderPosition = AshigaruDisplayPosition.movedBy(0, state.modelHeightSettings.ashigaruOffsetY, 0);
			return frame;
		}
	}

	SkyAppResources::SkyAppResources()
		: groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB }
		, blacksmithModel{ U"example/obj/blacksmith.obj" }
		, millModel{ U"example/obj/mill.obj" }
		, treeModel{ U"example/obj/tree.obj" }
		, pineModel{ U"example/obj/pine.obj" }
		, birdModel{ BirdModelPath, BirdDisplayHeight }
		, ashigaruModel{ AshigaruModelPath, BirdDisplayHeight }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
	{
		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.cameraSettings = LoadCameraSettings();
		EnsureValidCameraSettings(state.cameraSettings);
		ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"InitializeSkyAppState");
		state.modelHeightSettings = LoadModelHeightSettings();
		const MapDataLoadResult initialMapDataLoad = LoadMapDataWithStatus(MapDataPath);
		state.mapData = initialMapDataLoad.mapData;
		state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.cameraSettings.eye, state.cameraSettings.focus };
		ResetMatch(state);

		if (not initialMapDataLoad.message.isEmpty())
		{
			state.mapDataMessage.show(initialMapDataLoad.message, 4.0);
		}
	}

	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state)
	{
		resources.birdModel.update(Scene::DeltaTime());
		resources.ashigaruModel.update(Scene::DeltaTime());
     UpdateSpawnedSappers(state.spawnedSappers, state.mapData);
		UpdateSpawnedSappers(state.enemySappers, state.mapData);

		if (not state.playerWon)
		{
			UpdateBattleState(state);
		}

		UpdateAttackEffects(state);

		state.selectedSapperIndices.remove_if([&state](const size_t index)
			{
				return (state.spawnedSappers.size() <= index);
			});

		if (KeyEscape.down())
		{
			state.showEscMenu = not state.showEscMenu;
			state.selectionDragStart.reset();
		}

      const SkyAppFrameState frame = Detail::BuildFrameState(state);

		{
			const ScopedRenderTarget3D target{ resources.renderTexture.clear(ColorF{ 0.0 }) };
            Detail::UpdateCameraAndEditor(state, frame);
			Detail::HandleSelectionInput(state, frame);
			RenderWorld(resources, state, frame);
		}

		DrawOverlay(resources, state, frame);
		DrawHudUi(resources, state, frame);
	}
}
