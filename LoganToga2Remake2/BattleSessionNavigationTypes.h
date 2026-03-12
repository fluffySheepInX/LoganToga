#pragma once

#include "BattleSessionCollision.h"

namespace BattleSessionInternal
{
	struct NavigationGridCell
	{
		int32 x = 0;
		int32 y = 0;
	};

	struct NavigationGrid
	{
		RectF bounds{ 0, 0, 0, 0 };
		double cellSize = 24.0;
		int32 columns = 1;
		int32 rows = 1;
		Array<char> blocked;
	};
}
