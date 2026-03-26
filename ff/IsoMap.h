# pragma once
# include "GameConstants.h"

namespace ff
{
	inline Vec2 ToIsometric(const Vec2& gridPos)
	{
		return{
			((gridPos.x - gridPos.y) * TileHalfSize.x),
			((gridPos.x + gridPos.y) * TileHalfSize.y)
		};
	}

	inline Vec2 ToScreenPos(const Point& tileIndex, const Vec2& worldOrigin)
	{
		return (worldOrigin + ToIsometric(Vec2{ static_cast<double>(tileIndex.x), static_cast<double>(tileIndex.y) }));
	}

	inline Vec2 ToTileBottomCenter(const Point& tileIndex, const Vec2& worldOrigin)
	{
		return (ToScreenPos(tileIndex, worldOrigin).movedBy(0, (TileHalfSize.y + TileThickness)));
	}

	inline Quad MakeTileQuad(const Vec2& center)
	{
		return{
			center.movedBy(0, -TileHalfSize.y),
			center.movedBy(TileHalfSize.x, 0),
			center.movedBy(0, TileHalfSize.y),
			center.movedBy(-TileHalfSize.x, 0)
		};
	}

	inline Point ToTileIndex(const Vec2& position)
	{
		return{
			Clamp(static_cast<int32>(position.x + 0.5), 0, (MapSize.x - 1)),
			Clamp(static_cast<int32>(position.y + 0.5), 0, (MapSize.y - 1))
		};
	}

	template <class F>
	void ForEachTileByDepth(F&& f)
	{
		for (int32 depth = 0; depth < (MapSize.x + MapSize.y - 1); ++depth)
		{
			const int32 xBegin = Max(0, (depth - (MapSize.y - 1)));
			const int32 xEnd = Min((MapSize.x - 1), depth);

			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = (depth - x);
				f(Point{ x, y });
			}
		}
	}
}
