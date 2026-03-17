#pragma once

#include "BonusRoomData.h"
#include "Localization.h"
#include "Remake2Common.h"
#include "RunData.h"
#include "SceneTransitionTypes.h"

enum class WindowResolutionPreset
{
	Small,
	Medium,
	Large,
};

enum class BattleLaunchMode
{
	Run,
	Tutorial
};

struct DisplaySettings
{
	WindowResolutionPreset resolutionPreset = WindowResolutionPreset::Medium;
};

[[nodiscard]] inline constexpr s3d::Size GetBaseSceneResolutionSize() noexcept
{
	return{ 1600, 900 };
}

[[nodiscard]] inline s3d::Size GetWindowResolutionSize(const WindowResolutionPreset preset)
{
	switch (preset)
	{
	case WindowResolutionPreset::Small:
		return{ 960, 540 };
	case WindowResolutionPreset::Large:
		return{ 1600, 900 };
	case WindowResolutionPreset::Medium:
	default:
		return{ 1280, 720 };
	}
}

[[nodiscard]] inline String GetWindowResolutionLabel(const WindowResolutionPreset preset)
{
	switch (preset)
	{
	case WindowResolutionPreset::Small:
		return Localization::GetText(U"common.resolution_small", U"小", U"Small");
	case WindowResolutionPreset::Large:
		return Localization::GetText(U"common.resolution_large", U"大", U"Large");
	case WindowResolutionPreset::Medium:
	default:
		return Localization::GetText(U"common.resolution_medium", U"中", U"Medium");
	}
}

[[nodiscard]] inline double CalculateDisplayScale(const s3d::Size& baseSize, const s3d::Size& currentSize)
{
	return Min((currentSize.x / static_cast<double>(baseSize.x)), (currentSize.y / static_cast<double>(baseSize.y)));
}

[[nodiscard]] inline s3d::Vec2 CalculateDisplayOffset(const s3d::Size& baseSize, const s3d::Size& currentSize)
{
	return ((s3d::Vec2{ currentSize } - (s3d::Vec2{ baseSize } * CalculateDisplayScale(baseSize, currentSize))) / 2.0);
}

inline void ApplyDisplaySettings(const DisplaySettings& displaySettings)
{
	const s3d::Size baseSceneSize = GetBaseSceneResolutionSize();
	const s3d::Size windowSize = GetWindowResolutionSize(displaySettings.resolutionPreset);
	const s3d::Size requiredAreaSize = (windowSize + s3d::Size{ 60, 10 });
	const s3d::Size workAreaSize = System::GetCurrentMonitor().workArea.size;
	const double uiScaling = Max(Window::GetState().scaling, 1.0);
	const s3d::Size availableWorkAreaSize = (s3d::SizeF{ workAreaSize } / uiScaling).asPoint();
	const bool fitsWorkArea = ((requiredAreaSize.x <= availableWorkAreaSize.x) && (requiredAreaSize.y <= availableWorkAreaSize.y));

	Scene::SetResizeMode(ResizeMode::Keep);
	Scene::Resize(baseSceneSize);

	if (fitsWorkArea)
	{
		Window::Resize(windowSize);
	}
	else
	{
		Window::ResizeActual(windowSize);
	}

	Window::Centering();
}

struct GameData
{
	Font titleFont{ FontMethod::MSDF, 44, Typeface::Bold };
	Font uiFont{ FontMethod::MSDF, 24, Typeface::Bold };
	Font smallFont{ 16, Typeface::Medium };
	DisplaySettings displaySettings;
	BattleConfigData baseBattleConfig{ LoadBattleConfig(U"config/battle.toml") };
	BattleConfigData tutorialBattleConfig{ LoadBattleConfig(U"config/battle_tutorial.toml") };
	Array<RewardCardDefinition> rewardCards{ LoadRewardCardDefinitions(U"config/cards.toml") };
	Array<BonusRoomDefinition> bonusRooms{ LoadBonusRoomDefinitions(U"config/bonus_rooms.toml") };
	BattleLaunchMode battleLaunchMode = BattleLaunchMode::Run;
	SceneTransitionSettings sceneTransitionSettings;
	SceneTransitionState sceneTransition;
	RunState runState;
	BonusRoomProgress bonusRoomProgress;
};

inline void ReloadGameConfigData(GameData& data)
{
	data.baseBattleConfig = LoadBattleConfig(U"config/battle.toml");
	data.tutorialBattleConfig = LoadBattleConfig(U"config/battle_tutorial.toml");
	data.rewardCards = LoadRewardCardDefinitions(U"config/cards.toml");
	data.bonusRooms = LoadBonusRoomDefinitions(U"config/bonus_rooms.toml");
}

using App = SceneManager<String, GameData>;
using SceneBase = s3d::IScene<String, GameData>;
