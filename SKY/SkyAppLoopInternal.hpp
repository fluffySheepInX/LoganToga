# pragma once
# include "SkyAppLoop.hpp"

namespace SkyAppFlow
{
   namespace Detail
	{
      void NormalizeFrameStateInputs(SkyAppState& state);
		void RefreshTerrainSurface(SkyAppState& state);
     [[nodiscard]] SkyAppFrameState BuildFrameState(const SkyAppState& state);

		namespace UiEditInput
		{
			[[nodiscard]] bool Handle(SkyAppState& state, SkyAppFrameState& frame);
		}

		void UpdateCameraAndEditor(SkyAppState& state, const SkyAppFrameState& frame);
		void HandleSelectionInput(SkyAppState& state, const SkyAppFrameState& frame);
	}

	[[nodiscard]] bool IsValidMillIndex(const SkyAppState& state, const Optional<size_t>& millIndex);
    void ResetCameraToPlayerBase(SkyAppState& state);
	void ResetMatch(SkyAppState& state);
	String ReloadMapAndResetMatch(SkyAppState& state);
 void UpdateAttackEffects(SkyAppState& state);
	void UpdateBattleState(SkyAppState& state);
 [[nodiscard]] bool TryUsePlayerSapperUniqueSkill(SkyAppState& state, size_t selectedSapperIndex);
    [[nodiscard]] bool TryUsePlayerSapperExplosionSkill(SkyAppState& state, size_t selectedSapperIndex);
    [[nodiscard]] bool TryOrderPlayerSapperRetreat(SkyAppState& state, size_t selectedSapperIndex);
        [[nodiscard]] bool HandleEscMenu(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
	void DrawSettingsHud(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
	void DrawContextHud(SkyAppState& state, const SkyAppFrameState& frame);
	void DrawHudModeToggles(SkyAppState& state, const SkyAppFrameState& frame);
	void DrawHudFooter(SkyAppState& state, const SkyAppFrameState& frame);
    void RenderWorld(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame);
	void DrawOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
	void DrawHudUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
}
