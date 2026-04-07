# pragma once
# include "SkyAppLoop.hpp"

namespace SkyAppFlow
{
	[[nodiscard]] bool IsValidMillIndex(const SkyAppState& state, const Optional<size_t>& millIndex);
    void ResetCameraToPlayerBase(SkyAppState& state);
	void ResetMatch(SkyAppState& state);
	String ReloadMapAndResetMatch(SkyAppState& state);
	void UpdateBattleState(SkyAppState& state);
	void RenderWorld(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
	void DrawOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
	void DrawHudUi(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
}
