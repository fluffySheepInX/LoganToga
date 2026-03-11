#pragma once

#include "Remake2Common.h"

struct GameData
{
	Font titleFont{ FontMethod::MSDF, 44, Typeface::Bold };
	Font uiFont{ FontMethod::MSDF, 24, Typeface::Bold };
	Font smallFont{ 16, Typeface::Medium };
};

using App = SceneManager<String, GameData>;
using SceneBase = s3d::IScene<String, GameData>;
