#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Data/BattleAssetPaths.h"
# include "QuarterView.h"

namespace LT3
{
	inline ColorF TileColor(int32 x, int32 y)
	{
		const bool alternate = ((x + y) % 2) == 0;
		if (alternate)
		{
			return ColorF{ 0.18, 0.31, 0.22 };
		}
		return ColorF{ 0.14, 0.27, 0.20 };
	}

	inline void DrawQuarterMap(const BattleWorld& world, const DefinitionStores&, const Font&)
	{
		for (int32 diagonal = 0; diagonal < (world.mapWidth + world.mapHeight - 1); ++diagonal)
		{
			const int32 xBegin = Max(0, diagonal - (world.mapHeight - 1));
			const int32 xEnd = Min(world.mapWidth - 1, diagonal);
			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = diagonal - x;
				const Vec2 center = QuarterBattleCellCenter(x, y);
				const Quad tile = ToQuarterTile(center);
				tile.draw(TileColor(x, y));
				tile.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.08 });
			}
		}
	}
}
