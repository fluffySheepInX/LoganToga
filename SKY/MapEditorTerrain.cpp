# include "MapEditorUpdateInternal.hpp"

namespace
{
	void PaintTerrainCellRange(MapData& mapData, const Point& startCell, const Point& endCell, const MapEditorState& state)
	{
		const int32 minX = Min(startCell.x, endCell.x);
		const int32 maxX = Max(startCell.x, endCell.x);
		const int32 minY = Min(startCell.y, endCell.y);
		const int32 maxY = Max(startCell.y, endCell.y);

		if (const auto terrainType = MapEditorDetail::ToTerrainCellType(state.selectedTool))
		{
			for (int32 y = minY; y <= maxY; ++y)
			{
				for (int32 x = minX; x <= maxX; ++x)
				{
					SetTerrainCell(mapData.terrainCells, Point{ x, y }, *terrainType, state.selectedTerrainColor);
				}
			}
			return;
		}

		for (int32 y = minY; y <= maxY; ++y)
		{
			for (int32 x = minX; x <= maxX; ++x)
			{
				RemoveTerrainCell(mapData.terrainCells, Point{ x, y });
			}
		}
	}

	[[nodiscard]] int32 CountTerrainCellsInRange(const Point& startCell, const Point& endCell)
	{
		return ((Abs(startCell.x - endCell.x) + 1) * (Abs(startCell.y - endCell.y) + 1));
	}

	[[nodiscard]] int32 RemoveTerrainCellRange(MapData& mapData, const Point& startCell, const Point& endCell)
	{
		const int32 minX = Min(startCell.x, endCell.x);
		const int32 maxX = Max(startCell.x, endCell.x);
		const int32 minY = Min(startCell.y, endCell.y);
		const int32 maxY = Max(startCell.y, endCell.y);
		int32 removedCount = 0;

		for (int32 y = minY; y <= maxY; ++y)
		{
			for (int32 x = minX; x <= maxX; ++x)
			{
				removedCount += RemoveTerrainCell(mapData.terrainCells, Point{ x, y }) ? 1 : 0;
			}
		}

		return removedCount;
	}
}

namespace MapEditorUpdateDetail
{
	bool HandleTerrainEditing(MapEditorState& state, MapData& mapData)
	{
		const auto terrainType = MapEditorDetail::ToTerrainCellType(state.selectedTool);
		const bool isTerrainEditingTool = (terrainType || (state.selectedTool == MapEditorTool::EraseTerrain));

		if (isTerrainEditingTool && state.hoveredGroundPosition)
		{
			const Point cell = ToTerrainCell(*state.hoveredGroundPosition);
			if (state.terrainPaintMode == MapEditorTerrainPaintMode::Area)
			{
				state.lastTerrainPaintCell.reset();

				if (MouseL.down())
				{
					state.pendingTerrainPaintRangeStartCell = cell;
					return true;
				}

				if (state.pendingTerrainPaintRangeStartCell && MouseL.up())
				{
					const Point startCell = *state.pendingTerrainPaintRangeStartCell;
					state.pendingTerrainPaintRangeStartCell.reset();

					if (terrainType)
					{
						PaintTerrainCellRange(mapData, startCell, cell, state);
						MapEditorDetail::SetStatusMessage(state, U"{} を範囲塗布 [{} セル]"_fmt(MapEditorDetail::ToLabel(state.selectedTool), CountTerrainCellsInRange(startCell, cell)));
					}
					else
					{
						MapEditorDetail::SetStatusMessage(state, U"地表セルを範囲削除 [{} セル]"_fmt(RemoveTerrainCellRange(mapData, startCell, cell)));
					}

					return true;
				}

				return true;
			}

			state.pendingTerrainPaintRangeStartCell.reset();
			if (MouseL.pressed())
			{
				if (state.lastTerrainPaintCell && (*state.lastTerrainPaintCell == cell))
				{
					return true;
				}

				state.lastTerrainPaintCell = cell;
				if (terrainType)
				{
					SetTerrainCell(mapData.terrainCells, cell, *terrainType, state.selectedTerrainColor);
					MapEditorDetail::SetStatusMessage(state, U"{} を塗布"_fmt(MapEditorDetail::ToLabel(state.selectedTool)));
				}
				else if (RemoveTerrainCell(mapData.terrainCells, cell))
				{
					MapEditorDetail::SetStatusMessage(state, U"地表セルを削除");
				}
			}

			return true;
		}

		if (isTerrainEditingTool && state.pendingTerrainPaintRangeStartCell && MouseL.up())
		{
			state.pendingTerrainPaintRangeStartCell.reset();
			return true;
		}

		return false;
	}
}
