#pragma once

#include "Localization.h"
#include "Remake2Common.h"

enum class SceneTransitionPreset
{
	Off,
	Fade,
	CyberScan,
	CyberGrid,
};

enum class SceneTransitionPhase
{
	None,
	FadeOut,
	FadeIn,
};

struct SceneTransitionSettings
{
	SceneTransitionPreset preset = SceneTransitionPreset::CyberScan;
};

struct SceneTransitionState
{
	SceneTransitionPhase phase = SceneTransitionPhase::None;
	SceneTransitionPreset activePreset = SceneTransitionPreset::CyberScan;
	String targetScene;
	double timer = 0.0;
	double duration = 0.22;
};

[[nodiscard]] inline String GetSceneTransitionPresetLabel(const SceneTransitionPreset preset)
{
	switch (preset)
	{
	case SceneTransitionPreset::Off:
      return Localization::GetText(U"common.transition.off");
	case SceneTransitionPreset::Fade:
     return Localization::GetText(U"common.transition.fade");
	case SceneTransitionPreset::CyberGrid:
       return Localization::GetText(U"common.transition.cyber_grid");
	case SceneTransitionPreset::CyberScan:
	default:
       return Localization::GetText(U"common.transition.cyber_scan");
	}
}

[[nodiscard]] inline SceneTransitionPreset CycleSceneTransitionPreset(const SceneTransitionPreset preset)
{
	switch (preset)
	{
	case SceneTransitionPreset::Off:
		return SceneTransitionPreset::Fade;
	case SceneTransitionPreset::Fade:
		return SceneTransitionPreset::CyberScan;
	case SceneTransitionPreset::CyberScan:
		return SceneTransitionPreset::CyberGrid;
	case SceneTransitionPreset::CyberGrid:
	default:
		return SceneTransitionPreset::Off;
	}
}
