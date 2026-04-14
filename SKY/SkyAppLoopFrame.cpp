# include "SkyAppLoopInternal.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		void UpdateUnitRenderModels(SkyAppResources& resources)
		{
			for (auto& renderModel : resources.unitRenderModels)
			{
				renderModel.update(Scene::DeltaTime());
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

		[[nodiscard]] SkyAppFrameState PrepareFrameState(SkyAppState& state)
		{
			Detail::NormalizeFrameStateInputs(state);
			SkyAppFrameState frame = Detail::BuildFrameState(state);
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
		UpdateUnitRenderModels(resources);
		UpdateFrameSimulation(state);
		HandleFrameInput(state);

		SkyAppFrameState frame = PrepareFrameState(state);
		RenderFrameWorld(resources, state, frame);
		RenderFrameUi(resources, state, frame);
	}
}
