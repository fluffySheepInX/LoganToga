#pragma once
# include <Siv3D.hpp>
# include "App/AppDefinitionState.h"
# include "App/AppRuntimeState.h"
# include "App/AppUiState.h"
# include "../UI/QuarterView.h"

namespace LT3
{
	struct AppState
	{
		Font titleFont{ FontMethod::MSDF, 38, Typeface::Bold };
		Font uiFont{ FontMethod::MSDF, 20, Typeface::Medium };
		AppDefinitionState definitions = CreateAppDefinitionState();
		AppRuntimeState runtime;
		AppUiState ui;
	};

	inline Vec2 ToWorldPos(const Vec2& screenPos)
	{
		return ToQuarterWorld(screenPos);
	}
}
