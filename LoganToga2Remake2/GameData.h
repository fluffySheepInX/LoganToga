#pragma once

#include "Remake2Common.h"
#include "RunData.h"

struct GameData
{
	Font titleFont{ FontMethod::MSDF, 44, Typeface::Bold };
	Font uiFont{ FontMethod::MSDF, 24, Typeface::Bold };
	Font smallFont{ 16, Typeface::Medium };
	BattleConfigData baseBattleConfig{ LoadBattleConfig(U"config/battle.toml") };
	Array<RewardCardDefinition> rewardCards{ LoadRewardCardDefinitions(U"config/cards.toml") };
	RunState runState;
};

using App = SceneManager<String, GameData>;
using SceneBase = s3d::IScene<String, GameData>;
