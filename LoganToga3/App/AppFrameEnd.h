#pragma once
# include <Siv3D.hpp>
# include "../libs/AddonGaussian.h"

namespace LT3
{
	inline bool ProcessGaussianAddonFrameEnd()
	{
		if (GaussianFSAddon::TriggerOrDisplayESC()) return false;
		if (GaussianFSAddon::TriggerOrDisplayLang()) return false;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) return false;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) return false;
		GaussianFSAddon::DragProcessWindow();
		return true;
	}
}
