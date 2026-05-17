#pragma once
# include <Siv3D.hpp>
# include "../Data/DefinitionStores.h"

namespace LT3
{
	struct BattleMapStore
	{
		int32 width  = 0;
		int32 height = 0;
		Array<uint32>  flags;          // bit 0 = passable, bit 1 = reserved barrier block
		Array<UnitId>  occupying;      // InvalidUnitId = empty
		uint32 revision = 0;

		void init(int32 w, int32 h)
		{
			width  = w;
			height = h;
			const size_t n = static_cast<size_t>(w * h);
			flags.assign(n, 1u);       // all passable by default
			occupying.assign(n, InvalidUnitId);
			revision = 0;
		}

		TileIndex index(int32 row, int32 col) const
		{
			return static_cast<TileIndex>(row * width + col);
		}

		bool inBounds(int32 row, int32 col) const
		{
			return row >= 0 && col >= 0 && row < height && col < width;
		}

		bool isPassable(int32 row, int32 col) const
		{
			return inBounds(row, col) && (flags[index(row, col)] & 1u) != 0;
		}

		bool hasBarrierReservation(int32 row, int32 col) const
		{
			return inBounds(row, col) && (flags[index(row, col)] & 2u) != 0;
		}

		void setBarrierReservation(int32 row, int32 col, bool reserved)
		{
			if (!inBounds(row, col))
			{
				return;
			}

			uint32& tileFlags = flags[index(row, col)];
			if (reserved)
			{
				tileFlags |= 2u;
			}
			else
			{
				tileFlags &= ~2u;
			}
		}
	};

	inline void ResizeBattleMapStore(BattleMapStore& map, int32 width, int32 height)
	{
		if (map.width == width && map.height == height)
		{
			return;
		}

		const int32 oldWidth = map.width;
		const int32 oldHeight = map.height;
		const Array<uint32> oldFlags = map.flags;
		const Array<UnitId> oldOccupying = map.occupying;

		map.init(width, height);
		const int32 copyWidth = Min(oldWidth, width);
		const int32 copyHeight = Min(oldHeight, height);
		for (int32 row = 0; row < copyHeight; ++row)
		{
			for (int32 col = 0; col < copyWidth; ++col)
			{
				const size_t oldIndex = static_cast<size_t>(row * oldWidth + col);
				const TileIndex newIndex = map.index(row, col);
				if (oldIndex < oldFlags.size())
				{
					map.flags[newIndex] = oldFlags[oldIndex];
				}
				if (oldIndex < oldOccupying.size())
				{
					map.occupying[newIndex] = oldOccupying[oldIndex];
				}
			}
		}
		++map.revision;
	}
}
