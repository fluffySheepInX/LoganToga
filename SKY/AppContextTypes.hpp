# pragma once
# include "MainConstants.hpp"

namespace MainSupport
{
	struct UiLayoutSettings
	{
		Point miniMapPosition{ 0, 0 };
		Point resourcePanelPosition{ 0, 0 };
      Point resourcePanelSize{ 0, 0 };
		Point modelHeightPosition{ 0, 0 };
       Point terrainVisualSettingsPosition{ 0, 0 };
       Point fogSettingsPosition{ 0, 0 };
		Point unitEditorPosition{ 0, 0 };
		Point unitEditorListPosition{ 0, 0 };
	};

	enum class AppMode
	{
		Play,
		EditMap,
	};

	enum class EnemyBattlePlan
	{
		SecureResources,
		AssaultBase,
	};

	enum class EnemyBattlePlanOverride
	{
		Auto,
		ForceSecureResources,
		ForceAssaultBase,
	};
}
