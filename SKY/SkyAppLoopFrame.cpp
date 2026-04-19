# include "SkyAppLoopInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
        void ApplyConfiguredAnimationClip(UnitModel& renderModel, const int32 configuredClipIndex)
		{
			if (not renderModel.hasAnimations())
			{
				return;
			}

			if (configuredClipIndex < 0)
			{
				renderModel.clearClipIndex();
				return;
			}

			const size_t resolvedClipIndex = Min(static_cast<size_t>(Max(configuredClipIndex, 0)), (renderModel.animations().size() - 1));
			if (renderModel.currentClipIndex() != resolvedClipIndex)
			{
				renderModel.setClipIndex(resolvedClipIndex);
			}
		}

     void UpdateUnitRenderModels(SkyAppResources& resources, const ModelHeightSettings& modelHeightSettings)
		{
            for (const UnitRenderModel renderModelType : GetUnitRenderModels())
			{
                for (const UnitModelAnimationRole role : { UnitModelAnimationRole::Idle, UnitModelAnimationRole::Move, UnitModelAnimationRole::Attack })
				{
					UnitModel& renderModel = resources.GetUnitRenderModel(renderModelType, role);
					ApplyConfiguredAnimationClip(renderModel, GetModelAnimationClipIndex(modelHeightSettings, renderModelType, role));
					renderModel.update(Scene::DeltaTime());
				}
			}

			for (size_t previewModelIndex = 0; previewModelIndex < resources.modelHeightEditorPreviewPaths.size(); ++previewModelIndex)
			{
				const FilePath& previewPath = resources.modelHeightEditorPreviewPaths[previewModelIndex];
				for (const UnitModelAnimationRole role : { UnitModelAnimationRole::Idle, UnitModelAnimationRole::Move, UnitModelAnimationRole::Attack })
				{
					UnitModel& previewRenderModel = resources.GetModelHeightEditorPreviewModel(previewModelIndex, role);
					ApplyConfiguredAnimationClip(previewRenderModel, GetModelAnimationClipIndex(modelHeightSettings, previewPath, role));
					previewRenderModel.update(Scene::DeltaTime());
				}
			}
		}

		void RemoveInvalidSelectedSapperIndices(SkyAppState& state)
		{
			state.selectedSapperIndices.remove_if([&state](const size_t index)
				{
					return (state.spawnedSappers.size() <= index);
				});
		}

		void UpdateFrameSimulation(SkyAppState& state)
		{
			UpdateSpawnedSappers(state.spawnedSappers, state.mapData, state.modelHeightSettings);
			UpdateSpawnedSappers(state.enemySappers, state.mapData, state.modelHeightSettings);
			UpdateFogOfWar(state.fogOfWar,
				state.fogOfWarSettings,
				state.mapData,
				state.spawnedSappers,
				state.resourceAreaStates);

			if (not state.playerWon)
			{
				UpdateBattleState(state);
			}

			UpdateAttackEffects(state);
			UpdateSkyFromTime(state.sky, state.skyTime);
			Detail::RefreshTerrainSurface(state);
			RemoveInvalidSelectedSapperIndices(state);
		}

		void HandleFrameInput(SkyAppState& state)
		{
			if (KeyEscape.down())
			{
				state.showEscMenu = not state.showEscMenu;
				state.selectionDragStart.reset();
				state.uiPanelDrag.reset();
			}
		}

        [[nodiscard]] SkyAppFrameState PrepareFrameState(SkyAppResources& resources, SkyAppState& state)
		{
			Detail::NormalizeFrameStateInputs(state);
            SkyAppFrameState frame = Detail::BuildFrameState(state, resources);
			Detail::UiEditInput::Handle(state, frame);
			return frame;
		}

		void RenderFrameWorld(SkyAppResources& resources, SkyAppState& state, SkyAppFrameState& frame)
		{
			const ScopedRenderTarget3D target{ resources.renderTexture.clear(ColorF{ 0.0 }) };
			Detail::UpdateCameraAndEditor(state, frame);
			Detail::HandleSelectionInput(state, frame);
			RenderWorld(resources, state, frame);
		}

		void RenderFrameUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
			DrawOverlay(resources, state, frame);
			DrawHudUi(resources, state, frame);
		}
	}

	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state)
	{
      UpdateUnitRenderModels(resources, state.modelHeightSettings);
		UpdateFrameSimulation(state);
		HandleFrameInput(state);

      SkyAppFrameState frame = PrepareFrameState(resources, state);
		RenderFrameWorld(resources, state, frame);
		RenderFrameUi(resources, state, frame);
	}
}
