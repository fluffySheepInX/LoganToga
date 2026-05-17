#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Data/BattleAssetPaths.h"
# include "MapEditorCoreTypes.h"
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

	inline Array<bool> BuildFogVisibleCellMask(const BattleWorld& world, const DefinitionStores& defs)
	{
		const int32 width = Max(1, world.mapWidth);
		const int32 height = Max(1, world.mapHeight);
		Array<bool> visible;
		visible.assign(static_cast<size_t>(width * height), false);

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!world.units.alive[unit] || world.units.faction[unit] != Faction::Player)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			const int32 radius = Max(0, def.visionRadiusCells);
			const int32 radiusSq = radius * radius;
			const Point unitCell = QuarterWorldToBattleCell(world.units.position[unit], width, height);

			for (int32 dy = -radius; dy <= radius; ++dy)
			{
				for (int32 dx = -radius; dx <= radius; ++dx)
				{
					const int32 x = unitCell.x + dx;
					const int32 y = unitCell.y + dy;
					if (x < 0 || y < 0 || x >= width || y >= height)
					{
						continue;
					}
					if ((dx * dx + dy * dy) > radiusSq)
					{
						continue;
					}

					visible[static_cast<size_t>(y * width + x)] = true;
				}
			}
		}

		return visible;
	}

	inline void DrawFogOfWarOverlay(const BattleWorld& world, const DefinitionStores& defs, const MapEditorState& mapEditor)
	{
		if (!mapEditor.fogEnabled)
		{
			return;
		}

		const int32 width = Max(1, world.mapWidth);
		const int32 height = Max(1, world.mapHeight);
		const Array<bool> visible = BuildFogVisibleCellMask(world, defs);
		const ColorF fogColor{ mapEditor.fogColor.r, mapEditor.fogColor.g, mapEditor.fogColor.b, mapEditor.fogOpacity };

		for (int32 diagonal = 0; diagonal < (width + height - 1); ++diagonal)
		{
			const int32 xBegin = Max(0, diagonal - (height - 1));
			const int32 xEnd = Min(width - 1, diagonal);
			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = diagonal - x;
				const size_t index = static_cast<size_t>(y * width + x);
				if (index >= visible.size() || visible[index])
				{
					continue;
				}

				const Quad tile = ToQuarterTile(QuarterBattleCellCenter(x, y));
				tile.draw(fogColor);
			}
		}

		if (!mapEditor.fogPreviewVision)
		{
			return;
		}

		for (int32 diagonal = 0; diagonal < (width + height - 1); ++diagonal)
		{
			const int32 xBegin = Max(0, diagonal - (height - 1));
			const int32 xEnd = Min(width - 1, diagonal);
			for (int32 x = xBegin; x <= xEnd; ++x)
			{
				const int32 y = diagonal - x;
				const size_t index = static_cast<size_t>(y * width + x);
				if (index < visible.size() && visible[index])
				{
					ToQuarterTile(QuarterBattleCellCenter(x, y)).drawFrame(1.0, ColorF{ 0.75, 0.95, 0.88, 0.10 });
				}
			}
		}
	}
}
