#pragma once

#include "GameData.h"

[[nodiscard]] inline double GetSceneTransitionStrength(const SceneTransitionState& transition)
{
	const double progress = Clamp(transition.timer / Max(transition.duration, 0.001), 0.0, 1.0);
	return (transition.phase == SceneTransitionPhase::FadeOut)
		? progress
		: (1.0 - progress);
}

template <class TChangeScene>
inline void RequestSceneTransition(GameData& data, const String& targetScene, TChangeScene&& changeSceneNow)
{
	auto& transition = data.sceneTransition;
	if (transition.phase != SceneTransitionPhase::None)
	{
		return;
	}

	const SceneTransitionPreset preset = data.sceneTransitionSettings.preset;
	if (preset == SceneTransitionPreset::Off)
	{
		changeSceneNow(targetScene);
		return;
	}

	transition.phase = SceneTransitionPhase::FadeOut;
	transition.activePreset = preset;
	transition.targetScene = targetScene;
	transition.timer = 0.0;
	transition.duration = 0.22;
}

template <class TChangeScene>
inline bool UpdateSceneTransition(GameData& data, TChangeScene&& changeSceneNow)
{
	auto& transition = data.sceneTransition;
	if (transition.phase == SceneTransitionPhase::None)
	{
		return false;
	}

	transition.timer += Scene::DeltaTime();
	if (transition.timer < transition.duration)
	{
		return true;
	}

	if (transition.phase == SceneTransitionPhase::FadeOut)
	{
		const String nextScene = transition.targetScene;
		transition.phase = SceneTransitionPhase::FadeIn;
		transition.timer = 0.0;
		changeSceneNow(nextScene);
		return true;
	}

	transition.phase = SceneTransitionPhase::None;
	transition.timer = 0.0;
	transition.targetScene.clear();
	return false;
}

inline void DrawStandardSceneTransition(const double strength)
{
	Scene::Rect().draw(ColorF{ 0.01, 0.02, 0.04, 0.92 * strength });
}

inline void DrawCyberScanSceneTransition(const SceneTransitionState& transition, const double strength)
{
	const double progress = Clamp(transition.timer / Max(transition.duration, 0.001), 0.0, 1.0);
	const double sweep = ((transition.phase == SceneTransitionPhase::FadeOut) ? progress : (1.0 - progress)) * Scene::Height();
	Scene::Rect().draw(ColorF{ 0.01, 0.03, 0.06, 0.84 * strength });

	for (double y = 0.0; y < Scene::Height(); y += 6.0)
	{
		RectF{ 0, y, Scene::Width(), 1 }.draw(ColorF{ 0.18, 0.42, 0.68, 0.09 * strength });
	}

	const RectF sweepRect{ 0, Max(0.0, sweep - 34.0), Scene::Width(), 68.0 };
	sweepRect.draw(ColorF{ 0.18, 0.82, 1.0, 0.20 * strength });
	RectF{ 0, Clamp(sweep - 1.5, 0.0, static_cast<double>(Scene::Height())), Scene::Width(), 3.0 }.draw(ColorF{ 0.92, 0.98, 1.0, 0.82 * strength });
	RectF{ 0, 0, 5, Scene::Height() }.draw(ColorF{ 0.14, 0.58, 0.88, 0.18 * strength });
	RectF{ Scene::Width() - 5.0, 0, 5, Scene::Height() }.draw(ColorF{ 0.14, 0.58, 0.88, 0.18 * strength });
}

inline void DrawCyberGridSceneTransition(const double strength)
{
	Scene::Rect().draw(ColorF{ 0.02, 0.03, 0.07, 0.82 * strength });
	const double spacing = 34.0;
	for (double x = 0.0; x <= Scene::Width(); x += spacing)
	{
		RectF{ x, 0, 1, Scene::Height() }.draw(ColorF{ 0.18, 0.80, 1.0, 0.12 * strength });
	}
	for (double y = 0.0; y <= Scene::Height(); y += spacing)
	{
		RectF{ 0, y, Scene::Width(), 1 }.draw(ColorF{ 0.18, 0.80, 1.0, 0.12 * strength });
	}

	const RectF centerRect{ Arg::center = Scene::CenterF(), Scene::Width() * 0.72, Scene::Height() * 0.52 };
	centerRect.drawFrame(2.0 + (2.0 * strength), ColorF{ 0.28, 0.92, 1.0, 0.34 * strength });
	RectF{ 0, Scene::CenterF().y - 2.0, Scene::Width(), 4.0 }.draw(ColorF{ 0.72, 0.96, 1.0, 0.18 * strength });
}

inline void DrawSceneTransitionOverlay(const GameData& data)
{
	const auto& transition = data.sceneTransition;
	if ((transition.phase == SceneTransitionPhase::None) || (transition.activePreset == SceneTransitionPreset::Off))
	{
		return;
	}

	const double strength = GetSceneTransitionStrength(transition);
	if (strength <= 0.0)
	{
		return;
	}

	switch (transition.activePreset)
	{
	case SceneTransitionPreset::Fade:
		DrawStandardSceneTransition(strength);
		break;
	case SceneTransitionPreset::CyberGrid:
		DrawCyberGridSceneTransition(strength);
		break;
	case SceneTransitionPreset::CyberScan:
		DrawCyberScanSceneTransition(transition, strength);
		break;
	case SceneTransitionPreset::Off:
	default:
		break;
	}
}
