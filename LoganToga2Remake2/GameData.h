#pragma once

#include "BonusRoomData.h"
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
		return U"小";
	case WindowResolutionPreset::Large:
		return U"大";
	case WindowResolutionPreset::Medium:
	default:
		return U"中";
	}
}

inline void ApplyDisplaySettings(const DisplaySettings& displaySettings)
{
	const s3d::Size windowSize = GetWindowResolutionSize(displaySettings.resolutionPreset);
	Window::Resize(windowSize.x, windowSize.y);
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

using App = SceneManager<String, GameData>;
using SceneBase = s3d::IScene<String, GameData>;
